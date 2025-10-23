/*
  FairFan ESP32-C6 with Display - CORRECTED PIN CONFIGURATION
  
  This version uses the CORRECT ESP32-C6 default SPI pins based on the official datasheet.
  The problem was that Arduino_GFX library doesn't have ESP32-C6 specific pin definitions,
  so it was using wrong default pins.
  
  ESP32-C6 Hardware SPI Pins:
  - SCK:  GPIO19 (SPI_CLK)
  - MOSI: GPIO18 (SPI_MOSI)
  - MISO: GPIO20 (SPI_MISO) - not used for display
  
  Display Control Pins (can be any digital GPIO):
  - CS:   GPIO8  (Chip Select)
  - DC:   GPIO9  (Data/Command)
  - RST:  GPIO10 (Reset)
  - BL:   GPIO11 (Backlight)
*/

#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <Arduino_GFX_Library.h>

// Network Configuration
const char* ssid = "your_wifi_network";
const char* password = "your_wifi_password";

// ESP32-C6 Corrected Pin Configuration
#define TFT_SCK  19  // ESP32-C6 Hardware SPI Clock
#define TFT_MOSI 18  // ESP32-C6 Hardware SPI Data
#define TFT_CS   8   // Chip Select
#define TFT_DC   9   // Data/Command
#define TFT_RST  10  // Reset
#define TFT_BL   11  // Backlight

// Create display bus with correct ESP32-C6 pins
Arduino_DataBus *bus = new Arduino_ESP32SPI(TFT_DC, TFT_CS, TFT_SCK, TFT_MOSI, GFX_NOT_DEFINED, FSPI);

// Create display with ST7789 driver for 1.47" display
Arduino_GFX *display = new Arduino_ST7789(bus, TFT_RST, 0 /* rotation */, true /* IPS */, 172 /* width */, 320 /* height */, 34 /* col offset 1 */, 0 /* row offset 1 */);

// Web server
WebServer server(80);

// Fan state
struct FanState {
  bool running = false;
  int speed = 0;
  bool oscillating = false;
  String mode = "manual";
  unsigned long runtime = 0;
  unsigned long lastUpdate = 0;
} fanState;

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("=== FairFan ESP32-C6 with Corrected Display Pins ===");
  Serial.println("Using ESP32-C6 Hardware SPI: SCK=19, MOSI=18");
  
  // Initialize display with correct pins
  Serial.print("Initializing display on pins: ");
  Serial.print("SCK="); Serial.print(TFT_SCK);
  Serial.print(", MOSI="); Serial.print(TFT_MOSI);
  Serial.print(", CS="); Serial.print(TFT_CS);
  Serial.print(", DC="); Serial.print(TFT_DC);
  Serial.print(", RST="); Serial.print(TFT_RST);
  Serial.print(", BL="); Serial.println(TFT_BL);
  
  // Configure backlight pin
  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, HIGH); // Turn on backlight (try HIGH first)
  
  // Initialize display
  if (!display->begin()) {
    Serial.println("ERROR: Display initialization failed!");
    Serial.println("Check your wiring and pin connections.");
  } else {
    Serial.println("SUCCESS: Display initialized!");
  }
  
  // Test display with basic graphics
  Serial.println("Testing display...");
  display->fillScreen(BLACK);
  delay(500);
  
  // Test colors
  display->fillScreen(RED);
  delay(500);
  display->fillScreen(GREEN);
  delay(500);
  display->fillScreen(BLUE);
  delay(500);
  display->fillScreen(BLACK);
  
  // Test text
  display->setTextColor(WHITE);
  display->setTextSize(2);
  display->setCursor(10, 10);
  display->println("ESP32-C6");
  display->setCursor(10, 40);
  display->println("Display");
  display->setCursor(10, 70);
  display->println("WORKING!");
  
  Serial.println("Display test complete. If you see colors and text, it's working!");
  
  // Initialize WiFi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.print("WiFi connected! IP: ");
    Serial.println(WiFi.localIP());
    
    // Update display with IP
    display->fillScreen(BLACK);
    display->setTextColor(WHITE);
    display->setTextSize(1);
    display->setCursor(10, 10);
    display->println("WiFi Connected");
    display->setCursor(10, 30);
    display->print("IP: ");
    display->println(WiFi.localIP());
  } else {
    Serial.println();
    Serial.println("WiFi connection failed");
    
    display->fillScreen(BLACK);
    display->setTextColor(RED);
    display->setTextSize(1);
    display->setCursor(10, 10);
    display->println("WiFi Failed");
  }
  
  // Setup web server routes
  server.on("/", handleRoot);
  server.on("/api/fan", HTTP_GET, handleGetFan);
  server.on("/api/fan", HTTP_POST, handleSetFan);
  server.on("/api/status", handleStatus);
  
  server.begin();
  Serial.println("Web server started");
  Serial.println("Test URL: http://" + WiFi.localIP().toString());
}

