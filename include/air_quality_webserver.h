#ifndef AIR_QUALITY_WEBSERVER_H
#define AIR_QUALITY_WEBSERVER_H

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include "pms_sensor.h"
#include "air_quality_display.h"

class AirQualityWebServer {
public:
    AirQualityWebServer(PMSSensor* pmsSensor, AirQualityDisplay* airDisplay);
    void begin(const char* ssid, const char* password);
    void handleClient();
    bool isWiFiConnected();
    String getIPAddress();

private:
    void handleRoot();
    void handleAirQuality();
    void handleAPIData();
    ESP8266WebServer server;
    PMSSensor* sensor;
    AirQualityDisplay* display;
};

#endif // AIR_QUALITY_WEBSERVER_H
