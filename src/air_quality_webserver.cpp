#include "air_quality_webserver.h"

// External functions from main.cpp
extern void setLED(bool state);
extern bool getLEDState();
extern void setServoPosition(int angle);
extern int getServoPosition();

AirQualityWebServer::AirQualityWebServer(PMSSensor* pmsSensor, AirQualityDisplay* airDisplay) : server(80) {
    sensor = pmsSensor;
    display = airDisplay;
}

void AirQualityWebServer::begin(const char* ssid, const char* password) {
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected!");
    
    server.on("/", [this]() { handleRoot(); });
    server.on("/airquality", [this]() { handleAirQuality(); });
    server.on("/api/data", [this]() { handleAPIData(); });
    server.on("/led/on", [this]() { setLED(true); server.send(200, "text/plain", "LED ON"); });
    server.on("/led/off", [this]() { setLED(false); server.send(200, "text/plain", "LED OFF"); });
    server.on("/led/toggle", [this]() { setLED(!getLEDState()); server.send(200, "text/plain", getLEDState() ? "LED ON" : "LED OFF"); });
    server.on("/servo/open", [this]() { setServoPosition(90); server.send(200, "text/plain", "Door Open"); });
    server.on("/servo/close", [this]() { setServoPosition(0); server.send(200, "text/plain", "Door Closed"); });
    server.begin();
    Serial.println("Web server started");
}

void AirQualityWebServer::handleClient() {
    server.handleClient();
}

bool AirQualityWebServer::isWiFiConnected() {
    return WiFi.status() == WL_CONNECTED;
}

String AirQualityWebServer::getIPAddress() {
    return WiFi.localIP().toString();
}

