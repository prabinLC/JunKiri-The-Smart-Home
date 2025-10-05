#include <Arduino.h>
#include "pms_sensor.h"
#include "air_quality_display.h"
#include "air_quality_webserver.h"

// WiFi Configuration - Update with your credentials
const char* WIFI_SSID = "Kalo phone";    // Your WiFi network name
const char* WIFI_PASS = "12345678";      // Your WiFi password

// LED Control Pin
#define LED_PIN D0  // External LED connected to D0 (GPIO 16)

// Global objects
PMSSensor airSensor;
AirQualityDisplay airDisplay(&airSensor);
AirQualityWebServer webServer(&airSensor, &airDisplay);

// LED control variables
bool ledState = false;

// Timing variables
unsigned long lastSensorRead = 0;
unsigned long lastDisplayUpdate = 0;
unsigned long lastSerialOutput = 0;

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
  digitalWrite(LED_PIN, LOW);  // Turn off LED initially
  Serial.println("LED control initialized on pin D0");
  
  // Test LED functionality during setup
  Serial.println("Testing LED functionality...");
  Serial.println("Turning LED ON for 2 seconds...");
  setLED(true);
  delay(2000);
  Serial.println("Turning LED OFF for 1 second...");
  setLED(false);
  delay(1000);
  Serial.println("LED test complete!");
  
  // Initialize display first (shows boot screen)
  Serial.println("Initializing OLED display...");
  airDisplay.begin();
  delay(2000);
  
  // Initialize PMS5003 sensor
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
    Serial.print("JSON API: http://");
    Serial.print(webServer.getIPAddress());
    Serial.println("/api");
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

// Additional helper functions for debugging
void printSystemStatus() {
  Serial.println("\n=== System Status ===");
  Serial.print("WiFi Status: ");
  Serial.println(webServer.isWiFiConnected() ? "Connected" : "Disconnected");
  if (webServer.isWiFiConnected()) {
    Serial.print("IP Address: ");
    Serial.println(webServer.getIPAddress());
  }
  Serial.print("Sensor Data Valid: ");
  Serial.println(airSensor.isDataValid() ? "Yes" : "No");
  Serial.print("Current Display Screen: ");
  Serial.println(airDisplay.getCurrentScreen());
  Serial.print("LED Status: ");
  Serial.println(ledState ? "ON" : "OFF");
  Serial.print("Free Heap: ");
  Serial.print(ESP.getFreeHeap());
  Serial.println(" bytes");
  Serial.print("Uptime: ");
  Serial.print(millis() / 1000);
  Serial.println(" seconds");
  Serial.println("====================\n");
}

// LED control functions
void setLED(bool state) {
  Serial.print("setLED() called with state: ");
  Serial.println(state ? "HIGH" : "LOW");
  
  ledState = state;
  digitalWrite(LED_PIN, state ? HIGH : LOW);
  
  // Read back the pin state to verify
  int pinState = digitalRead(LED_PIN);
  Serial.print("Pin D0 (GPIO16) physical state: ");
  Serial.println(pinState ? "HIGH" : "LOW");
  
  Serial.print("LED turned ");
  Serial.println(state ? "ON" : "OFF");
}

void toggleLED() {
  Serial.print("toggleLED() called. Current state: ");
  Serial.println(ledState ? "ON" : "OFF");
  setLED(!ledState);
}

bool getLEDState() {
  Serial.print("getLEDState() called. Returning: ");
  Serial.println(ledState ? "true" : "false");
  return ledState;
}

// Print system status every 5 minutes
void checkSystemHealth() {
  static unsigned long lastHealthCheck = 0;
  if (millis() - lastHealthCheck >= 300000) {  // 5 minutes
    printSystemStatus();
    lastHealthCheck = millis();
  }
}