void loop() {
  server.handleClient();
  
  // Update fan runtime
  if (fanState.running) {
    unsigned long now = millis();
    if (fanState.lastUpdate > 0) {
      fanState.runtime += (now - fanState.lastUpdate);
    }
    fanState.lastUpdate = now;
  } else {
    fanState.lastUpdate = 0;
  }
  
  // Update display every 2 seconds
  static unsigned long lastDisplayUpdate = 0;
  if (millis() - lastDisplayUpdate > 2000) {
    updateDisplay();
    lastDisplayUpdate = millis();
  }
  
  delay(10);
}

void updateDisplay() {
  display->fillScreen(BLACK);
  display->setTextColor(WHITE);
  display->setTextSize(1);
  
  // Status header
  display->setCursor(10, 10);
  display->println("=== FairFan Status ===");
  
  // WiFi status
  display->setCursor(10, 30);
  if (WiFi.status() == WL_CONNECTED) {
    display->setTextColor(GREEN);
    display->print("WiFi: Connected");
  } else {
    display->setTextColor(RED);
    display->print("WiFi: Disconnected");
  }
  
  // Fan status
  display->setCursor(10, 50);
  display->setTextColor(WHITE);
  display->print("Fan: ");
  if (fanState.running) {
    display->setTextColor(GREEN);
    display->print("ON");
  } else {
    display->setTextColor(RED);
    display->print("OFF");
  }
  
  // Speed
  display->setCursor(10, 70);
  display->setTextColor(WHITE);
  display->print("Speed: ");
  display->print(fanState.speed);
  display->print("%");
  
  // Mode
  display->setCursor(10, 90);
  display->print("Mode: ");
  display->print(fanState.mode);
  
  // Runtime
  display->setCursor(10, 110);
  display->print("Runtime: ");
  display->print(fanState.runtime / 1000);
  display->print("s");
  
  // IP Address
  display->setCursor(10, 130);
  display->setTextColor(CYAN);
  display->print("IP: ");
  display->print(WiFi.localIP());
  
  // Pin info for debugging
  display->setCursor(10, 150);
  display->setTextColor(YELLOW);
  display->print("SCK:");
  display->print(TFT_SCK);
  display->print(" MOSI:");
  display->print(TFT_MOSI);
}