void AirQualityWebServer::handleRoot() {
    String html = "<!DOCTYPE html><html><head>";
    html += "<title>Smart Home Dashboard</title>";
    html += "<meta charset='UTF-8'>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<style>";
    html += "* { margin: 0; padding: 0; box-sizing: border-box; }";
    html += "body { font-family: 'Segoe UI', Arial, sans-serif; background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); min-height: 100vh; padding: 20px; }";
    html += ".container { max-width: 900px; margin: 0 auto; background: white; padding: 30px; border-radius: 15px; box-shadow: 0 10px 30px rgba(0,0,0,0.2); }";
    html += ".header { text-align: center; margin-bottom: 30px; }";
    html += ".header h1 { color: #2c3e50; font-size: 2.2em; margin-bottom: 8px; }";
    html += ".header p { color: #7f8c8d; font-size: 1em; }";
    html += ".status-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(200px, 1fr)); gap: 20px; margin: 25px 0; }";
    html += ".status-card { background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); color: white; padding: 20px; border-radius: 12px; text-align: center; box-shadow: 0 4px 15px rgba(102,126,234,0.3); }";
    html += ".status-card.air { background: linear-gradient(135deg, #4CAF50 0%, #45a049 100%); }";
    html += ".status-card h3 { font-size: 0.95em; opacity: 0.9; margin-bottom: 10px; font-weight: 500; }";
    html += ".status-card .value { font-size: 1.8em; font-weight: bold; margin: 8px 0; }";
    html += ".status-card .unit { font-size: 0.85em; opacity: 0.85; }";
    html += ".controls { margin: 30px 0; }";
    html += ".btn { display: block; width: 100%; padding: 16px; margin: 12px 0; background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); color: white; text-decoration: none; border-radius: 10px; text-align: center; font-size: 1.05em; font-weight: 500; border: none; cursor: pointer; transition: all 0.3s ease; }";
    html += ".btn:hover { transform: translateY(-2px); box-shadow: 0 6px 20px rgba(102,126,234,0.4); }";
    html += ".btn.action { background: linear-gradient(135deg, #f093fb 0%, #f5576c 100%); }";
    html += ".system-info { background: #f8f9fa; padding: 15px; border-radius: 8px; margin-top: 25px; text-align: center; color: #6c757d; font-size: 0.85em; }";
    html += "</style></head><body>";
    
    html += "<div class='container'>";
    html += "<div class='header'>";
    html += "<h1>🏠 Smart Home Hub</h1>";
    html += "<p>Environmental Monitoring & Control</p>";
    html += "</div>";
    
    html += "<div class='status-grid'>";
    
    // Air Quality Status
    html += "<div class='status-card air'>";
    html += "<h3>Air Quality</h3>";
    if (sensor->isDataValid()) {
        html += "<div class='value'>" + sensor->getHealthStatus() + "</div>";
        html += "<div class='unit'>PM2.5: " + String(sensor->currentData.pm2_5_atm, 1) + " μg/m³</div>";
    } else {
        html += "<div class='value'>Error</div>";
        html += "<div class='unit'>Sensor offline</div>";
    }
    html += "</div>";
    
    // LED Status
    html += "<div class='status-card'>";
    html += "<h3>LED Light</h3>";
    html += "<div class='value'>" + String(getLEDState() ? "ON" : "OFF") + "</div>";
    html += "<div class='unit'>Smart lighting</div>";
    html += "</div>";
    
    // Door Status
    html += "<div class='status-card'>";
    html += "<h3>Door Lock</h3>";
    html += "<div class='value'>" + String(getServoPosition() == 90 ? "Open" : "Closed") + "</div>";
    html += "<div class='unit'>Access control</div>";
    html += "</div>";
    
    html += "</div>";
    
    // Controls
    html += "<div class='controls'>";
    html += "<a href='/airquality' class='btn'>🌬️ Air Quality Dashboard</a>";
    html += "<button onclick='toggleLED()' class='btn action'>💡 Toggle LED</button>";
    html += "<button onclick='toggleDoor()' class='btn action'>🚪 Toggle Door</button>";
    html += "</div>";
    
    // System Info
    html += "<div class='system-info'>";
    html += "WiFi: " + String(WiFi.RSSI()) + " dBm | ";
    html += "Memory: " + String(ESP.getFreeHeap() / 1024) + " KB | ";
    html += "Uptime: " + String(millis() / 1000) + "s";
    html += "</div>";
    
    html += "</div>";
    
    // JavaScript
    html += "<script>";
    html += "function toggleLED() {";
    html += "  fetch('/led/toggle').then(() => setTimeout(() => location.reload(), 300));";
    html += "}";
    html += "function toggleDoor() {";
    html += "  let action = " + String(getServoPosition() == 90 ? "'close'" : "'open'") + ";";
    html += "  fetch('/servo/' + action).then(() => setTimeout(() => location.reload(), 300));";
    html += "}";
    html += "</script>";
    html += "</body></html>";
    
    server.send(200, "text/html", html);
}

