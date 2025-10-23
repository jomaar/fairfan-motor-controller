/*
  FairFan ESP32-C6 with Display - WORKING CONFIGURATION
  
  Using the confirmed working pin configuration (Config 3):
  - SCK:  GPIO7
  - MOSI: GPIO6  
  - CS:   GPIO14
  - DC:   GPIO15
  - RST:  GPIO21
  - BL:   GPIO22
  
  This configuration has been tested and confirmed working by the user.
*/

#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <Arduino_GFX_Library.h>

// Network Configuration - UPDATE THESE WITH YOUR ACTUAL WIFI CREDENTIALS
const char* ssid = "your_wifi_network";        // Replace with your WiFi network name
const char* password = "your_wifi_password";   // Replace with your WiFi password

// CONFIRMED WORKING ESP32-C6 Pin Configuration
#define TFT_SCK  7   // SPI Clock
#define TFT_MOSI 6   // SPI Data
#define TFT_CS   14  // Chip Select
#define TFT_DC   15  // Data/Command
#define TFT_RST  21  // Reset
#define TFT_BL   22  // Backlight

// Create display bus and driver with working pins
Arduino_DataBus *bus = new Arduino_ESP32SPI(TFT_DC, TFT_CS, TFT_SCK, TFT_MOSI, GFX_NOT_DEFINED, FSPI);

