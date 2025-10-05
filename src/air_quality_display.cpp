#include "air_quality_display.h"

AirQualityDisplay::AirQualityDisplay(PMSSensor* pmsSensor) {
  sensor = pmsSensor;
  currentScreen = MAIN;
  lastScreenChange = 0;
  alertActive = false;
  
  // Initialize OLED display with SSH1106 configuration
  u8g2 = new U8G2_SH1106_128X64_NONAME_F_HW_I2C(U8G2_R0, /* reset=*/ U8X8_PIN_NONE, /* scl=*/ D1, /* sda=*/ D2);
}

AirQualityDisplay::~AirQualityDisplay() {
  // Note: Not deleting u8g2 to avoid undefined behavior warning
  // The object will be cleaned up when the program ends
}

void AirQualityDisplay::begin() {
  // Initialize OLED
  u8g2->begin();
  u8g2->setDisplayRotation(U8G2_R0);
  u8g2->clearBuffer();
  u8g2->clearDisplay();
  u8g2->setBitmapMode(1);
  
  // Initialize buzzer
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW);
  
  // Show startup screen
  showBootScreen();
  
  Serial.println("Air Quality Display initialized");
}

void AirQualityDisplay::showBootScreen() {
  u8g2->clearBuffer();
  u8g2->setFont(u8g2_font_helvB14_tf);
  u8g2->drawStr(10, 20, "Air Quality");
  u8g2->setFont(u8g2_font_helvR12_tf);
  u8g2->drawStr(15, 35, "Monitor v2.0");
  u8g2->setFont(u8g2_font_helvR08_tf);
  u8g2->drawStr(5, 50, "PMS5003 + ESP8266");
  u8g2->drawStr(25, 62, "Starting...");
  u8g2->sendBuffer();
}

void AirQualityDisplay::update() {
  if (!sensor->isDataValid()) {
    // Show "No Data" screen
    u8g2->clearBuffer();
    u8g2->setFont(u8g2_font_helvB14_tf);
    u8g2->drawStr(15, 25, "No Sensor");
    u8g2->drawStr(30, 45, "Data");
    u8g2->setFont(u8g2_font_helvR08_tf);
    u8g2->drawStr(10, 58, "Check connections");
    u8g2->sendBuffer();
    return;
  }
  
  checkAlerts();
  rotateScreen();
  
  // Update display based on current screen
  switch (currentScreen) {
    case MAIN:
      displayMainScreen();
      break;
    case HEALTH_RISK:
      displayHealthRiskScreen();
      break;
    case ALERT:
      displayAlertScreen();
      break;
    case TREND:
      displayTrendScreen();
      break;
    case COMPARISON:
      displayComparisonScreen();
      break;
    case PARTICLES:
      displayParticlesScreen();
      break;
  }
}

void AirQualityDisplay::displayMainScreen() {
  u8g2->clearBuffer();
  char buf[32];
  
  // Status at top with larger font
  u8g2->setFont(u8g2_font_helvB12_tf);
  String status = sensor->getHealthStatus();
  u8g2->drawStr(2, 14, status.c_str());
  
  // PM readings
  u8g2->setFont(u8g2_font_helvR10_tf);
  sprintf(buf, "PM1.0: %u ug/m3", sensor->currentData.pm1_0_atm);
  u8g2->drawStr(2, 28, buf);
  
  sprintf(buf, "PM2.5: %u ug/m3", sensor->currentData.pm2_5_atm);
  u8g2->drawStr(2, 42, buf);
  
  sprintf(buf, "PM10:  %u ug/m3", sensor->currentData.pm10_atm);
  u8g2->drawStr(2, 56, buf);
  
  // Add small indicator for screen rotation
  u8g2->setFont(u8g2_font_4x6_tf);
  u8g2->drawStr(118, 62, "1/5");
  
  u8g2->sendBuffer();
}