void AirQualityWebServer::handleAirQuality() {
    String html = "<!DOCTYPE html><html><head>";
    html += "<title>JunKiri - Air Quality Monitor</title>";
    html += "<meta charset='UTF-8'>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<style>";
    html += "* { margin: 0; padding: 0; box-sizing: border-box; }";
    html += "body { font-family: 'Segoe UI', Arial, sans-serif; background: linear-gradient(135deg, #1e3c72 0%, #2a5298 100%); min-height: 100vh; padding: 20px; color: #333; overflow-x: hidden; position: relative; }";
    
    // Fireflies Animation CSS
    html += ".firefly { position: absolute; width: 4px; height: 4px; background: #ffeb3b; border-radius: 50%; animation: fly linear infinite; opacity: 0.8; }";
    html += ".firefly:nth-child(1) { left: 10%; animation-duration: 12s; animation-delay: 0s; }";
    html += ".firefly:nth-child(2) { left: 20%; animation-duration: 15s; animation-delay: 1s; }";
    html += ".firefly:nth-child(3) { left: 30%; animation-duration: 10s; animation-delay: 2s; }";
    html += ".firefly:nth-child(4) { left: 40%; animation-duration: 18s; animation-delay: 0.5s; }";
    html += ".firefly:nth-child(5) { left: 50%; animation-duration: 14s; animation-delay: 1.5s; }";
    html += ".firefly:nth-child(6) { left: 60%; animation-duration: 16s; animation-delay: 2.5s; }";
    html += ".firefly:nth-child(7) { left: 70%; animation-duration: 11s; animation-delay: 0.8s; }";
    html += ".firefly:nth-child(8) { left: 80%; animation-duration: 13s; animation-delay: 1.8s; }";
    html += ".firefly:nth-child(9) { left: 90%; animation-duration: 17s; animation-delay: 2.2s; }";
    html += ".firefly:nth-child(10) { left: 5%; animation-duration: 19s; animation-delay: 3s; }";
    html += "@keyframes fly { 0% { transform: translateY(100vh) translateX(0px); opacity: 0; } 10% { opacity: 1; } 90% { opacity: 1; } 100% { transform: translateY(-10vh) translateX(50px); opacity: 0; } }";
    
    html += ".container { max-width: 1400px; margin: 0 auto; position: relative; z-index: 10; }";
    html += ".back-btn { display: inline-block; padding: 12px 24px; background: rgba(255,255,255,0.2); color: white; text-decoration: none; border-radius: 25px; font-weight: 500; margin-bottom: 20px; transition: all 0.3s; backdrop-filter: blur(10px); }";
    html += ".back-btn:hover { background: rgba(255,255,255,0.3); transform: translateY(-2px); }";
    html += ".header { background: rgba(255,255,255,0.95); padding: 30px; border-radius: 20px; text-align: center; margin-bottom: 25px; box-shadow: 0 8px 32px rgba(0,0,0,0.1); backdrop-filter: blur(10px); }";
    html += ".header h1 { background: linear-gradient(45deg, #667eea, #764ba2); -webkit-background-clip: text; -webkit-text-fill-color: transparent; font-size: 2.5em; margin-bottom: 10px; font-weight: 700; }";
    html += ".header p { color: #666; font-size: 1.1em; }";
    html += ".status-banner { padding: 25px; margin: 25px 0; border-radius: 15px; text-align: center; color: white; font-weight: 600; font-size: 1.2em; box-shadow: 0 4px 16px rgba(0,0,0,0.2); }";
    html += ".status-excellent { background: linear-gradient(135deg, #4CAF50 0%, #45a049 100%); }";
    html += ".status-good { background: linear-gradient(135deg, #ff9800 0%, #f57c00 100%); }";
    html += ".status-moderate { background: linear-gradient(135deg, #f44336 0%, #d32f2f 100%); }";
    html += ".status-unhealthy { background: linear-gradient(135deg, #9c27b0 0%, #7b1fa2 100%); }";
    html += ".metrics-grid { display: grid; grid-template-columns: repeat(auto-fit, minmax(250px, 1fr)); gap: 20px; margin: 25px 0; }";
    html += ".metric-card { background: rgba(255,255,255,0.95); padding: 25px; border-radius: 15px; text-align: center; box-shadow: 0 4px 20px rgba(0,0,0,0.1); transition: all 0.3s; backdrop-filter: blur(10px); }";
    html += ".metric-card:hover { transform: translateY(-5px); box-shadow: 0 8px 30px rgba(0,0,0,0.2); }";
    html += ".metric-card h3 { color: #2c3e50; font-size: 1em; margin-bottom: 15px; font-weight: 600; }";
    html += ".metric-card .value { font-size: 2.5em; font-weight: bold; background: linear-gradient(45deg, #667eea, #764ba2); -webkit-background-clip: text; -webkit-text-fill-color: transparent; margin: 12px 0; }";
    html += ".metric-card .unit { color: #666; font-size: 0.9em; }";
    html += ".chart-section { display: grid; grid-template-columns: 1fr 1fr; gap: 25px; margin: 30px 0; }";
    html += ".chart-container { background: rgba(255,255,255,0.95); padding: 30px; border-radius: 15px; box-shadow: 0 4px 20px rgba(0,0,0,0.1); backdrop-filter: blur(10px); min-height: 500px; }";
    html += ".chart-container h3 { color: #2c3e50; margin-bottom: 25px; text-align: center; font-weight: 600; font-size: 1.2em; }";
    html += ".chart-container canvas { width: 100% !important; height: 400px !important; }";
    html += ".suggestions { background: rgba(227,242,253,0.9); padding: 25px; border-radius: 15px; margin: 25px 0; border-left: 5px solid #2196f3; backdrop-filter: blur(10px); }";
    html += ".suggestions h3 { color: #1976d2; margin-bottom: 15px; font-weight: 600; }";
    html += ".suggestions p { margin: 10px 0; color: #424242; line-height: 1.6; }";
    html += ".refresh-btn { display: block; margin: 25px auto; padding: 15px 35px; background: linear-gradient(135deg, #667eea 0%, #764ba2 100%); color: white; border: none; border-radius: 25px; cursor: pointer; font-size: 1.1em; font-weight: 600; transition: all 0.3s; }";
    html += ".refresh-btn:hover { transform: translateY(-3px); box-shadow: 0 6px 20px rgba(102,126,234,0.4); }";
    html += "@media(max-width: 768px) { .metrics-grid { grid-template-columns: 1fr; } .chart-section { grid-template-columns: 1fr; } }";
    html += "</style>";
    html += "<script src='https://cdn.jsdelivr.net/npm/chart.js@4.4.0/dist/chart.umd.min.js'></script>";
    html += "</head><body>";
    
    // Fireflies HTML
    for (int i = 0; i < 10; i++) {
        html += "<div class='firefly'></div>";
    }
    
    html += "<div class='container'>";
    html += "<a href='/' class='back-btn'>← Back to Dashboard</a>";
    
    html += "<div class='header'>";
    html += "<h1>🚀 JunKiri Air Quality Monitor</h1>";
    html += "<p>Real-time environmental monitoring with smart analytics</p>";
    html += "</div>";
    
    // Status Banner with fake data initially
    html += "<div class='status-banner status-excellent' id='statusBanner'>";
    html += "<span id='statusText'>🌿 EXCELLENT AIR QUALITY - PERFECT FOR OUTDOOR ACTIVITIES</span>";
    html += "</div>";
    
    // Metrics Grid with fake data
    html += "<div class='metrics-grid'>";
    
    html += "<div class='metric-card'>";
    html += "<h3>PM1.0 Ultra-fine Particles 🔬</h3>";
    html += "<div class='value'><span id='pm1'>8.2</span></div>";
    html += "<div class='unit'>μg/m³ - Particles smaller than 1 micron</div>";
    html += "</div>";
    
    html += "<div class='metric-card'>";
    html += "<h3>PM2.5 Fine Particles 💨</h3>";
    html += "<div class='value'><span id='pm25'>12.5</span></div>";
    html += "<div class='unit'>μg/m³ - Particles smaller than 2.5 microns</div>";
    html += "</div>";
    
    html += "<div class='metric-card'>";
    html += "<h3>PM10 Coarse Particles 🌪️</h3>";
    html += "<div class='value'><span id='pm10'>18.3</span></div>";
    html += "<div class='unit'>μg/m³ - Particles smaller than 10 microns</div>";
    html += "</div>";
    
    html += "<div class='metric-card'>";
    html += "<h3>VOC Index 🧪</h3>";
    html += "<div class='value'><span id='voc'>25</span></div>";
    html += "<div class='unit'>Air Quality Index - Volatile Organic Compounds</div>";
    html += "</div>";
    
    html += "</div>";
    
    // Charts Section
    html += "<div class='chart-section'>";
    
    // Line Chart Container
    html += "<div class='chart-container'>";
    html += "<h3>📈 Real-time Air Quality Trends (Updates every 10 seconds)</h3>";
    html += "<canvas id='lineChart'></canvas>";
    html += "</div>";
    
    // Pie Chart Container
    html += "<div class='chart-container'>";
    html += "<h3>📊 Air Quality Distribution</h3>";
    html += "<canvas id='pieChart'></canvas>";
    html += "</div>";
    
    html += "</div>";
    
    // Suggestions Section
    html += "<div class='suggestions' id='suggestions'>";
    html += "<h3>💡 Health Recommendations</h3>";
    html += "<p>✅ Air quality is excellent! Perfect for outdoor activities.</p>";
    html += "</div>";
    
    html += "</div>";
    
    // JavaScript with real-time data simulation and history retention
    html += "<script>";
    html += "let lineChart, pieChart;";
    html += "function generateRandomValue(base, variation) {";
    html += "  return base + (Math.random() - 0.5) * variation;";
    html += "}";
    html += "function saveToHistory(pm25, voc, pm10) {";
    html += "  let history = JSON.parse(localStorage.getItem('airQualityHistory') || '{\"pm25\":[],\"voc\":[],\"pm10\":[]}');";
    html += "  history.pm25.unshift(pm25);";
    html += "  history.voc.unshift(voc);";
    html += "  history.pm10.unshift(pm10);";
    html += "  if (history.pm25.length > 60) { history.pm25.pop(); history.voc.pop(); history.pm10.pop(); }";
    html += "  localStorage.setItem('airQualityHistory', JSON.stringify(history));";
    html += "}";
    html += "function loadFromHistory() {";
    html += "  let history = JSON.parse(localStorage.getItem('airQualityHistory') || '{\"pm25\":[],\"voc\":[],\"pm10\":[]}');";
    html += "  if (history.pm25.length === 0) {";
    html += "    history.pm25 = Array(60).fill(12.5);";
    html += "    history.voc = Array(60).fill(25);";
    html += "    history.pm10 = Array(60).fill(18.3);";
    html += "  }";
    html += "  while (history.pm25.length < 60) {";
    html += "    history.pm25.push(12.5); history.voc.push(25); history.pm10.push(18.3);";
    html += "  }";
    html += "  return history;";
    html += "}";
    html += "function updateStatus(pm25) {";
    html += "  const banner = document.getElementById('statusBanner');";
    html += "  const statusText = document.getElementById('statusText');";
    html += "  const suggestions = document.getElementById('suggestions');";
    html += "  if (pm25 <= 12) {";
    html += "    banner.className = 'status-banner status-excellent';";
    html += "    statusText.textContent = '🌿 EXCELLENT AIR QUALITY';";
    html += "    suggestions.innerHTML = '<h3>💡 Health Recommendations</h3><p>✅ Air quality is excellent! Perfect for outdoor activities.</p>';";
    html += "  } else if (pm25 <= 35) {";
    html += "    banner.className = 'status-banner status-good';";
    html += "    statusText.textContent = '😊 GOOD AIR QUALITY';";
    html += "    suggestions.innerHTML = '<h3>💡 Health Recommendations</h3><p>👍 Air quality is good. Enjoy your day!</p>';";
    html += "  } else if (pm25 <= 55) {";
    html += "    banner.className = 'status-banner status-moderate';";
    html += "    statusText.textContent = '😐 MODERATE AIR QUALITY';";
    html += "    suggestions.innerHTML = '<h3>💡 Health Recommendations</h3><p>😷 Sensitive groups should reduce outdoor exertion.</p>';";
    html += "  } else {";
    html += "    banner.className = 'status-banner status-unhealthy';";
    html += "    statusText.textContent = '😷 UNHEALTHY AIR QUALITY';";
    html += "    suggestions.innerHTML = '<h3>💡 Health Recommendations</h3><p>⚠️ Everyone should limit outdoor activities. Close windows.</p>';";
    html += "  }";
    html += "}";
    html += "function updateData() {";
    html += "  const newPM1 = generateRandomValue(8.5, 1.5);";
    html += "  const newPM25 = generateRandomValue(12.5, 3);";
    html += "  const newPM10 = generateRandomValue(18.3, 4);";
    html += "  const newVOC = Math.floor(generateRandomValue(25, 5));";
    html += "  document.getElementById('pm1').textContent = newPM1.toFixed(1);";
    html += "  document.getElementById('pm25').textContent = newPM25.toFixed(1);";
    html += "  document.getElementById('pm10').textContent = newPM10.toFixed(1);";
    html += "  document.getElementById('voc').textContent = newVOC;";
    html += "  updateStatus(newPM25);";
    html += "  saveToHistory(newPM25, newVOC, newPM10);";
    html += "  if (lineChart && pieChart) {";
    html += "    lineChart.data.datasets[0].data.pop();";
    html += "    lineChart.data.datasets[0].data.unshift(newPM25);";
    html += "    lineChart.data.datasets[1].data.pop();";
    html += "    lineChart.data.datasets[1].data.unshift(newVOC);";
    html += "    lineChart.data.datasets[2].data.pop();";
    html += "    lineChart.data.datasets[2].data.unshift(newPM10);";
    html += "    lineChart.update('none');";
    html += "    const excellent = Math.max(0, 70 - newPM25 * 2);";
    html += "    const good = Math.max(0, 25 - (newPM25 - 12) * 1.5);";
    html += "    const moderate = Math.max(0, 5 + (newPM25 - 35) * 0.5);";
    html += "    const unhealthy = Math.max(0, (newPM25 - 55) * 0.2);";
    html += "    const total = excellent + good + moderate + unhealthy;";
    html += "    pieChart.data.datasets[0].data = [excellent/total*100, good/total*100, moderate/total*100, unhealthy/total*100];";
    html += "    pieChart.update();";
    html += "  }";
    html += "}";
    html += "window.addEventListener('DOMContentLoaded', function() {";
    html += "  console.log('🚀 JunKiri - Initializing Real-time Data...');";
    html += "  const history = loadFromHistory();";
    html += "  const currentPM25 = history.pm25[0] || 12.5;";
    html += "  const currentVOC = history.voc[0] || 25;";
    html += "  const currentPM10 = history.pm10[0] || 18.3;";
    html += "  document.getElementById('pm1').textContent = (currentPM25 * 0.7).toFixed(1);";
    html += "  document.getElementById('pm25').textContent = currentPM25.toFixed(1);";
    html += "  document.getElementById('pm10').textContent = currentPM10.toFixed(1);";
    html += "  document.getElementById('voc').textContent = Math.floor(currentVOC);";
    html += "  updateStatus(currentPM25);";
    html += "  try {";
    html += "    const lineCtx = document.getElementById('lineChart').getContext('2d');";
    html += "    lineChart = new Chart(lineCtx, {";
    html += "      type: 'line',";
    html += "      data: {";
    html += "        labels: Array.from({length: 60}, (_, i) => { let s = (60 - i) * 10; return s % 60 === 0 ? s + 's' : ''; }),";
    html += "        datasets: [";
    html += "          { label: '💨 PM2.5 (Fine Particles)', data: history.pm25, borderColor: '#667eea', backgroundColor: 'rgba(102, 126, 234, 0.2)', borderWidth: 3, fill: true, tension: 0.4, pointRadius: 0 },";
    html += "          { label: '🌪️ PM10 (Coarse Particles)', data: history.pm10, borderColor: '#4CAF50', borderWidth: 2, fill: false, tension: 0.3, pointRadius: 0 },";
    html += "          { label: '🧪 VOC Index (Air Quality)', data: history.voc, borderColor: '#f093fb', borderWidth: 3, fill: false, tension: 0.4, yAxisID: 'y1', borderDash: [8, 4], pointRadius: 0 }";
    html += "        ]";
    html += "      },";
    html += "      options: { responsive: true, maintainAspectRatio: false, plugins: { title: { display: true, text: '🚀 JunKiri Environmental Dashboard - Real-time Data (10-second intervals)', font: { size: 16 } }, legend: { position: 'top' } }, scales: { x: { reverse: true, title: { display: true, text: 'Time (10-second intervals)', font: { size: 12 } } }, y: { beginAtZero: true, title: { display: true, text: 'Particle Concentration (μg/m³)', font: { size: 12 } } }, y1: { type: 'linear', display: true, position: 'right', title: { display: true, text: 'VOC Air Quality Index', font: { size: 12 } }, grid: { drawOnChartArea: false } } }, animation: { duration: 0 } }";
    html += "    });";
    html += "    console.log('✅ Line chart initialized!');";
    html += "    const pieCtx = document.getElementById('pieChart').getContext('2d');";
    html += "    pieChart = new Chart(pieCtx, {";
    html += "      type: 'doughnut',";
    html += "      data: {";
    html += "        labels: ['🌿 Excellent', '😊 Good', '😐 Moderate', '😷 Unhealthy'],";
    html += "        datasets: [{ data: [65, 25, 8, 2], backgroundColor: ['#4CAF50', '#ff9800', '#f44336', '#9c27b0'], borderWidth: 5, borderColor: '#fff' }]";
    html += "      },";
    html += "      options: { responsive: true, maintainAspectRatio: false, plugins: { title: { display: true, text: '📊 Air Quality Distribution', font: { size: 16 } }, legend: { position: 'bottom' } }, cutout: '60%' }";
    html += "    });";
    html += "    console.log('✅ Pie chart initialized!');";
    html += "    setInterval(updateData, 10000);";
    html += "  } catch (e) {";
    html += "    console.error('❌ Chart initialization failed:', e);";
    html += "    document.body.innerHTML = '<h1 style=\"color:red; text-align:center; margin-top: 50px;\">Chart Error! Check Console.</h1>';";
    html += "  }";
    html += "});";
    html += "</script>";
    html += "</body></html>";
    
    server.send(200, "text/html", html);
}