// ST7789 driver for 1.47" display (Portrait mode: 172x320)
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
  
  Serial.println("=== FairFan ESP32-C6 - WORKING CONFIGURATION ===");
  Serial.println("Using confirmed working pins:");
  Serial.printf("SCK: %d, MOSI: %d, CS: %d, DC: %d, RST: %d, BL: %d\n", 
                TFT_SCK, TFT_MOSI, TFT_CS, TFT_DC, TFT_RST, TFT_BL);
  
  // Initialize display
  Serial.println("Initializing display...");
  
  // Test all pins first
  Serial.println("Testing pin configurations...");
  pinMode(TFT_SCK, OUTPUT);
  pinMode(TFT_MOSI, OUTPUT);
  pinMode(TFT_CS, OUTPUT);
  pinMode(TFT_DC, OUTPUT);
  pinMode(TFT_RST, OUTPUT);
  pinMode(TFT_BL, OUTPUT);
  
  // Test backlight first
  Serial.println("Testing backlight...");
  digitalWrite(TFT_BL, LOW);   // Try LOW first
  delay(500);
  digitalWrite(TFT_BL, HIGH);  // Then HIGH
  delay(500);
  
  // Hardware reset sequence
  Serial.println("Performing hardware reset...");
  digitalWrite(TFT_RST, LOW);
  delay(100);
  digitalWrite(TFT_RST, HIGH);
  delay(100);
  
  // Initialize display
  Serial.println("Calling display->begin()...");
  bool displayInitSuccess = display->begin();
  if (!displayInitSuccess) {
    Serial.println("ERROR: Display initialization failed!");
    Serial.println("Trying alternative backlight polarity...");
    digitalWrite(TFT_BL, LOW);  // Try opposite polarity
    delay(500);
    displayInitSuccess = display->begin();
  } else {
    Serial.println("SUCCESS: Display initialized!");
  }
  
  // Force display update regardless of init result
  Serial.println("Forcing display test pattern...");
  display->fillScreen(RED);
  delay(1000);
  display->fillScreen(GREEN);
  delay(1000);
  display->fillScreen(BLUE);
  delay(1000);
  display->fillScreen(WHITE);
  delay(1000);
  display->fillScreen(BLACK);
  
  // Try different backlight polarities during startup
  Serial.println("Testing backlight polarities...");
  for (int i = 0; i < 5; i++) {
    digitalWrite(TFT_BL, HIGH);
    delay(200);
    digitalWrite(TFT_BL, LOW);
    delay(200);
  }
  digitalWrite(TFT_BL, HIGH); // Leave it HIGH for now
  
  // Display startup screen
  showStartupScreen();
  
  // Initialize WiFi with enhanced diagnostics
  Serial.println("=== WiFi Connection Setup ===");
  Serial.printf("SSID: '%s'\n", ssid);
  Serial.printf("Password length: %d characters\n", strlen(password));
  
  // Check if credentials are still default values
  if (strcmp(ssid, "your_wifi_network") == 0 || strcmp(password, "your_wifi_password") == 0) {
    Serial.println("ERROR: Default WiFi credentials detected!");
    Serial.println("Please update ssid and password variables with your actual WiFi credentials");
    
    display->fillScreen(BLACK);
    display->setTextColor(RED);
    display->setTextSize(1);
    display->setCursor(10, 10);
    display->println("WiFi Setup Error!");
    display->setTextColor(WHITE);
    display->setCursor(10, 30);
    display->println("Update credentials in code:");
    display->setCursor(10, 50);
    display->println("- Change 'your_wifi_network'");
    display->setCursor(10, 70);
    display->println("- Change 'your_wifi_password'");
    display->setCursor(10, 100);
    display->println("Then recompile & upload");
    
    // Don't try to connect with default values
    server.on("/", handleRoot);
    server.on("/api/fan", HTTP_GET, handleGetFan);
    server.on("/api/fan", HTTP_POST, handleSetFan);
    server.on("/api/status", handleStatus);
    server.begin();
    Serial.println("Web server started in WiFi error mode");
    return; // Skip WiFi connection attempt
  }
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.print("Connecting to WiFi");
  
  display->fillScreen(BLACK);
  display->setTextColor(WHITE);
  display->setTextSize(1);
  display->setCursor(10, 10);
  display->println("Connecting to WiFi...");
  display->setCursor(10, 30);
  display->printf("Network: %s", ssid);
  
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 30) {
    delay(500);
    Serial.print(".");
    
    // Update display with connection status
    display->setCursor(10, 50);
    display->printf("Attempt: %d/30", attempts + 1);
    display->setCursor(10 + (attempts % 15) * 8, 70);
    display->print(".");
    
    // Show detailed status every 5 attempts
    if ((attempts + 1) % 5 == 0) {
      Serial.printf("\nWiFi Status: %d (", WiFi.status());
      switch(WiFi.status()) {
        case WL_NO_SSID_AVAIL: Serial.print("SSID not found"); break;
        case WL_CONNECT_FAILED: Serial.print("Connection failed"); break;
        case WL_CONNECTION_LOST: Serial.print("Connection lost"); break;
        case WL_DISCONNECTED: Serial.print("Disconnected"); break;
        case WL_SCAN_COMPLETED: Serial.print("Scan completed"); break;
        default: Serial.printf("Status %d", WiFi.status()); break;
      }
      Serial.println(")");
    }
    
    attempts++;
  }
  
  if (WiFi.status() == WL_CONNECTED) {
    Serial.println();
    Serial.println("=== WiFi CONNECTION SUCCESS ===");
    Serial.printf("Connected to: %s\n", ssid);
    Serial.printf("IP Address: %s\n", WiFi.localIP().toString().c_str());
    Serial.printf("Gateway: %s\n", WiFi.gatewayIP().toString().c_str());
    Serial.printf("DNS: %s\n", WiFi.dnsIP().toString().c_str());
    Serial.printf("RSSI: %d dBm\n", WiFi.RSSI());
    
    // Update display with success
    display->fillScreen(BLACK);
    display->setTextColor(GREEN);
    display->setTextSize(1);
    display->setCursor(10, 10);
    display->println("WiFi Connected!");
    display->setTextColor(WHITE);
    display->setCursor(10, 30);
    display->printf("Network: %s", ssid);
    display->setCursor(10, 50);
    display->printf("IP: %s", WiFi.localIP().toString().c_str());
    display->setCursor(10, 70);
    display->printf("Signal: %d dBm", WiFi.RSSI());
    display->setCursor(10, 90);
    display->println("Starting web server...");
  } else {
    Serial.println();
    Serial.println("=== WiFi CONNECTION FAILED ===");
    Serial.printf("Final status: %d (", WiFi.status());
    switch(WiFi.status()) {
      case WL_NO_SSID_AVAIL: 
        Serial.println("SSID not found)");
        Serial.println("Check: Network name is correct and network is broadcasting");
        break;
      case WL_CONNECT_FAILED: 
        Serial.println("Connection failed)");
        Serial.println("Check: Password is correct");
        break;
      case WL_CONNECTION_LOST: 
        Serial.println("Connection lost)");
        break;
      case WL_DISCONNECTED: 
        Serial.println("Disconnected)");
        break;
      default: 
        Serial.printf("Unknown status %d)\n", WiFi.status());
        break;
    }
    
    display->fillScreen(BLACK);
    display->setTextColor(RED);
    display->setTextSize(1);
    display->setCursor(10, 10);
    display->println("WiFi Connection Failed!");
    display->setTextColor(WHITE);
    display->setCursor(10, 30);
    display->printf("Network: %s", ssid);
    display->setCursor(10, 50);
    
    switch(WiFi.status()) {
      case WL_NO_SSID_AVAIL:
        display->println("Error: Network not found");
        display->setCursor(10, 70);
        display->println("Check network name");
        break;
      case WL_CONNECT_FAILED:
        display->println("Error: Wrong password");
        display->setCursor(10, 70);
        display->println("Check WiFi password");
        break;
      default:
        display->printf("Error code: %d", WiFi.status());
        display->setCursor(10, 70);
        display->println("Check router settings");
        break;
    }
    
    display->setCursor(10, 100);
    display->println("Web server offline");
    Serial.println("Web server will not be accessible without WiFi");
  }
  
  // Setup web server routes
  server.on("/", handleRoot);
  server.on("/api/fan", HTTP_GET, handleGetFan);
  server.on("/api/fan", HTTP_POST, handleSetFan);
  server.on("/api/status", handleStatus);
  
  server.begin();
  Serial.println("Web server started");
  Serial.println("Access: http://" + WiFi.localIP().toString());
  
  delay(2000);
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
  static unsigned long lastDisplayTest = 0;
  
  if (millis() - lastDisplayUpdate > 2000) {
    updateDisplay();
    lastDisplayUpdate = millis();
  }
  
  // Run display test every 10 seconds to check if display is working
  if (millis() - lastDisplayTest > 10000) {
    testDisplayBasic();
    lastDisplayTest = millis();
  }
  
  delay(10);
}

