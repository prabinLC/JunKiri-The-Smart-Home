#include "air_quality_webserver.h"

AirQualityWebServer::AirQualityWebServer(PMSSensor* pmsSensor, AirQualityDisplay* airDisplay) {
  sensor = pmsSensor;
  display = airDisplay;
  server = new ESP8266WebServer(WEB_SERVER_PORT);
}

AirQualityWebServer::~AirQualityWebServer() {
  if (server) {
    delete server;
  }
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
  String html = "<!DOCTYPE html><html><head>";
  html += "<meta charset=\"UTF-8\"><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0\">";
  html += "<title>Junkiri Smart Home - Air Quality Monitor</title>";
  html += "<script src=\"https://cdn.jsdelivr.net/npm/chart.js\"></script>";
  html += "<style>";
  html += "body{background:#1a1a2e;color:white;font-family:Arial;margin:0;padding:20px}";
  html += ".container{max-width:1200px;margin:0 auto}";
  html += ".header{text-align:center;background:#16213e;padding:30px;border-radius:15px;margin-bottom:30px}";
  html += ".header h1{font-size:2.5rem;color:#4fc3f7;margin-bottom:10px}";
  html += ".header p{font-size:1.2rem;color:#cccccc}";
  html += ".nav-buttons{text-align:center;margin:30px 0}";
  html += ".nav-btn{background:#2563eb;color:white;padding:15px 30px;text-decoration:none;border-radius:10px;margin:10px;display:inline-block}";
  html += ".nav-btn:hover{background:#1d4ed8}";
  html += ".card{background:#0f3460;border-radius:15px;padding:25px;margin-bottom:20px;border:1px solid #334155}";
  html += ".card h3{color:#4fc3f7;margin-bottom:20px;font-size:1.5rem}";
  html += ".grid{display:grid;gap:20px}";
  html += ".grid-2{grid-template-columns:repeat(auto-fit,minmax(300px,1fr))}";
  html += ".grid-3{grid-template-columns:repeat(auto-fit,minmax(250px,1fr))}";
  html += ".metric{text-align:center}";
  html += ".metric-value{font-size:2.5rem;font-weight:bold;color:#4fc3f7;margin:15px 0}";
  html += ".metric-label{color:#cccccc;font-size:1.1rem}";
  html += ".status{padding:15px;border-radius:10px;margin:20px 0;text-align:center;font-weight:bold;font-size:1.2rem}";
  html += ".status-good{background:#065f46;border:2px solid #10b981}";
  html += ".status-moderate{background:#92400e;border:2px solid #f59e0b}";
  html += ".status-risky{background:#991b1b;border:2px solid #ef4444}";
  html += ".chart-container{position:relative;height:300px;margin:20px 0}";
  html += ".info-panel{background:rgba(255,255,255,0.05);padding:20px;border-radius:12px}";
  html += ".info-item{margin:12px 0;display:flex;justify-content:space-between;align-items:center}";
  html += ".info-label{color:#9ca3af;font-size:1.1rem}";
  html += ".info-value{color:#ffffff;font-weight:bold;font-size:1.1rem}";
  html += "</style></head><body>";
  
  html += "<div class=\"container\">";
  html += "<div class=\"header\">";
  html += "<h1>🌬️ Air Quality Monitor</h1>";
  html += "<p>Real-time environmental monitoring with Chart.js visualization</p>";
  html += "</div>";
  
  html += "<div class=\"nav-buttons\">";
  html += "<a href=\"/control\" class=\"nav-btn\">🏠 Control Home</a>";
  html += "<a href=\"/api\" class=\"nav-btn\">📊 API Data</a>";
  html += "<a href=\"javascript:updateCharts()\" class=\"nav-btn\">🔄 Refresh Data</a>";
  html += "</div>";
  
  if (sensor->isDataValid()) {
    String statusClass = "status-good";
    String statusIcon = "😊";
    if (sensor->getRiskLevel() == "MODERATE") {
      statusClass = "status-moderate";
      statusIcon = "😐";
    } else if (sensor->getRiskLevel() == "HIGH") {
      statusClass = "status-risky";
      statusIcon = "😟";
    }
    
    html += "<div class=\"card\">";
    html += "<div class=\"status " + statusClass + "\">";
    html += statusIcon + " " + sensor->getHealthStatus() + " - Risk Level: " + sensor->getRiskLevel();
    html += "</div>";
    html += "</div>";
  }
  
  html += "<div class=\"grid grid-3\">";
  
  if (sensor->isDataValid()) {
    html += "<div class=\"card metric\">";
    html += "<h3>🫁 PM2.5</h3>";
    html += "<div class=\"metric-value\" id=\"pm25Value\">" + String(sensor->currentData.pm2_5_atm, 1) + "</div>";
    html += "<div class=\"metric-label\">μg/m³</div>";
    html += "</div>";
    
    html += "<div class=\"card metric\">";
    html += "<h3>🌫️ PM10</h3>";
    html += "<div class=\"metric-value\" id=\"pm10Value\">" + String(sensor->currentData.pm10_atm, 1) + "</div>";
    html += "<div class=\"metric-label\">μg/m³</div>";
    html += "</div>";
    
    html += "<div class=\"card metric\">";
    html += "<h3>🧪 VOC Index</h3>";
    html += "<div class=\"metric-value\" id=\"vocValue\">" + String(sensor->getVOCIndex()) + "</div>";
    html += "<div class=\"metric-label\">Index</div>";
    html += "</div>";
  } else {
    html += "<div class=\"card metric\">";
    html += "<h3>⚠️ Sensor Status</h3>";
    html += "<div class=\"metric-value\">ERROR</div>";
    html += "<div class=\"metric-label\">Check Connections</div>";
    html += "</div>";
  }
  
  html += "</div>";
  
  html += "<div class=\"grid grid-2\">";
  
  html += "<div class=\"card\">";
  html += "<h3>📊 PM2.5 & PM10 Levels</h3>";
  html += "<div class=\"chart-container\">";
  html += "<canvas id=\"particleChart\"></canvas>";
  html += "</div>";
  html += "</div>";
  
  html += "<div class=\"card\">";
  html += "<h3>🔄 Real-time Trend</h3>";
  html += "<div class=\"chart-container\">";
  html += "<canvas id=\"trendChart\"></canvas>";
  html += "</div>";
  html += "</div>";
  
  html += "</div>";
  
  html += "<div class=\"card\">";
  html += "<h3>ℹ️ System Information</h3>";
  html += "<div class=\"info-panel\">";
  html += "<div class=\"info-item\"><span class=\"info-label\">IP Address:</span> <span class=\"info-value\">" + getIPAddress() + "</span></div>";
  html += "<div class=\"info-item\"><span class=\"info-label\">Uptime:</span> <span class=\"info-value\">" + String(millis() / 1000) + " seconds</span></div>";
  html += "<div class=\"info-item\"><span class=\"info-label\">Free RAM:</span> <span class=\"info-value\">" + String(ESP.getFreeHeap()) + " bytes</span></div>";
  html += "<div class=\"info-item\"><span class=\"info-label\">WiFi Signal:</span> <span class=\"info-value\">" + String(WiFi.RSSI()) + " dBm</span></div>";
  html += "</div>";
  html += "</div>";
  
  html += "</div>";
  
  html += "<script>";
  html += "let particleChart, trendChart;";
  html += "let trendData = {pm25: [], pm10: [], timestamps: []};";
  
  // Initialize charts
  html += "function initCharts() {";
  html += "  const ctx1 = document.getElementById('particleChart').getContext('2d');";
  html += "  particleChart = new Chart(ctx1, {";
  html += "    type: 'doughnut',";
  html += "    data: {";
  html += "      labels: ['PM2.5', 'PM10', 'Safe Zone'],";
  html += "      datasets: [{";
  html += "        data: [" + String(sensor->isDataValid() ? sensor->currentData.pm2_5_atm : 0) + ", " + String(sensor->isDataValid() ? sensor->currentData.pm10_atm : 0) + ", 50],";
  html += "        backgroundColor: ['#ef4444', '#f59e0b', '#10b981'],";
  html += "        borderColor: ['#dc2626', '#d97706', '#059669'],";
  html += "        borderWidth: 2";
  html += "      }]";
  html += "    },";
  html += "    options: {";
  html += "      responsive: true,";
  html += "      maintainAspectRatio: false,";
  html += "      plugins: {";
  html += "        legend: { labels: { color: '#ffffff' } }";
  html += "      }";
  html += "    }";
  html += "  });";
  
  html += "  const ctx2 = document.getElementById('trendChart').getContext('2d');";
  html += "  trendChart = new Chart(ctx2, {";
  html += "    type: 'line',";
  html += "    data: {";
  html += "      labels: [],";
  html += "      datasets: [{";
  html += "        label: 'PM2.5',";
  html += "        data: [],";
  html += "        borderColor: '#ef4444',";
  html += "        backgroundColor: 'rgba(239, 68, 68, 0.1)',";
  html += "        tension: 0.4";
  html += "      }, {";
  html += "        label: 'PM10',";
  html += "        data: [],";
  html += "        borderColor: '#f59e0b',";
  html += "        backgroundColor: 'rgba(245, 158, 11, 0.1)',";
  html += "        tension: 0.4";
  html += "      }]";
  html += "    },";
  html += "    options: {";
  html += "      responsive: true,";
  html += "      maintainAspectRatio: false,";
  html += "      plugins: {";
  html += "        legend: { labels: { color: '#ffffff' } }";
  html += "      },";
  html += "      scales: {";
  html += "        x: { ticks: { color: '#ffffff' } },";
  html += "        y: { ticks: { color: '#ffffff' } }";
  html += "      }";
  html += "    }";
  html += "  });";
  html += "}";
  
  // Update charts with new data
  html += "function updateCharts() {";
  html += "  fetch('/api')";
  html += "    .then(response => response.json())";
  html += "    .then(data => {";
  html += "      if (data.pm2_5 !== undefined) {";
  html += "        document.getElementById('pm25Value').textContent = data.pm2_5.toFixed(1);";
  html += "        document.getElementById('pm10Value').textContent = (data.pm10 || 0).toFixed(1);";
  html += "        document.getElementById('vocValue').textContent = data.voc_index || 0;";
  
  html += "        particleChart.data.datasets[0].data = [data.pm2_5, data.pm10 || 0, Math.max(50 - data.pm2_5, 10)];";
  html += "        particleChart.update();";
  
  html += "        const now = new Date().toLocaleTimeString();";
  html += "        trendData.timestamps.push(now);";
  html += "        trendData.pm25.push(data.pm2_5);";
  html += "        trendData.pm10.push(data.pm10 || 0);";
  
  html += "        if (trendData.timestamps.length > 10) {";
  html += "          trendData.timestamps.shift();";
  html += "          trendData.pm25.shift();";
  html += "          trendData.pm10.shift();";
  html += "        }";
  
  html += "        trendChart.data.labels = trendData.timestamps;";
  html += "        trendChart.data.datasets[0].data = trendData.pm25;";
  html += "        trendChart.data.datasets[1].data = trendData.pm10;";
  html += "        trendChart.update();";
  html += "      }";
  html += "    })";
  html += "    .catch(error => console.error('Error fetching data:', error));";
  html += "}";
  
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
  html += "<title>Home Control</title>";
  html += "<style>body{background:#1a1a2e;color:white;font-family:Arial;margin:0;padding:20px}";
  html += ".container{max-width:1000px;margin:0 auto}";
  html += ".header{text-align:center;background:#16213e;padding:30px;border-radius:15px;margin-bottom:30px}";
  html += ".nav-btn{background:#2563eb;color:white;padding:15px 30px;text-decoration:none;border-radius:10px;margin:10px}";
  html += ".led-button{background:#059669;color:white;border:none;padding:20px 40px;font-size:1.3rem;border-radius:15px;cursor:pointer}</style></head><body>";
  
  html += "<div class=\"container\">";
  html += "<div class=\"header\"><h1>Home Control Panel</h1></div>";
  
  html += "<div style=\"text-align:center;margin:30px 0\">";
  html += "<a href=\"/\" class=\"nav-btn\">Back to Monitor</a>";
  html += "</div>";
  
  html += "<div style=\"text-align:center;padding:30px\">";
  html += "<h3>LED Light Control</h3>";
  html += "<button onclick=\"controlLED()\" class=\"led-button\">Toggle LED</button>";
  html += "<div>Status: " + String(getLEDState() ? "ON" : "OFF") + "</div>";
  html += "</div>";
  
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
    
    // Add particle counts for advanced charts
    JsonObject particles = doc.createNestedObject("particles");
    particles["0_3um"] = sensor->currentData.particles_03;
    particles["0_5um"] = sensor->currentData.particles_05;
    particles["1_0um"] = sensor->currentData.particles_10;
    particles["2_5um"] = sensor->currentData.particles_25;
    particles["5_0um"] = sensor->currentData.particles_50;
    particles["10_0um"] = sensor->currentData.particles_100;
  } else {
    doc["valid"] = false;
    doc["pm2_5"] = 0;
    doc["pm10"] = 0;
    doc["voc_index"] = 0;
    doc["error"] = "No valid sensor data";
  }
  
  doc["led_state"] = getLEDState();
  doc["timestamp"] = millis();
  doc["uptime"] = millis() / 1000;
  doc["free_memory"] = ESP.getFreeHeap();
  doc["wifi_rssi"] = WiFi.RSSI();
  
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
  return "<style>body{background:#1a1a2e;color:white}</style>";
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
