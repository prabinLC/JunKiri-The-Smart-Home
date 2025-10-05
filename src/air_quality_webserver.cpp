#include "air_quality_webserver.h"

AirQualityWebServer::AirQualityWebServer(PMSSensor* pmsSensor, AirQualityDisplay* airDisplay) {
  sensor = pmsSensor;
  display = airDisplay;
  server = new ESP8266WebServer(80);
}

AirQualityWebServer::~AirQualityWebServer() {
  delete server;
}

void AirQualityWebServer::begin(const char* ssid, const char* password) {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
  }
  
  server->on("/", [this]() { handleRoot(); });
  server->on("/control", [this]() { handleControl(); });
  server->on("/api", [this]() { handleAPI(); });
  server->on("/toggle", HTTP_POST, [this]() { handleLEDToggle(); });
  server->on("/led/on", HTTP_POST, [this]() { handleLEDOn(); });
  server->on("/led/off", HTTP_POST, [this]() { handleLEDOff(); });
  
  server->begin();
}

void AirQualityWebServer::handleRoot() {
  // Get current sensor values (real or demo)
  float pm25 = sensor->isDataValid() ? sensor->currentData.pm2_5_atm : sensor->getDemoPM25();
  float pm10 = sensor->isDataValid() ? sensor->currentData.pm10_atm : sensor->getDemoPM10();
  float vocIndex = sensor->getVOCIndex();
  
  String html = "<!DOCTYPE html><html><head>";
  html += "<title>Junkari - Air Quality Monitor</title>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  html += "<script src='https://cdn.jsdelivr.net/npm/chart.js'></script>";
  
  // Beautiful purple gradient styling
  html += "<style>";
  html += "* { margin: 0; padding: 0; box-sizing: border-box; }";
  html += "body { background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); min-height: 100vh; font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; color: white; }";
  html += ".container { max-width: 1400px; margin: 0 auto; padding: 20px; }";
  
  // Header with Junkari branding
  html += ".header { display: flex; align-items: center; justify-content: space-between; background: rgba(255,255,255,0.1); backdrop-filter: blur(10px); border-radius: 20px; padding: 20px 30px; margin-bottom: 30px; border: 1px solid rgba(255,255,255,0.2); }";
  html += ".logo-section { display: flex; align-items: center; gap: 15px; }";
  html += ".logo { width: 60px; height: 60px; background: rgba(255,255,255,0.2); border-radius: 15px; display: flex; align-items: center; justify-content: center; font-size: 24px; }";
  html += ".brand-text h1 { font-size: 2rem; font-weight: 700; margin-bottom: 5px; }";
  html += ".brand-text p { font-size: 1rem; opacity: 0.8; }";
  
  // Navigation buttons
  html += ".nav-buttons { display: flex; gap: 15px; }";
  html += ".nav-btn { background: rgba(255,255,255,0.2); color: white; padding: 12px 24px; text-decoration: none; border-radius: 12px; font-weight: 500; transition: all 0.3s ease; backdrop-filter: blur(10px); border: 1px solid rgba(255,255,255,0.3); }";
  html += ".nav-btn:hover { background: rgba(255,255,255,0.3); transform: translateY(-2px); }";
  
  // Metric cards with colors from mockup
  html += ".metrics-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(280px, 1fr)); gap: 20px; margin-bottom: 30px; }";
  html += ".metric-card { background: rgba(255,255,255,0.1); backdrop-filter: blur(10px); border-radius: 20px; padding: 25px; text-align: center; border: 1px solid rgba(255,255,255,0.2); position: relative; overflow: hidden; }";
  html += ".metric-card::before { content: ''; position: absolute; top: 0; left: 0; right: 0; height: 4px; }";
  
  // PM2.5 - Orange theme
  html += ".pm25-card { background: linear-gradient(135deg, #ff9a9e 0%, #fecfef 50%, #fecfef 100%); }";
  html += ".pm25-card::before { background: linear-gradient(90deg, #ff6b6b, #ff8e8e); }";
  
  // PM10 - Teal theme  
  html += ".pm10-card { background: linear-gradient(135deg, #a8edea 0%, #fed6e3 100%); }";
  html += ".pm10-card::before { background: linear-gradient(90deg, #4ecdc4, #44a08d); }";
  
  // VOC - Yellow theme
  html += ".voc-card { background: linear-gradient(135deg, #ffecd2 0%, #fcb69f 100%); }";
  html += ".voc-card::before { background: linear-gradient(90deg, #f093fb, #f5576c); }";
  
  html += ".metric-icon { font-size: 2rem; margin-bottom: 10px; opacity: 0.8; }";
  html += ".metric-title { font-size: 1.1rem; margin-bottom: 15px; font-weight: 600; }";
  html += ".metric-value { font-size: 2.5rem; font-weight: 700; margin-bottom: 5px; }";
  html += ".metric-unit { font-size: 1rem; opacity: 0.8; margin-bottom: 10px; }";
  html += ".metric-status { display: inline-block; padding: 6px 12px; border-radius: 20px; font-size: 0.9rem; font-weight: 600; }";
  html += ".status-good { background: #10b981; color: white; }";
  html += ".status-moderate { background: #f59e0b; color: white; }";
  html += ".status-poor { background: #ef4444; color: white; }";
  
  // Chart containers
  html += ".chart-section { background: rgba(255,255,255,0.1); backdrop-filter: blur(10px); border-radius: 20px; padding: 25px; margin-bottom: 20px; border: 1px solid rgba(255,255,255,0.2); }";
  html += ".chart-title { font-size: 1.4rem; font-weight: 600; margin-bottom: 20px; }";
  html += ".chart-container { position: relative; height: 300px; }";
  
  // Info tooltips
  html += ".info-tooltip { position: relative; display: inline-block; margin-left: 8px; cursor: help; }";
  html += ".info-tooltip::after { content: 'ℹ️'; font-size: 0.9rem; }";
  html += ".tooltip-text { visibility: hidden; width: 300px; background: rgba(0,0,0,0.9); color: white; text-align: left; border-radius: 8px; padding: 15px; position: absolute; z-index: 1; bottom: 125%; left: 50%; margin-left: -150px; opacity: 0; transition: opacity 0.3s; font-size: 0.9rem; line-height: 1.4; }";
  html += ".info-tooltip:hover .tooltip-text { visibility: visible; opacity: 1; }";
  html += ".tooltip-text::after { content: ''; position: absolute; top: 100%; left: 50%; margin-left: -5px; border-width: 5px; border-style: solid; border-color: rgba(0,0,0,0.9) transparent transparent transparent; }";
  html += "</style>";
  html += "</head><body>";
  
  html += "<div class='container'>";
  
  // Header with Junkari branding
  html += "<div class='header'>";
  html += "<div class='logo-section'>";
  html += "<div class='logo'>💨</div>";
  html += "<div class='brand-text'>";
  html += "<h1>Junkari</h1>";
  html += "<p>Smart Air Quality Monitoring</p>";
  html += "</div></div>";
  html += "<div class='nav-buttons'>";
  html += "<a href='/' class='nav-btn'>🏠 Home</a>";
  html += "<a href='/control' class='nav-btn'>⚙️ Controls</a>";
  html += "<a href='/api' class='nav-btn'>📊 API Data</a>";
  html += "<a href='javascript:updateCharts()' class='nav-btn'>🔄 Refresh</a>";
  html += "</div></div>";
  
  // Metrics Grid with colorful cards
  html += "<div class='metrics-grid'>";
  
  // PM2.5 Card (Orange theme)
  html += "<div class='metric-card pm25-card'>";
  html += "<div class='metric-icon'>🔴</div>";
  html += "<div class='metric-title'>PM2.5 Level";
  html += "<span class='info-tooltip'>";
  html += "<span class='tooltip-text'>PM2.5 particles are tiny pollutants (2.5 micrometers or smaller) that can penetrate deep into lungs and enter bloodstream. Safe level: <12 μg/m³</span>";
  html += "</span></div>";
  html += "<div class='metric-value'>" + String(pm25) + "</div>";
  html += "<div class='metric-unit'>μg/m³</div>";
  
  String pm25Status = "Good";
  String pm25Class = "status-good";
  if (pm25 > 35) { pm25Status = "Poor"; pm25Class = "status-poor"; }
  else if (pm25 > 12) { pm25Status = "Moderate"; pm25Class = "status-moderate"; }
  html += "<span class='metric-status " + pm25Class + "'>" + pm25Status + "</span>";
  html += "</div>";
  
  // PM10 Card (Teal theme)
  html += "<div class='metric-card pm10-card'>";
  html += "<div class='metric-icon'>🔵</div>";
  html += "<div class='metric-title'>PM10 Level";
  html += "<span class='info-tooltip'>";
  html += "<span class='tooltip-text'>PM10 particles are larger pollutants (10 micrometers or smaller) that affect respiratory system. Safe level: <20 μg/m³</span>";
  html += "</span></div>";
  html += "<div class='metric-value'>" + String(pm10) + "</div>";
  html += "<div class='metric-unit'>μg/m³</div>";
  
  String pm10Status = "Good";
  String pm10Class = "status-good";
  if (pm10 > 50) { pm10Status = "Poor"; pm10Class = "status-poor"; }
  else if (pm10 > 20) { pm10Status = "Moderate"; pm10Class = "status-moderate"; }
  html += "<span class='metric-status " + pm10Class + "'>" + pm10Status + "</span>";
  html += "</div>";
  
  // VOC Card (Yellow theme)
  html += "<div class='metric-card voc-card'>";
  html += "<div class='metric-icon'>🟡</div>";
  html += "<div class='metric-title'>VOC Index";
  html += "<span class='info-tooltip'>";
  html += "<span class='tooltip-text'>Volatile Organic Compounds indicate air freshness and chemical pollution. Index 1-5: Excellent to Poor air quality</span>";
  html += "</span></div>";
  html += "<div class='metric-value'>" + String(vocIndex, 1) + "</div>";
  html += "<div class='metric-unit'>Index</div>";
  
  String vocStatus = "Excellent";
  String vocClass = "status-good";
  if (vocIndex > 4) { vocStatus = "Poor"; vocClass = "status-poor"; }
  else if (vocIndex > 2) { vocStatus = "Moderate"; vocClass = "status-moderate"; }
  html += "<span class='metric-status " + vocClass + "'>" + vocStatus + "</span>";
  html += "</div>";
  
  html += "</div>"; // End metrics grid
  
  // Chart sections with beautiful modern styling
  html += "<div class='chart-section'>";
  html += "<div class='chart-title'>📈 Real-time Air Quality Trends</div>";
  html += "<div class='chart-container'>";
  html += "<canvas id='lineChart'></canvas>";
  html += "</div></div>";
  
  html += "<div class='chart-section'>";
  html += "<div class='chart-title'>🍰 Air Quality Distribution</div>";
  html += "<div class='chart-container'>";
  html += "<canvas id='doughnutChart'></canvas>";
  html += "</div></div>";
  
  // Enhanced Chart.js with purple theme
  html += "<script>";
  html += "let lineChart, doughnutChart;";
  html += "let trendData = {pm25: [], pm10: [], voc: [], timestamps: []};";
  
  // Initialize charts with beautiful purple theme
  html += "function initCharts() {";
  
  // Line Chart for trends
  html += "  const lineCtx = document.getElementById('lineChart').getContext('2d');";
  html += "  lineChart = new Chart(lineCtx, {";
  html += "    type: 'line',";
  html += "    data: {";
  html += "      labels: [],";
  html += "      datasets: [{";
  html += "        label: 'PM2.5',";
  html += "        data: [],";
  html += "        borderColor: '#ff6b6b',";
  html += "        backgroundColor: 'rgba(255, 107, 107, 0.1)',";
  html += "        tension: 0.4,";
  html += "        fill: true";
  html += "      }, {";
  html += "        label: 'PM10',";
  html += "        data: [],";
  html += "        borderColor: '#4ecdc4',";
  html += "        backgroundColor: 'rgba(78, 205, 196, 0.1)',";
  html += "        tension: 0.4,";
  html += "        fill: true";
  html += "      }]";
  html += "    },";
  html += "    options: {";
  html += "      responsive: true,";
  html += "      maintainAspectRatio: false,";
  html += "      plugins: {";
  html += "        legend: { labels: { color: 'white' } }";
  html += "      },";
  html += "      scales: {";
  html += "        y: { ticks: { color: 'white' }, grid: { color: 'rgba(255,255,255,0.2)' } },";
  html += "        x: { ticks: { color: 'white' }, grid: { color: 'rgba(255,255,255,0.2)' } }";
  html += "      }";
  html += "    }";
  html += "  });";
  
  // Doughnut Chart for distribution
  html += "  const doughnutCtx = document.getElementById('doughnutChart').getContext('2d');";
  html += "  doughnutChart = new Chart(doughnutCtx, {";
  html += "    type: 'doughnut',";
  html += "    data: {";
  html += "      labels: ['PM2.5', 'PM10', 'VOC Index'],";
  html += "      datasets: [{";
  html += "        data: [" + String(pm25) + ", " + String(pm10) + ", " + String(vocIndex * 10, 1) + "],";
  html += "        backgroundColor: ['#ff6b6b', '#4ecdc4', '#ffd93d'],";
  html += "        borderColor: ['#ff5252', '#26a69a', '#ffc107'],";
  html += "        borderWidth: 3";
  html += "      }]";
  html += "    },";
  html += "    options: {";
  html += "      responsive: true,";
  html += "      maintainAspectRatio: false,";
  html += "      plugins: {";
  html += "        legend: { labels: { color: 'white', font: { size: 14 } } }";
  html += "      }";
  html += "    }";
  html += "  });";
  html += "};";
  
  // Update charts function
  html += "function updateCharts() {";
  html += "  fetch('/api').then(response => response.json()).then(data => {";
  html += "    const now = new Date().toLocaleTimeString();";
  html += "    ";
  html += "    // Update line chart";
  html += "    lineChart.data.labels.push(now);";
  html += "    lineChart.data.datasets[0].data.push(data.pm2_5);";
  html += "    lineChart.data.datasets[1].data.push(data.pm10);";
  html += "    ";
  html += "    // Keep only last 10 points";
  html += "    if (lineChart.data.labels.length > 10) {";
  html += "      lineChart.data.labels.shift();";
  html += "      lineChart.data.datasets[0].data.shift();";
  html += "      lineChart.data.datasets[1].data.shift();";
  html += "    }";
  html += "    ";
  html += "    // Update doughnut chart";
  html += "    doughnutChart.data.datasets[0].data = [data.pm2_5, data.pm10, data.voc_index * 10];";
  html += "    ";
  html += "    lineChart.update();";
  html += "    doughnutChart.update();";
  html += "    ";
  html += "    // Update metric cards";
  html += "    document.querySelectorAll('.metric-value')[0].textContent = data.pm2_5;";
  html += "    document.querySelectorAll('.metric-value')[1].textContent = data.pm10;";
  html += "    document.querySelectorAll('.metric-value')[2].textContent = data.voc_index.toFixed(1);";
  html += "  });";
  html += "};";
  
  html += "document.addEventListener('DOMContentLoaded', function() {";
  html += "  initCharts();";
  html += "  setInterval(updateCharts, 5000);";
  html += "});";
  
  html += "</script>";
  html += "</body></html>";
  
  server->send(200, "text/html", html);
}