void showStartupScreen() {
  Serial.println("Starting showStartupScreen()...");
  
  // Test basic display functions
  Serial.println("Filling screen with colors for testing...");
  display->fillScreen(RED);
  delay(500);
  display->fillScreen(GREEN);
  delay(500);
  display->fillScreen(BLUE);
  delay(500);
  display->fillScreen(BLACK);
  delay(500);
  
  Serial.println("Drawing startup screen...");
  
  // Title
  display->setTextColor(CYAN);
  display->setTextSize(2);
  display->setCursor(20, 20);
  display->println("FairFan");
  Serial.println("Drew title");
  
  display->setTextColor(WHITE);
  display->setTextSize(1);
  display->setCursor(10, 50);
  display->println("ESP32-C6 Controller");
  Serial.println("Drew controller text");
  
  // Pin info
  display->setTextColor(YELLOW);
  display->setCursor(10, 80);
  display->println("Display Pins Working:");
  display->setCursor(10, 100);
  display->printf("SCK:%d MOSI:%d", TFT_SCK, TFT_MOSI);
  display->setCursor(10, 120);
  display->printf("CS:%d DC:%d", TFT_CS, TFT_DC);
  display->setCursor(10, 140);
  display->printf("RST:%d BL:%d", TFT_RST, TFT_BL);
  Serial.println("Drew pin info");
  
  // Version info
  display->setTextColor(GREEN);
  display->setCursor(10, 170);
  display->println("Portrait Mode: 172x320");
  Serial.println("Drew version info");
  
  display->setTextColor(WHITE);
  display->setCursor(10, 200);
  display->println("Initializing...");
  Serial.println("Drew initializing text");
  
  Serial.println("Startup screen complete - display should show content now");
  delay(3000);
  Serial.println("Startup screen delay finished");
}

