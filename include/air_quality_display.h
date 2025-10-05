#ifndef AIR_QUALITY_DISPLAY_H
#define AIR_QUALITY_DISPLAY_H

#include <Arduino.h>
#include <U8g2lib.h>
#include "pms_sensor.h"

// Pin definitions
#define BUZZER_PIN D5  // Buzzer positive to D5

// Screen modes for OLED
enum ScreenMode { 
  MAIN, 
  HEALTH_RISK, 
  ALERT, 
  TREND, 
  COMPARISON,
  PARTICLES
};

class AirQualityDisplay {
private:
  U8G2_SH1106_128X64_NONAME_F_HW_I2C* u8g2;
  PMSSensor* sensor;
  ScreenMode currentScreen;
  unsigned long lastScreenChange;
  bool alertActive;
  
public:
  AirQualityDisplay(PMSSensor* pmsSensor);
  ~AirQualityDisplay();
  void begin();
  void update();
  void displayMainScreen();
  void displayHealthRiskScreen();
  void displayAlertScreen();
  void displayTrendScreen();
  void displayComparisonScreen();
  void displayParticlesScreen();
  void checkAlerts();
  void rotateScreen();
  ScreenMode getCurrentScreen();
  void setScreen(ScreenMode screen);
  void showBootScreen();
};

#endif