void handleRoot() {
  String html = R"(
<!DOCTYPE html>
<html>
<head>
    <title>FairFan Control</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { font-family: Arial; margin: 20px; background: #f0f0f0; }
        .container { max-width: 600px; margin: 0 auto; background: white; padding: 20px; border-radius: 10px; }
        .status { background: #e8f5e8; padding: 10px; border-radius: 5px; margin: 10px 0; }
        .controls { margin: 20px 0; }
        button { padding: 10px 20px; margin: 5px; border: none; border-radius: 5px; cursor: pointer; }
        .btn-on { background: #4CAF50; color: white; }
        .btn-off { background: #f44336; color: white; }
        .btn-mode { background: #2196F3; color: white; }
        input[type="range"] { width: 100%; }
        .debug { background: #fff3cd; padding: 10px; border-radius: 5px; margin: 10px 0; font-family: monospace; }
    </style>
</head>
<body>
    <div class="container">
        <h1>🌀 FairFan Control Panel</h1>
        <div class="debug">
            <strong>ESP32-C6 Display Test</strong><br>
            Hardware SPI Pins: SCK=19, MOSI=18<br>
            Control Pins: CS=8, DC=9, RST=10, BL=11
        </div>
        <div class="status" id="status">Loading...</div>
        <div class="controls">
            <button class="btn-on" onclick="toggleFan()">Toggle Fan</button>
            <button class="btn-mode" onclick="toggleMode()">Toggle Mode</button><br><br>
            <label>Speed: <span id="speedValue">0</span>%</label><br>
            <input type="range" id="speedSlider" min="0" max="100" value="0" onchange="setSpeed(this.value)">
        </div>
    </div>
    
    <script>
        function updateStatus() {
            fetch('/api/status')
                .then(response => response.json())
                .then(data => {
                    document.getElementById('status').innerHTML = 
                        `<strong>Status:</strong> ${data.running ? 'ON' : 'OFF'}<br>` +
                        `<strong>Speed:</strong> ${data.speed}%<br>` +
                        `<strong>Mode:</strong> ${data.mode}<br>` +
                        `<strong>Oscillating:</strong> ${data.oscillating ? 'Yes' : 'No'}<br>` +
                        `<strong>Runtime:</strong> ${Math.floor(data.runtime/1000)}s`;
                    document.getElementById('speedSlider').value = data.speed;
                    document.getElementById('speedValue').textContent = data.speed;
                });
        }
        
        function toggleFan() {
            fetch('/api/fan', {method: 'POST', headers: {'Content-Type': 'application/json'}, body: JSON.stringify({action: 'toggle'})})
                .then(() => updateStatus());
        }
        
        function toggleMode() {
            fetch('/api/fan', {method: 'POST', headers: {'Content-Type': 'application/json'}, body: JSON.stringify({action: 'mode'})})
                .then(() => updateStatus());
        }
        
        function setSpeed(speed) {
            document.getElementById('speedValue').textContent = speed;
            fetch('/api/fan', {method: 'POST', headers: {'Content-Type': 'application/json'}, body: JSON.stringify({action: 'speed', value: parseInt(speed)})})
                .then(() => updateStatus());
        }
        
        // Update status every 2 seconds
        setInterval(updateStatus, 2000);
        updateStatus();
    </script>
</body>
</html>
)";
  server.send(200, "text/html", html);
}

void handleGetFan() {
  JsonDocument doc;
  doc["running"] = fanState.running;
  doc["speed"] = fanState.speed;
  doc["oscillating"] = fanState.oscillating;
  doc["mode"] = fanState.mode;
  doc["runtime"] = fanState.runtime;
  
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void handleSetFan() {
  if (server.hasArg("plain")) {
    JsonDocument doc;
    deserializeJson(doc, server.arg("plain"));
    
    String action = doc["action"];
    
    if (action == "toggle") {
      fanState.running = !fanState.running;
      Serial.println("Fan toggled: " + String(fanState.running ? "ON" : "OFF"));
    }
    else if (action == "speed") {
      fanState.speed = doc["value"];
      Serial.println("Speed set to: " + String(fanState.speed) + "%");
    }
    else if (action == "mode") {
      fanState.mode = (fanState.mode == "manual") ? "auto" : "manual";
      Serial.println("Mode changed to: " + fanState.mode);
    }
    
    server.send(200, "application/json", "{\"status\":\"ok\"}");
  } else {
    server.send(400, "application/json", "{\"error\":\"Invalid request\"}");
  }
}

void handleStatus() {
  JsonDocument doc;
  doc["running"] = fanState.running;
  doc["speed"] = fanState.speed;
  doc["oscillating"] = fanState.oscillating;
  doc["mode"] = fanState.mode;
  doc["runtime"] = fanState.runtime;
  doc["wifi_connected"] = (WiFi.status() == WL_CONNECTED);
  doc["ip_address"] = WiFi.localIP().toString();
  doc["pins"] = "SCK=" + String(TFT_SCK) + ",MOSI=" + String(TFT_MOSI) + ",CS=" + String(TFT_CS) + ",DC=" + String(TFT_DC);
  
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}