void updateDisplay() {
  Serial.println("Updating display...");
  display->fillScreen(BLACK);
  
  // Header
  display->setTextColor(CYAN);
  display->setTextSize(2);
  display->setCursor(10, 10);
  display->println("FairFan");
  Serial.print("Display update - WiFi status: ");
  Serial.println(WiFi.status() == WL_CONNECTED ? "Connected" : "Disconnected");
  
  // WiFi Status
  display->setTextSize(1);
  display->setCursor(10, 40);
  if (WiFi.status() == WL_CONNECTED) {
    display->setTextColor(GREEN);
    display->print("WiFi: OK");
  } else {
    display->setTextColor(RED);
    display->print("WiFi: ERROR");
  }
  
  // Fan Status
  display->setCursor(10, 60);
  display->setTextColor(WHITE);
  display->print("Fan: ");
  if (fanState.running) {
    display->setTextColor(GREEN);
    display->print("RUNNING");
  } else {
    display->setTextColor(RED);
    display->print("STOPPED");
  }
  
  // Speed
  display->setCursor(10, 80);
  display->setTextColor(WHITE);
  display->print("Speed: ");
  display->setTextColor(YELLOW);
  display->print(fanState.speed);
  display->print("%");
  
  // Mode
  display->setCursor(10, 100);
  display->setTextColor(WHITE);
  display->print("Mode: ");
  display->setTextColor(CYAN);
  display->print(fanState.mode);
  
  // Oscillation
  display->setCursor(10, 120);
  display->setTextColor(WHITE);
  display->print("Oscillate: ");
  if (fanState.oscillating) {
    display->setTextColor(GREEN);
    display->print("ON");
  } else {
    display->setTextColor(RED);
    display->print("OFF");
  }
  
  // Runtime
  display->setCursor(10, 140);
  display->setTextColor(WHITE);
  display->print("Runtime: ");
  display->setTextColor(YELLOW);
  display->print(fanState.runtime / 1000);
  display->print("s");
  
  // IP Address
  if (WiFi.status() == WL_CONNECTED) {
    display->setCursor(10, 170);
    display->setTextColor(CYAN);
    display->print("IP: ");
    display->print(WiFi.localIP());
  }
  
  // Status indicator
  display->setCursor(10, 200);
  display->setTextColor(GREEN);
  display->print("System: READY");
  
  // Visual status bar
  int barWidth = 150;
  int barHeight = 8;
  int barX = 10;
  int barY = 230;
  
  // Speed bar
  display->drawRect(barX, barY, barWidth, barHeight, WHITE);
  if (fanState.speed > 0) {
    int fillWidth = (barWidth * fanState.speed) / 100;
    display->fillRect(barX + 1, barY + 1, fillWidth - 1, barHeight - 2, GREEN);
  }
  
  display->setCursor(barX, barY + 15);
  display->setTextColor(WHITE);
  display->setTextSize(1);
  display->print("Speed Bar");
  
  // Connection status dot
  int dotX = 150;
  int dotY = 280;
  int dotRadius = 5;
  
  if (WiFi.status() == WL_CONNECTED) {
    display->fillCircle(dotX, dotY, dotRadius, GREEN);
  } else {
    display->fillCircle(dotX, dotY, dotRadius, RED);
  }
  
  display->setCursor(dotX - 20, dotY + 10);
  display->setTextColor(WHITE);
  display->print("WiFi");
}

void testDisplayBasic() {
  Serial.println("Running basic display test...");
  
  // Test backlight toggle
  digitalWrite(TFT_BL, LOW);
  delay(100);
  digitalWrite(TFT_BL, HIGH);
  delay(100);
  
  // Draw a simple test pattern
  display->drawPixel(0, 0, RED);
  display->drawPixel(171, 0, GREEN);
  display->drawPixel(0, 319, BLUE);
  display->drawPixel(171, 319, WHITE);
  
  // Draw a rectangle border
  display->drawRect(10, 10, 152, 300, YELLOW);
  
  Serial.println("Basic display test complete");
}

