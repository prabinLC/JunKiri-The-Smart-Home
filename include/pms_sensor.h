#ifndef PMS_SENSOR_H
#define PMS_SENSOR_H

#include <Arduino.h>
#include <SoftwareSerial.h>
#include <PMS.h>

// Pin definitions for PMS5003
#define PMS5003_RX_PIN D3  // PMS5003 TX to D3 (RX)
#define PMS5003_TX_PIN D4  // PMS5003 RX to D4 (TX)

class PMSSensor {
private:
  SoftwareSerial* pmsSerial;
  PMS* pms;
  unsigned long lastReadTime;
  unsigned long readInterval;
  
public:
  // Data structure for air quality readings
  struct AirQualityData {
    uint16_t pm1_0_cf1;     // PM1.0 CF=1 (µg/m³)
    uint16_t pm2_5_cf1;     // PM2.5 CF=1 (µg/m³)
    uint16_t pm10_cf1;      // PM10 CF=1 (µg/m³)
    uint16_t pm1_0_atm;     // PM1.0 atmospheric (µg/m³)
    uint16_t pm2_5_atm;     // PM2.5 atmospheric (µg/m³)
    uint16_t pm10_atm;      // PM10 atmospheric (µg/m³)
    uint16_t particles_03;  // Particles > 0.3µm per 0.1L air
    uint16_t particles_05;  // Particles > 0.5µm per 0.1L air
    uint16_t particles_10;  // Particles > 1.0µm per 0.1L air
    uint16_t particles_25;  // Particles > 2.5µm per 0.1L air
    uint16_t particles_50;  // Particles > 5.0µm per 0.1L air
    uint16_t particles_100; // Particles > 10µm per 0.1L air
    bool isValid;           // Data validity flag
  };
  
  AirQualityData currentData;
  float trendData[24];        // 24-hour PM2.5 trend
  uint8_t trendIndex;
  bool trendInitialized;
  
  PMSSensor();
  ~PMSSensor();
  void begin();
  bool readData();
  void updateTrend();
  String getHealthStatus();
  String getRiskLevel();
  uint8_t getVOCIndex();
  void printData();
  bool isDataValid();
  unsigned long getLastReadTime();
  
  // Demo data methods for when sensor is not available
  float getDemoPM25();
  float getDemoPM10();
  String getDemoHealthStatus();
  String getDemoRiskLevel();
};

#endif