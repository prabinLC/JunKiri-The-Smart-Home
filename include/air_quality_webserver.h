#ifndef AIR_QUALITY_WEBSERVER_H
#define AIR_QUALITY_WEBSERVER_H

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ArduinoJson.h>
#include "pms_sensor.h"
#include "air_quality_display.h"

#define WEB_SERVER_PORT 80

class AirQualityWebServer {
private:
  ESP8266WebServer* server;
  PMSSensor* sensor;
  AirQualityDisplay* display;
  
  void handleRoot();
  void handleControl();
  void handleAPI();
  void handleData();
  void handleMetrics();
  void handleParticles();
  void handleLEDToggle();
  void handleLEDOn();
  void handleLEDOff();
  void handleNotFound();
  String getDataHTML();
  String getCommonCSS();
  
public:
  AirQualityWebServer(PMSSensor* pmsSensor, AirQualityDisplay* airDisplay);
  ~AirQualityWebServer();
  void begin(const char* ssid, const char* password);
  void handleClient();
  bool isWiFiConnected();
  String getIPAddress();
};

// External LED control functions (defined in main.cpp)
extern void setLED(bool state);
extern void toggleLED();
extern bool getLEDState();

#endif