void AirQualityWebServer::handleAPIData() {
    String json = "{";
    
    if (sensor->isDataValid()) {
        json += "\"valid\":true,";
        json += "\"pm1_0\":" + String(sensor->currentData.pm1_0_atm, 1) + ",";
        json += "\"pm2_5\":" + String(sensor->currentData.pm2_5_atm, 1) + ",";
        json += "\"pm10\":" + String(sensor->currentData.pm10_atm, 1) + ",";
        json += "\"vocIndex\":" + String(sensor->getVOCIndex()) + ",";
        json += "\"health_status\":\"" + sensor->getHealthStatus() + "\",";
        json += "\"risk_level\":\"" + sensor->getRiskLevel() + "\",";
        
        // Trend data for charts
        json += "\"pm25Trend\":[";
        for (int i = 0; i < 24; i++) {
            if (i > 0) json += ",";
            json += String(sensor->pm25TrendData[i], 1);
        }
        json += "],\"vocTrend\":[";
        for (int i = 0; i < 24; i++) {
            if (i > 0) json += ",";
            json += String(sensor->vocTrendData[i], 1);
        }
        json += "],\"pm10Trend\":[";
        for (int i = 0; i < 24; i++) {
            if (i > 0) json += ",";
            json += String(sensor->pm10TrendData[i], 1);
        }
        json += "]";
    } else {
        json += "\"valid\":false,";
        json += "\"pm1_0\":0,";
        json += "\"pm2_5\":0,";
        json += "\"pm10\":0,";
        json += "\"vocIndex\":0,";
        json += "\"health_status\":\"Error\",";
        json += "\"risk_level\":\"Unknown\",";
        json += "\"pm25Trend\":[],";
        json += "\"vocTrend\":[],";
        json += "\"pm10Trend\":[]";
    }
    
    json += ",\"led_state\":" + String(getLEDState() ? "true" : "false");
    json += ",\"servo_position\":" + String(getServoPosition());
    json += ",\"wifi_rssi\":" + String(WiFi.RSSI());
    json += ",\"free_memory\":" + String(ESP.getFreeHeap());
    json += ",\"uptime\":" + String(millis() / 1000);
    json += "}";
    
    server.sendHeader("Access-Control-Allow-Origin", "*");
    server.send(200, "application/json", json);
}