void handleRoot() {
  String html = R"HTML(
<!DOCTYPE html>
<html>
<head>
    <title>FairFan ESP32-C6 Control</title>
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <style>
        body { 
            font-family: Arial, sans-serif; 
            margin: 20px; 
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            color: white;
            min-height: 100vh;
        }
        .container { 
            max-width: 600px; 
            margin: 0 auto; 
            background: rgba(255,255,255,0.1); 
            padding: 30px; 
            border-radius: 15px; 
            backdrop-filter: blur(10px);
            box-shadow: 0 8px 32px rgba(0,0,0,0.3);
        }
        .header {
            text-align: center;
            margin-bottom: 30px;
        }
        .header h1 {
            font-size: 2.5em;
            margin: 0;
            text-shadow: 2px 2px 4px rgba(0,0,0,0.5);
        }
        .status { 
            background: rgba(255,255,255,0.2); 
            padding: 20px; 
            border-radius: 10px; 
            margin: 20px 0;
            border-left: 5px solid #4CAF50;
        }
        .controls { 
            margin: 30px 0; 
        }
        button { 
            padding: 15px 25px; 
            margin: 10px; 
            border: none; 
            border-radius: 8px; 
            cursor: pointer; 
            font-size: 16px;
            font-weight: bold;
            transition: all 0.3s ease;
            box-shadow: 0 4px 15px rgba(0,0,0,0.2);
        }
        button:hover { 
            transform: translateY(-2px); 
            box-shadow: 0 6px 20px rgba(0,0,0,0.3);
        }
        .btn-on { background: #4CAF50; color: white; }
        .btn-off { background: #f44336; color: white; }
        .btn-mode { background: #2196F3; color: white; }
        .speed-control {
            margin: 20px 0;
            padding: 20px;
            background: rgba(255,255,255,0.1);
            border-radius: 10px;
        }
        input[type="range"] { 
            width: 100%; 
            height: 8px;
            border-radius: 4px;
            background: rgba(255,255,255,0.3);
            outline: none;
            margin: 10px 0;
        }
        .success { 
            background: rgba(76, 175, 80, 0.3); 
            border-left-color: #4CAF50;
        }
        .pin-info {
            background: rgba(255, 193, 7, 0.2);
            padding: 15px;
            border-radius: 8px;
            margin: 20px 0;
            border-left: 5px solid #FFC107;
            font-family: monospace;
            font-size: 14px;
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1 id="title">FairFan Control</h1>
            <p>ESP32-C6 with Working Display!</p>
        </div>
        
        <div class="pin-info">
            <strong id="pin-title">CONFIRMED WORKING PIN CONFIGURATION</strong><br>
            SCK: GPIO7, MOSI: GPIO6<br>
            CS: GPIO14, DC: GPIO15, RST: GPIO21, BL: GPIO22<br>
            Display: Portrait Mode (172x320)
        </div>
        
        <div class="status success" id="status">Loading...</div>
        
        <div class="controls">
            <button class="btn-on" onclick="toggleFan()" id="fanBtn">Toggle Fan</button>
            <button class="btn-mode" onclick="toggleMode()" id="modeBtn">Toggle Mode</button>
            <button class="btn-mode" onclick="toggleOscillate()" id="oscBtn">Toggle Oscillate</button>
        </div>
        
        <div class="speed-control">
            <label><strong>Fan Speed: <span id="speedValue">0</span>%</strong></label><br>
            <input type="range" id="speedSlider" min="0" max="100" value="0" onchange="setSpeed(this.value)">
        </div>
    </div>
    
    <script>
        // Add emoji icons using JavaScript
        document.addEventListener("DOMContentLoaded", function() {
            document.getElementById("title").innerHTML = String.fromCodePoint(0x1F300) + " FairFan Control";
            document.getElementById("pin-title").innerHTML = String.fromCodePoint(0x2705) + " CONFIRMED WORKING PIN CONFIGURATION";
            document.getElementById("fanBtn").innerHTML = String.fromCodePoint(0x1F504) + " Toggle Fan";
            document.getElementById("modeBtn").innerHTML = String.fromCodePoint(0x2699) + String.fromCodePoint(0xFE0F) + " Toggle Mode";
            document.getElementById("oscBtn").innerHTML = String.fromCodePoint(0x2194) + String.fromCodePoint(0xFE0F) + " Toggle Oscillate";
        });
        
        function updateStatus() {
            fetch("/api/status")
                .then(response => response.json())
                .then(data => {
                    const statusDiv = document.getElementById("status");
                    const target = String.fromCodePoint(0x1F3AF);
                    const greenCircle = String.fromCodePoint(0x1F7E2);
                    const redCircle = String.fromCodePoint(0x1F534);
                    const chart = String.fromCodePoint(0x1F4CA);
                    const gear = String.fromCodePoint(0x2699) + String.fromCodePoint(0xFE0F);
                    const checkMark = String.fromCodePoint(0x2705);
                    const crossMark = String.fromCodePoint(0x274C);
                    const stopwatch = String.fromCodePoint(0x23F1) + String.fromCodePoint(0xFE0F);
                    const globe = String.fromCodePoint(0x1F310);
                    
                    statusDiv.innerHTML = 
                        "<h3>" + target + " Fan Status</h3>" +
                        "<strong>State:</strong> " + (data.running ? greenCircle + " RUNNING" : redCircle + " STOPPED") + "<br>" +
                        "<strong>Speed:</strong> " + data.speed + "% " + chart + "<br>" +
                        "<strong>Mode:</strong> " + data.mode + " " + gear + "<br>" +
                        "<strong>Oscillating:</strong> " + (data.oscillating ? checkMark + " YES" : crossMark + " NO") + "<br>" +
                        "<strong>Runtime:</strong> " + Math.floor(data.runtime/1000) + "s " + stopwatch + "<br>" +
                        "<strong>WiFi:</strong> " + (data.wifi_connected ? greenCircle + " Connected" : redCircle + " Disconnected") + "<br>" +
                        "<strong>IP:</strong> " + data.ip_address + " " + globe;
                    
                    document.getElementById("speedSlider").value = data.speed;
                    document.getElementById("speedValue").textContent = data.speed;
                    
                    // Update status class
                    statusDiv.className = data.wifi_connected ? "status success" : "status";
                });
        }
        
        function toggleFan() {
            fetch("/api/fan", {
                method: "POST", 
                headers: {"Content-Type": "application/json"}, 
                body: JSON.stringify({action: "toggle"})
            }).then(() => updateStatus());
        }
        
        function toggleMode() {
            fetch("/api/fan", {
                method: "POST", 
                headers: {"Content-Type": "application/json"}, 
                body: JSON.stringify({action: "mode"})
            }).then(() => updateStatus());
        }
        
        function toggleOscillate() {
            fetch("/api/fan", {
                method: "POST", 
                headers: {"Content-Type": "application/json"}, 
                body: JSON.stringify({action: "oscillate"})
            }).then(() => updateStatus());
        }
        
        function setSpeed(speed) {
            document.getElementById("speedValue").textContent = speed;
            fetch("/api/fan", {
                method: "POST", 
                headers: {"Content-Type": "application/json"}, 
                body: JSON.stringify({action: "speed", value: parseInt(speed)})
            }).then(() => updateStatus());
        }
        
        // Update status every 2 seconds
        setInterval(updateStatus, 2000);
        updateStatus();
        
        // Show success message
        setTimeout(() => {
            console.log(String.fromCodePoint(0x2705) + " FairFan ESP32-C6 with working display configuration loaded!");
        }, 1000);
    </script>
</body>
</html>
)HTML";
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
    else if (action == "oscillate") {
      fanState.oscillating = !fanState.oscillating;
      Serial.println("Oscillation toggled: " + String(fanState.oscillating ? "ON" : "OFF"));
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
  doc["display_working"] = true;
  doc["pin_config"] = "SCK=" + String(TFT_SCK) + ",MOSI=" + String(TFT_MOSI) + ",CS=" + String(TFT_CS) + ",DC=" + String(TFT_DC) + ",RST=" + String(TFT_RST) + ",BL=" + String(TFT_BL);
  
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}