void AirQualityWebServer::handleControl() {
  String html = "<!DOCTYPE html><html><head>";
  html += "<title>Junkari - Controls</title>";
  html += "<meta name='viewport' content='width=device-width, initial-scale=1.0'>";
  
  // Use same purple gradient styling
  html += "<style>";
  html += "* { margin: 0; padding: 0; box-sizing: border-box; }";
  html += "body { background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); min-height: 100vh; font-family: 'Segoe UI', Tahoma, Geneva, Verdana, sans-serif; color: white; }";
  html += ".container { max-width: 1000px; margin: 0 auto; padding: 20px; }";
  
  // Header styling
  html += ".header { display: flex; align-items: center; justify-content: space-between; background: rgba(255,255,255,0.1); backdrop-filter: blur(10px); border-radius: 20px; padding: 20px 30px; margin-bottom: 30px; border: 1px solid rgba(255,255,255,0.2); }";
  html += ".logo-section { display: flex; align-items: center; gap: 15px; }";
  html += ".logo { width: 60px; height: 60px; background: rgba(255,255,255,0.2); border-radius: 15px; display: flex; align-items: center; justify-content: center; font-size: 24px; }";
  html += ".brand-text h1 { font-size: 2rem; font-weight: 700; margin-bottom: 5px; }";
  html += ".brand-text p { font-size: 1rem; opacity: 0.8; }";
  
  // Control cards
  html += ".control-card { background: rgba(255,255,255,0.1); backdrop-filter: blur(10px); border-radius: 20px; padding: 30px; margin-bottom: 20px; border: 1px solid rgba(255,255,255,0.2); text-align: center; }";
  html += ".control-title { font-size: 1.5rem; font-weight: 600; margin-bottom: 20px; }";
  
  // LED control button
  html += ".led-button { background: linear-gradient(135deg, #10b981, #059669); color: white; border: none; padding: 15px 40px; font-size: 1.2rem; font-weight: 600; border-radius: 12px; cursor: pointer; transition: all 0.3s ease; margin: 10px; }";
  html += ".led-button:hover { transform: translateY(-2px); box-shadow: 0 8px 25px rgba(16, 185, 129, 0.3); }";
  html += ".led-status { margin-top: 15px; font-size: 1.1rem; padding: 8px 16px; border-radius: 20px; display: inline-block; }";
  html += ".status-on { background: rgba(16, 185, 129, 0.2); border: 1px solid #10b981; }";
  html += ".status-off { background: rgba(107, 114, 128, 0.2); border: 1px solid #6b7280; }";
  
  // Navigation
  html += ".nav-btn { background: rgba(255,255,255,0.2); color: white; padding: 12px 24px; text-decoration: none; border-radius: 12px; font-weight: 500; transition: all 0.3s ease; backdrop-filter: blur(10px); border: 1px solid rgba(255,255,255,0.3); }";
  html += ".nav-btn:hover { background: rgba(255,255,255,0.3); transform: translateY(-2px); }";
  html += "</style>";
  html += "</head><body>";
  
  html += "<div class='container'>";
  
  // Header with Junkari branding
  html += "<div class='header'>";
  html += "<div class='logo-section'>";
  html += "<div class='logo'>⚙️</div>";
  html += "<div class='brand-text'>";
  html += "<h1>Junkari</h1>";
  html += "<p>Smart Home Controls</p>";
  html += "</div></div>";
  html += "<div>";
  html += "<a href='/' class='nav-btn'>🏠 Back to Monitor</a>";
  html += "</div></div>";
  
  // LED Control Card
  html += "<div class='control-card'>";
  html += "<div class='control-title'>💡 LED Light Control</div>";
  html += "<button onclick='controlLED()' class='led-button'>Toggle LED</button>";
  
  String ledState = getLEDState() ? "ON" : "OFF";
  String statusClass = getLEDState() ? "status-on" : "status-off";
  html += "<div class='led-status " + statusClass + "'>Status: " + ledState + "</div>";
  html += "</div>";
  
  // System Info Card
  html += "<div class='control-card'>";
  html += "<div class='control-title'>📊 System Information</div>";
  html += "<div style='text-align: left; max-width: 400px; margin: 0 auto;'>";
  html += "<div style='margin: 10px 0; display: flex; justify-content: space-between;'>";
  html += "<span>IP Address:</span> <span>" + getIPAddress() + "</span></div>";
  html += "<div style='margin: 10px 0; display: flex; justify-content: space-between;'>";
  html += "<span>Uptime:</span> <span>" + String(millis() / 1000) + "s</span></div>";
  html += "<div style='margin: 10px 0; display: flex; justify-content: space-between;'>";
  html += "<span>Free RAM:</span> <span>" + String(ESP.getFreeHeap()) + " bytes</span></div>";
  html += "<div style='margin: 10px 0; display: flex; justify-content: space-between;'>";
  html += "<span>WiFi Signal:</span> <span>" + String(WiFi.RSSI()) + " dBm</span></div>";
  html += "</div></div>";
  
  html += "<script>";
  html += "function controlLED() {";
  html += "  fetch('/toggle', { method: 'POST' })";
  html += "    .then(() => location.reload());";
  html += "}";
  html += "</script>";
  
  html += "</div></body></html>";
  server->send(200, "text/html", html);
}