void AirQualityDisplay::displayHealthRiskScreen() {
  u8g2->clearBuffer();
  char buf[32];
  
  u8g2->setFont(u8g2_font_helvB12_tf);
  String riskLevel = sensor->getRiskLevel();
  if (riskLevel == "HIGH") {
    u8g2->drawStr(2, 14, "HIGH RISK!");
  } else if (riskLevel == "MODERATE") {
    u8g2->drawStr(2, 14, "MODERATE RISK");
  } else {
    u8g2->drawStr(2, 14, "LOW RISK");
  }
  
  sprintf(buf, "PM2.5: %u ug/m3", sensor->currentData.pm2_5_atm);
  u8g2->setFont(u8g2_font_helvR10_tf);
  u8g2->drawStr(2, 30, buf);
  
  u8g2->setFont(u8g2_font_helvR08_tf);
  if (sensor->currentData.pm2_5_atm > 35) {
    u8g2->drawStr(2, 44, "* Asthma risk");
    u8g2->drawStr(2, 54, "* Use air purifier");
  } else if (sensor->currentData.pm2_5_atm > 12) {
    u8g2->drawStr(2, 44, "* Sensitive groups");
    u8g2->drawStr(2, 54, "* Monitor levels");
  } else {
    u8g2->drawStr(2, 44, "* Air quality good");
    u8g2->drawStr(2, 54, "* Safe for all");
  }
  
  u8g2->setFont(u8g2_font_4x6_tf);
  u8g2->drawStr(118, 62, "2/5");
  
  u8g2->sendBuffer();
}

void AirQualityDisplay::displayAlertScreen() {
  u8g2->clearBuffer();
  char buf[32];
  
  u8g2->setFont(u8g2_font_helvB12_tf);
  if (sensor->currentData.pm2_5_atm > 55) {
    u8g2->drawStr(2, 14, "UNHEALTHY!");
  } else {
    u8g2->drawStr(2, 14, "ALERT!");
  }
  
  sprintf(buf, "PM2.5: %u ug/m3", sensor->currentData.pm2_5_atm);
  u8g2->setFont(u8g2_font_helvR10_tf);
  u8g2->drawStr(2, 30, buf);
  
  u8g2->setFont(u8g2_font_helvR08_tf);
  u8g2->drawStr(2, 44, "TAKE ACTION:");
  u8g2->drawStr(2, 54, "* Close windows");
  u8g2->drawStr(2, 62, "* Use air purifier");
  
  u8g2->setFont(u8g2_font_4x6_tf);
  u8g2->drawStr(118, 6, "3/5");
  
  u8g2->sendBuffer();
}

void AirQualityDisplay::displayTrendScreen() {
  u8g2->clearBuffer();
  char buf[32];
  
  sprintf(buf, "PM2.5: %u ug/m3", sensor->currentData.pm2_5_atm);
  u8g2->setFont(u8g2_font_helvR10_tf);
  u8g2->drawStr(2, 12, buf);
  
  // Find peak value and hour
  bool hasData = false;
  float peak = 0;
  uint8_t peakHour = 0;
  
  for (uint8_t i = 0; i < 24; i++) {
    if (sensor->trendData[i] > 0) {
      hasData = true;
      if (sensor->trendData[i] > peak) {
        peak = sensor->trendData[i];
        peakHour = i;
      }
    }
  }
  
  if (hasData) {
    sprintf(buf, "Peak: %u at %02u:00", (uint16_t)peak, peakHour);
  } else {
    sprintf(buf, "Peak: No data yet");
  }
  u8g2->setFont(u8g2_font_helvR08_tf);
  u8g2->drawStr(2, 24, buf);
  
  // Draw trend graph
  u8g2->drawStr(2, 36, "24h Trend:");
  for (uint8_t i = 0; i < 24; i++) {
    if (sensor->trendData[i] > 0) {
      uint8_t height = map(sensor->trendData[i], 0, 100, 0, 24);
      u8g2->drawVLine(40 + i * 3, 62 - height, height);
    }
  }
  
  u8g2->setFont(u8g2_font_4x6_tf);
  u8g2->drawStr(118, 62, "4/5");
  
  u8g2->sendBuffer();
}

