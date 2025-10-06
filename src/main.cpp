#include <Arduino.h>
#include <Servo.h>
#include "pms_sensor.h"
#include "air_quality_display.h"
#include "air_quality_webserver.h"

// WiFi Configuration - Update with your credentials
const char* WIFI_SSID = "Kalo phone";    // Your WiFi network name
const char* WIFI_PASS = "12345678";      // Your WiFi password

// Pin definitions
#define LED_PIN D0       // Single LED
// D1 = SCL (Display)
// D2 = SDA (Display)  
// D3 = TXD (PMS5003 sensor)
// D4 = RST (PMS5003 sensor)
#define SERVO_PIN D5     // Servo motor

// Component states
bool ledState = false;
int servoPosition = 0;

// Global objects
Servo doorServo;
PMSSensor airSensor;
AirQualityDisplay airDisplay(&airSensor);
AirQualityWebServer webServer(&airSensor, &airDisplay);

// Timing variables
unsigned long lastSensorRead = 0;
unsigned long lastDisplayUpdate = 0;
unsigned long lastSerialOutput = 0;

// LED control functions
void setLED(bool state) {
  ledState = state;
  digitalWrite(LED_PIN, state ? HIGH : LOW);
  Serial.println("LED: " + String(state ? "ON" : "OFF"));
}

void toggleLED() {
  setLED(!ledState);
}

bool getLEDState() {
  return ledState;
}

// Servo control functions
void setServoPosition(int angle) {
  servoPosition = angle;
  doorServo.write(angle);
  Serial.println("Servo position: " + String(angle));
}

int getServoPosition() {
  return servoPosition;
}

// Air quality alert function
void checkAirQualityAlerts() {
  if (airSensor.isDataValid()) {
    float pm25 = airSensor.currentData.pm2_5_atm;
    
    // Alert thresholds
    if (pm25 > 55) { // Unhealthy level
      // Blink LED rapidly for warning
      for(int i = 0; i < 5; i++) {
        setLED(true);
        delay(200);
        setLED(false);
        delay(200);
      }
      Serial.println("⚠️ AIR QUALITY ALERT: Unhealthy PM2.5 level detected: " + String(pm25));
    }
  }
}

void setup() {
  // Initialize serial communication
  Serial.begin(115200);
  Serial.println();
  Serial.println("=========================================");
  Serial.println("    PMS5003 Air Quality Monitor v2.0");
  Serial.println("    ESP8266 + OLED + Web Interface");
  Serial.println("=========================================");
  
  // Initialize LED pin
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  Serial.println("LED initialized on pin D0");
  
  // Test LED functionality
  Serial.println("Testing LED...");
  setLED(true);
  delay(2000);
  setLED(false);
  delay(1000);
  Serial.println("LED test complete!");
  
  // Initialize servo
  doorServo.attach(SERVO_PIN);
  doorServo.write(0);
  Serial.println("Servo initialized on pin D5");
  
  // Initialize display (D1=SCL, D2=SDA)
  Serial.println("Initializing OLED display...");
  airDisplay.begin();
  delay(2000);
  
  // Initialize PMS5003 sensor (D3=TXD, D4=RST)
  Serial.println("Initializing PMS5003 sensor...");
  airSensor.begin();
  delay(1000);
  
  // Initialize web server
  Serial.println("Starting web server...");
  webServer.begin(WIFI_SSID, WIFI_PASS);
  
  Serial.println("Setup complete!");
  Serial.println("=========================================");
  if (webServer.isWiFiConnected()) {
    Serial.print("Web dashboard: http://");
    Serial.println(webServer.getIPAddress());
  }
  Serial.println("=========================================");
  
  // Initialize timing
  lastSensorRead = millis();
  lastDisplayUpdate = millis();
  lastSerialOutput = millis();
}

void loop() {
  // Check WiFi connection status periodically
  static unsigned long lastWiFiCheck = 0;
  if (millis() - lastWiFiCheck >= 60000) { // Check every minute
    lastWiFiCheck = millis();
    if (WiFi.status() != WL_CONNECTED) {
      Serial.println("WARNING: WiFi connection lost!");
      Serial.print("WiFi status: ");
      Serial.println(WiFi.status());
    } else {
      Serial.print("WiFi OK - IP: ");
      Serial.print(WiFi.localIP());
      Serial.print(", RSSI: ");
      Serial.print(WiFi.RSSI());
      Serial.println(" dBm");
    }
  }
  
  // Handle web server requests
  webServer.handleClient();
  
  // Read sensor data every 30 seconds
  if (millis() - lastSensorRead >= 30000) {
    Serial.println("Reading PMS5003 sensor data...");
    
    if (airSensor.readData()) {
      airSensor.updateTrend();
      Serial.println("Sensor data updated successfully");
      
      // Check for air quality alerts
      checkAirQualityAlerts();
      
      // Print detailed data every 2 minutes
      if (millis() - lastSerialOutput >= 120000) {
        airSensor.printData();
        lastSerialOutput = millis();
      }
    } else {
      Serial.println("Failed to read sensor data - check connections");
    }
    
    lastSensorRead = millis();
  }
  
  // Update display every 100ms
  if (millis() - lastDisplayUpdate >= 100) {
    airDisplay.update();
    lastDisplayUpdate = millis();
  }
  
  // Small delay to prevent overwhelming the system
  delay(50);
}