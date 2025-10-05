# JunKiri - The Smart Home System

## 🏠 Air Quality Monitoring & Control System

A comprehensive IoT solution built with ESP8266 NodeMCU for real-time air quality monitoring and smart home control.

### 🌟 Features

#### 📊 Air Quality Monitoring

- **Real-time PM2.5, PM10, and VOC measurements** using PMS5003 sensor
- **Health status indicators** with color-coded risk levels
- **Professional Chart.js visualizations**:
  - Doughnut chart showing particle distribution vs safe zones
  - Real-time line chart tracking PM2.5 and PM10 trends
- **Auto-refresh every 5 seconds** with live data updates

#### 🌐 Multi-Page Web Interface

- **Main Dashboard** (`/`) - Air quality monitor with charts and metrics
- **Control Panel** (`/control`) - LED and smart home device control
- **JSON API** (`/api`) - Complete sensor data and system metrics
- **Responsive design** with modern dark theme and blue gradients

#### ⚡ Smart Home Control

- **LED control** with toggle, ON/OFF endpoints
- **Real-time status updates** without page reloads
- **Extensible architecture** for additional smart devices

#### 📱 System Information

- WiFi signal strength monitoring
- System uptime and memory usage
- IP address and network details

### 🔧 Hardware Components

- **ESP8266 NodeMCU** - Main microcontroller
- **PMS5003** - Air quality sensor (PM1.0, PM2.5, PM10)
- **SH1106 OLED Display** (128x64) - Local status display
- **LED** - Status indicator and control demo
- **Power Supply** - 5V via USB or external

### 📋 Pin Configuration

```
PMS5003 Sensor:
- VCC → 5V
- GND → GND
- RX  → D3 (GPIO0)
- TX  → D4 (GPIO2)

SH1106 OLED (I2C):
- VCC → 3.3V
- GND → GND
- SCL → D1 (GPIO5)
- SDA → D2 (GPIO4)

LED Control:
- LED → D0 (GPIO16)
- GND → GND
```

### 🚀 Getting Started

#### Prerequisites

- PlatformIO IDE or Arduino IDE
- ESP8266 Arduino Core
- Required libraries (see `platformio.ini`)

#### Installation

1. **Clone the repository**

```bash
git clone https://github.com/prabinLC/JunKiri-The-Smart-Home.git
cd JunKiri-The-Smart-Home
```

2. **Configure WiFi credentials**
   Edit `src/main.cpp` and update:

```cpp
const char* ssid = "YOUR_WIFI_SSID";
const char* password = "YOUR_WIFI_PASSWORD";
```

3. **Upload to ESP8266**

```bash
platformio run --target upload
```

4. **Monitor serial output**

```bash
platformio device monitor
```

### 📚 Libraries Used

- **PMS Library** (1.1.0) - PMS5003 sensor communication
- **U8g2** (2.36.12) - OLED display driver
- **ArduinoJson** (6.21.5) - JSON data handling
- **ESP8266WebServer** - Web server functionality
- **ESP8266WiFi** - WiFi connectivity
- **SoftwareSerial** - UART communication

### 🌐 Web Interface

#### Main Dashboard

- Real-time sensor data visualization
- Interactive Chart.js charts
- Health status with emoji indicators
- System information panel

#### Control Panel

- LED toggle functionality
- Real-time status updates
- Navigation back to dashboard

#### API Endpoints

- `GET /` - Main dashboard
- `GET /control` - Control panel
- `GET /api` - JSON sensor data
- `POST /toggle` - Toggle LED
- `POST /led/on` - Turn LED ON
- `POST /led/off` - Turn LED OFF

### 📊 Data Format

#### API Response Example

```json
{
  "valid": true,
  "pm1_0": 12.3,
  "pm2_5": 15.7,
  "pm10": 18.9,
  "voc_index": 42,
  "health_status": "Good",
  "risk_level": "LOW",
  "particles": {
    "0_3um": 1234,
    "0_5um": 567,
    "1_0um": 234,
    "2_5um": 89,
    "5_0um": 12,
    "10_0um": 3
  },
  "led_state": true,
  "timestamp": 1234567890,
  "uptime": 3600,
  "free_memory": 45678,
  "wifi_rssi": -42
}
```

### 🏗️ Project Structure

```
JunKiri-The-Smart-Home/
├── platformio.ini          # PlatformIO configuration
├── src/
│   ├── main.cpp            # Main application entry point
│   ├── air_quality_webserver.cpp  # Web server implementation
│   ├── air_quality_webserver.h    # Web server header
│   ├── pms_sensor.cpp      # PMS5003 sensor handling
│   ├── pms_sensor.h        # PMS sensor header
│   ├── air_quality_display.cpp    # OLED display management
│   └── air_quality_display.h      # Display header
├── include/                # Header files
├── lib/                    # Local libraries
├── test/                   # Unit tests
└── README.md              # This file
```

### 🎨 Features Highlights

- **Professional Chart.js Integration** - Beautiful, responsive charts
- **Real-time Data Updates** - Live sensor readings every 5 seconds
- **Modern Dark Theme** - Professional UI with blue gradients
- **Multi-page Navigation** - Separate pages for monitoring and control
- **Mobile Responsive** - Works great on phones and tablets
- **Extensible Architecture** - Easy to add new sensors and controls

### 🔧 Development

#### Building

```bash
platformio run
```

#### Uploading

```bash
platformio run --target upload
```

#### Monitoring

```bash
platformio device monitor
```

### 📝 License

This project is open source and available under the [MIT License](LICENSE).

### 🤝 Contributing

Contributions are welcome! Please feel free to submit a Pull Request.

### 📞 Support

If you encounter any issues or have questions, please open an issue on GitHub.

---

**Built with ❤️ for smart home enthusiasts and air quality awareness**