void AirQualityWebServer::handleAPI() {
  DynamicJsonDocument doc(1024);
  
  if (sensor->isDataValid()) {
    doc["valid"] = true;
    doc["pm1_0"] = sensor->currentData.pm1_0_atm;
    doc["pm2_5"] = sensor->currentData.pm2_5_atm;
    doc["pm10"] = sensor->currentData.pm10_atm;
    doc["voc_index"] = sensor->getVOCIndex();
    doc["health_status"] = sensor->getHealthStatus();
    doc["risk_level"] = sensor->getRiskLevel();
    doc["demo_mode"] = false;
    
    // Add particle counts for advanced charts
    JsonObject particles = doc.createNestedObject("particles");
    particles["0_3um"] = sensor->currentData.particles_03;
    particles["0_5um"] = sensor->currentData.particles_05;
    particles["1_0um"] = sensor->currentData.particles_10;
    particles["2_5um"] = sensor->currentData.particles_25;
    particles["5_0um"] = sensor->currentData.particles_50;
    particles["10um"] = sensor->currentData.particles_100;
  } else {
    doc["valid"] = false;
    doc["pm1_0"] = sensor->getDemoPM10() * 0.7; // Approximate PM1.0
    doc["pm2_5"] = sensor->getDemoPM25();
    doc["pm10"] = sensor->getDemoPM10();
    doc["voc_index"] = sensor->getVOCIndex();
    doc["health_status"] = sensor->getDemoHealthStatus();
    doc["risk_level"] = sensor->getDemoRiskLevel();
    doc["demo_mode"] = true;
    
    // Add demo particle counts
    JsonObject particles = doc.createNestedObject("particles");
    particles["0_3um"] = random(1000, 5000);
    particles["0_5um"] = random(500, 2000);
    particles["1_0um"] = random(200, 800);
    particles["2_5um"] = random(50, 200);
    particles["5_0um"] = random(10, 50);
    particles["10um"] = random(5, 20);
  }
  
  doc["led_state"] = getLEDState();
  doc["timestamp"] = millis();
  
  doc["free_memory"] = ESP.getFreeHeap();
  doc["wifi_rssi"] = WiFi.RSSI();
  
  // Add network info
  doc["ip_address"] = getIPAddress();
  
  String response;
  serializeJson(doc, response);
  
  server->sendHeader("Access-Control-Allow-Origin", "*");
  server->send(200, "application/json", response);
}

void AirQualityWebServer::handleLEDToggle() {
  toggleLED();
  DynamicJsonDocument doc(128);
  doc["led_state"] = getLEDState();
  String response;
  serializeJson(doc, response);
  server->send(200, "application/json", response);
}

void AirQualityWebServer::handleLEDOn() {
  if (!getLEDState()) toggleLED();
  DynamicJsonDocument doc(128);
  doc["led_state"] = true;
  String response;
  serializeJson(doc, response);
  server->send(200, "application/json", response);
}

void AirQualityWebServer::handleLEDOff() {
  if (getLEDState()) toggleLED();
  DynamicJsonDocument doc(128);
  doc["led_state"] = false;
  String response;
  serializeJson(doc, response);
  server->send(200, "application/json", response);
}

String AirQualityWebServer::getCommonCSS() {
  return "";
}

void AirQualityWebServer::handleClient() {
  server->handleClient();
}

String AirQualityWebServer::getIPAddress() {
  return WiFi.localIP().toString();
}

bool AirQualityWebServer::isWiFiConnected() {
  return WiFi.status() == WL_CONNECTED;
}