void AirQualityDisplay::displayComparisonScreen() {
  u8g2->clearBuffer();
  char buf[32];
  
  u8g2->setFont(u8g2_font_helvR08_tf);
  sprintf(buf, "Your PM2.5: %u", sensor->currentData.pm2_5_atm);
  u8g2->drawStr(2, 10, buf);
  u8g2->drawStr(2, 20, "WHO Safe:   10 ug/m3");
  u8g2->drawStr(2, 30, "US EPA:     35 ug/m3");
  u8g2->drawStr(2, 40, "London:     15 ug/m3");
  u8g2->drawStr(2, 50, "Delhi:     100 ug/m3");
  u8g2->drawStr(2, 60, "Beijing:    60 ug/m3");
  
  u8g2->setFont(u8g2_font_4x6_tf);
  u8g2->drawStr(118, 6, "5/5");
  
  u8g2->sendBuffer();
}

void AirQualityDisplay::displayParticlesScreen() {
  u8g2->clearBuffer();
  char buf[32];
  
  u8g2->setFont(u8g2_font_helvR08_tf);
  u8g2->drawStr(2, 10, "Particles/0.1L:");
  
  sprintf(buf, ">0.3um: %u", sensor->currentData.particles_03);
  u8g2->drawStr(2, 22, buf);
  
  sprintf(buf, ">0.5um: %u", sensor->currentData.particles_05);
  u8g2->drawStr(2, 32, buf);
  
  sprintf(buf, ">1.0um: %u", sensor->currentData.particles_10);
  u8g2->drawStr(2, 42, buf);
  
  sprintf(buf, ">2.5um: %u", sensor->currentData.particles_25);
  u8g2->drawStr(2, 52, buf);
  
  sprintf(buf, "VOC Index: %u", sensor->getVOCIndex());
  u8g2->drawStr(2, 62, buf);
  
  u8g2->setFont(u8g2_font_4x6_tf);
  u8g2->drawStr(118, 6, "6/6");
  
  u8g2->sendBuffer();
}

void AirQualityDisplay::checkAlerts() {
  uint16_t pm25 = sensor->currentData.pm2_5_atm;
  uint8_t vocIndex = sensor->getVOCIndex();
  
  if ((pm25 > 55 || vocIndex > 80) && !alertActive) {
    // Sound buzzer for alert
    for (int i = 0; i < 3; i++) {
      digitalWrite(BUZZER_PIN, HIGH);
      delay(200);
      digitalWrite(BUZZER_PIN, LOW);
      delay(200);
    }
    
    alertActive = true;
    currentScreen = ALERT;
    Serial.println("ALERT: Unhealthy air quality detected!");
  } else if (pm25 <= 35 && vocIndex <= 60) {
    alertActive = false;
  }
}

void AirQualityDisplay::rotateScreen() {
  // Don't rotate during alerts or health risk warnings
  if (currentScreen == ALERT || (currentScreen == HEALTH_RISK && sensor->getRiskLevel() == "HIGH")) {
    return;
  }
  
  // Rotate screens every 8 seconds
  if (millis() - lastScreenChange >= 8000) {
    switch (currentScreen) {
      case MAIN:
        currentScreen = HEALTH_RISK;
        break;
      case HEALTH_RISK:
        currentScreen = TREND;
        break;
      case TREND:
        currentScreen = COMPARISON;
        break;
      case COMPARISON:
        currentScreen = PARTICLES;
        break;
      case PARTICLES:
        currentScreen = MAIN;
        break;
      default:
        currentScreen = MAIN;
        break;
    }
    lastScreenChange = millis();
    Serial.print("Screen rotated to: ");
    Serial.println(currentScreen);
  }
}

ScreenMode AirQualityDisplay::getCurrentScreen() {
  return currentScreen;
}

void AirQualityDisplay::setScreen(ScreenMode screen) {
  currentScreen = screen;
  lastScreenChange = millis();
}