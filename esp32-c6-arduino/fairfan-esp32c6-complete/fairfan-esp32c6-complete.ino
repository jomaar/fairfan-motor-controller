/*
  FairFan ESP32-C6 Complete - Arduino IDE Version
  
  This version combines:
  - Working ESP32-C6 display configuration (Config 3 pins)
  - Complete application logic from PlatformIO main.cpp
  - Professional web interface for Controllino communication
  - All modular components merged into single Arduino IDE file
  
  Hardware Setup:
  ESP32-C6 Display Pins (CONFIRMED WORKING):
  - SCK:  GPIO7  (SPI Clock)
  - MOSI: GPIO6  (SPI Data)  
  - CS:   GPIO14 (Chip Select)
  - DC:   GPIO15 (Data/Command)
  - RST:  GPIO21 (Reset)
  - BL:   GPIO22 (Backlight)
  
  Controllino Communication:
  - ESP32-C6 TX (GPIO5) → Controllino Serial2 RX  
  - ESP32-C6 RX (GPIO4) → Controllino Serial2 TX
  - GND → GND
  
  IMPORTANT: Controllino must use Serial2, NOT Serial (USB)
  NOTE: GPIO21/22 are used by display, so serial uses GPIO4/5
*/

#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include <Arduino_GFX_Library.h>
#include <HardwareSerial.h>

// ============================================================================
// CONFIGURATION
// ============================================================================

// Network Configuration - UPDATE WITH YOUR CREDENTIALS
namespace Config {
  namespace WiFi {
    // Access Point Mode (for direct phone connection)
    const char* AP_SSID = "FairFan-Control";
    const char* AP_PASSWORD = "fairfan2025";  // Minimum 8 characters
    
    // Station Mode (to connect to existing WiFi) - Leave empty to use AP mode only
    const char* STA_SSID = "your_wifi_network";        // Replace with your WiFi network name
    const char* STA_PASSWORD = "your_wifi_password";   // Replace with your WiFi password
    
    const unsigned long CONNECT_TIMEOUT_MS = 10000;
  }
  
  namespace Serial {
    // Controllino Communication (using Serial1 on ESP32-C6)
    // CHANGED: Avoid conflict with display pins (RST:21, BL:22)
    const uint8_t RX_PIN = 4;    // ESP32-C6 RX → Controllino Serial2 TX  
    const uint8_t TX_PIN = 5;    // ESP32-C6 TX → Controllino Serial2 RX
    const unsigned long BAUD_RATE = 115200;
    const unsigned long TIMEOUT_MS = 1000;
  }
  
  namespace Web {
    const uint16_t HTTP_PORT = 80;
    const unsigned long UPDATE_INTERVAL_MS = 100;
  }
  
  namespace Display {
    const bool ENABLED = true;
    const uint16_t SCREEN_WIDTH = 172;   // Portrait mode
    const uint16_t SCREEN_HEIGHT = 320;  // Portrait mode
    const uint8_t ROTATION = 0;          // Portrait orientation
    const unsigned long UPDATE_INTERVAL_MS = 500;
  }
  
  namespace System {
    const unsigned long SERIAL_BUFFER_SIZE = 256;
  }
}

// ============================================================================
// ESP32-C6 DISPLAY CONFIGURATION (CONFIRMED WORKING)
// ============================================================================

#define TFT_SCK  7   // SPI Clock
#define TFT_MOSI 6   // SPI Data
#define TFT_CS   14  // Chip Select
#define TFT_DC   15  // Data/Command
#define TFT_RST  21  // Reset
#define TFT_BL   22  // Backlight

// Create display bus and driver with working pins
Arduino_DataBus *bus = new Arduino_ESP32SPI(TFT_DC, TFT_CS, TFT_SCK, TFT_MOSI, GFX_NOT_DEFINED, FSPI);
Arduino_GFX *display = new Arduino_ST7789(bus, TFT_RST, 0 /* rotation */, true /* IPS */, 
                                           Config::Display::SCREEN_WIDTH, Config::Display::SCREEN_HEIGHT, 
                                           34 /* col offset 1 */, 0 /* row offset 1 */);

// ============================================================================
// GLOBAL OBJECTS
// ============================================================================

WebServer server(Config::Web::HTTP_PORT);
HardwareSerial controllinoSerial(1);  // Use Serial1 for ESP32-C6

// ============================================================================
// SERIAL BRIDGE CLASS
// ============================================================================

class SerialBridge {
private:
  String receiveBuffer;
  String lastResponse;
  unsigned long lastResponseTime;
  
public:
  SerialBridge() : receiveBuffer(""), lastResponse(""), lastResponseTime(0) {
    receiveBuffer.reserve(Config::System::SERIAL_BUFFER_SIZE);
  }
  
  void begin() {
    controllinoSerial.begin(Config::Serial::BAUD_RATE, SERIAL_8N1, 
                           Config::Serial::RX_PIN, Config::Serial::TX_PIN);
    
    Serial.println(F("[Bridge] Serial connection to Controllino initialized"));
    Serial.printf("[Bridge] RX: GPIO%d, TX: GPIO%d\n", Config::Serial::RX_PIN, Config::Serial::TX_PIN);
    Serial.printf("[Bridge] Baud Rate: %lu\n", Config::Serial::BAUD_RATE);
    Serial.println(F("[Bridge] NOTE: Display uses GPIO21/22, Serial uses GPIO4/5"));
  }
  
  bool sendCommand(const String& command) {
    if (command.length() == 0) return false;
    
    controllinoSerial.println(command);
    Serial.printf("[Bridge] Sent: %s\n", command.c_str());
    return true;
  }
  
  void update() {
    while (controllinoSerial.available()) {
      char c = controllinoSerial.read();
      
      if (c == '\n' || c == '\r') {
        if (receiveBuffer.length() > 0) {
          processResponse(receiveBuffer);
          receiveBuffer = "";
        }
      } else {
        receiveBuffer += c;
        
        if (receiveBuffer.length() >= Config::System::SERIAL_BUFFER_SIZE) {
          Serial.println(F("[Bridge] Warning: Buffer overflow, clearing"));
          receiveBuffer = "";
        }
      }
    }
  }
  
  String getLastResponse() const { return lastResponse; }
  unsigned long getLastResponseTime() const { return lastResponseTime; }
  bool isConnected() const { return (millis() - lastResponseTime) < Config::Serial::TIMEOUT_MS; }
  
private:
  void processResponse(const String& response) {
    lastResponse = response;
    lastResponseTime = millis();
    Serial.printf("[Bridge] Received: %s\n", response.c_str());
  }
};

// ============================================================================
// WIFI MANAGER CLASS  
// ============================================================================

class WiFiManager {
private:
  bool apMode;
  String ipAddress;
  
public:
  WiFiManager() : apMode(false), ipAddress("") {}
  
  void begin() {
    WiFi.mode(WIFI_OFF);
    delay(100);
    
    // Try Station mode if credentials provided
    if (strlen(Config::WiFi::STA_SSID) > 0 && strcmp(Config::WiFi::STA_SSID, "your_wifi_network") != 0) {
      if (connectStation()) {
        return;
      }
    }
    
    // Fall back to Access Point mode
    startAccessPoint();
  }
  
  String getIPAddress() const { return ipAddress; }
  bool isAPMode() const { return apMode; }
  
  String getStatusString() const {
    if (apMode) {
      return "AP: " + String(Config::WiFi::AP_SSID);
    } else {
      return "WiFi: " + String(Config::WiFi::STA_SSID);
    }
  }
  
private:
  bool connectStation() {
    Serial.println(F("[WiFi] Connecting to WiFi..."));
    Serial.printf("[WiFi] SSID: %s\n", Config::WiFi::STA_SSID);
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(Config::WiFi::STA_SSID, Config::WiFi::STA_PASSWORD);
    
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED) {
      if (millis() - startTime > Config::WiFi::CONNECT_TIMEOUT_MS) {
        Serial.println(F("[WiFi] Connection timeout"));
        WiFi.disconnect();
        return false;
      }
      delay(500);
      Serial.print(".");
    }
    
    Serial.println();
    Serial.println(F("[WiFi] Connected!"));
    ipAddress = WiFi.localIP().toString();
    Serial.printf("[WiFi] IP Address: %s\n", ipAddress.c_str());
    
    apMode = false;
    return true;
  }
  
  void startAccessPoint() {
    Serial.println(F("[WiFi] Starting Access Point..."));
    
    WiFi.mode(WIFI_AP);
    IPAddress apIP(192, 168, 4, 1);
    IPAddress gateway(192, 168, 4, 1);
    IPAddress subnet(255, 255, 255, 0);
    WiFi.softAPConfig(apIP, gateway, subnet);
    WiFi.softAP(Config::WiFi::AP_SSID, Config::WiFi::AP_PASSWORD);
    
    ipAddress = WiFi.softAPIP().toString();
    
    Serial.println(F("[WiFi] Access Point started"));
    Serial.printf("[WiFi] SSID: %s\n", Config::WiFi::AP_SSID);
    Serial.printf("[WiFi] Password: %s\n", Config::WiFi::AP_PASSWORD);
    Serial.printf("[WiFi] IP Address: %s\n", ipAddress.c_str());
    
    apMode = true;
  }
};

// ============================================================================
// DISPLAY MANAGER CLASS
// ============================================================================

class DisplayManager {
private:
  unsigned long lastUpdate;
  String lastWiFiStatus;
  String lastIPAddress;
  bool lastConnectedStatus;
  String lastResponse;
  
  // Colors
  const uint16_t COLOR_BG = BLACK;
  const uint16_t COLOR_TEXT = WHITE;
  const uint16_t COLOR_TITLE = CYAN;
  const uint16_t COLOR_OK = GREEN;
  const uint16_t COLOR_ERROR = RED;
  const uint16_t COLOR_WARNING = YELLOW;
  const uint16_t COLOR_GRAY = 0x8410;
  
public:
  DisplayManager() : lastUpdate(0), lastWiFiStatus(""), lastIPAddress(""),
                     lastConnectedStatus(false), lastResponse("") {}
  
  bool begin() {
    if (!Config::Display::ENABLED) {
      Serial.println(F("[Display] Disabled in config"));
      return false;
    }
    
    Serial.println(F("[Display] Initializing ESP32-C6 LCD..."));
    
    // Initialize display with confirmed working configuration
    pinMode(TFT_SCK, OUTPUT);
    pinMode(TFT_MOSI, OUTPUT);
    pinMode(TFT_CS, OUTPUT);
    pinMode(TFT_DC, OUTPUT);
    pinMode(TFT_RST, OUTPUT);
    pinMode(TFT_BL, OUTPUT);
    
    // Test backlight
    digitalWrite(TFT_BL, LOW);
    delay(500);
    digitalWrite(TFT_BL, HIGH);
    delay(500);
    
    // Hardware reset
    digitalWrite(TFT_RST, LOW);
    delay(100);
    digitalWrite(TFT_RST, HIGH);
    delay(100);
    
    // Initialize display
    bool success = display->begin();
    if (!success) {
      Serial.println(F("[Display] ERROR: Display initialization failed!"));
      return false;
    }
    
    Serial.println(F("[Display] SUCCESS: Display initialized!"));
    
    // Test colors
    display->fillScreen(RED);
    delay(500);
    display->fillScreen(GREEN);
    delay(500);
    display->fillScreen(BLUE);
    delay(500);
    display->fillScreen(COLOR_BG);
    
    showStartup();
    return true;
  }
  
  void update(const String& wifiStatus, const String& ipAddress, 
              bool isConnected, const String& response) {
    if (!Config::Display::ENABLED) return;
    
    unsigned long now = millis();
    if (now - lastUpdate < Config::Display::UPDATE_INTERVAL_MS) {
      return;
    }
    lastUpdate = now;
    
    // Only redraw if something changed
    bool needsRedraw = (wifiStatus != lastWiFiStatus) ||
                      (ipAddress != lastIPAddress) ||
                      (isConnected != lastConnectedStatus) ||
                      (response != lastResponse);
    
    if (!needsRedraw) return;
    
    // Store current values
    lastWiFiStatus = wifiStatus;
    lastIPAddress = ipAddress;
    lastConnectedStatus = isConnected;
    lastResponse = response;
    
    // Redraw screen
    drawStatus();
  }
  
private:
  void showStartup() {
    display->fillScreen(COLOR_BG);
    
    // Title
    display->setTextColor(COLOR_TITLE);
    display->setTextSize(3);
    display->setCursor(20, 40);
    display->println("FairFan");
    
    display->setTextColor(COLOR_TEXT);
    display->setTextSize(1);
    display->setCursor(10, 80);
    display->println("Motor Controller");
    
    display->setTextColor(COLOR_WARNING);
    display->setCursor(10, 100);
    display->println("ESP32-C6 + Display");
    
    display->setTextColor(COLOR_OK);
    display->setCursor(10, 120);
    display->println("Pins: SCK:7 MOSI:6");
    display->setCursor(10, 140);
    display->println("CS:14 DC:15 RST:21 BL:22");
    
    display->setTextColor(COLOR_TEXT);
    display->setCursor(10, 180);
    display->println("Starting...");
    
    delay(3000);
  }
  
  void drawStatus() {
    display->fillScreen(COLOR_BG);
    
    int16_t y = 10;
    
    // Title
    display->setTextSize(2);
    display->setTextColor(COLOR_TITLE);
    display->setCursor(10, y);
    display->println("FairFan");
    y += 30;
    
    // WiFi Status
    display->setTextSize(1);
    display->setTextColor(COLOR_TEXT);
    display->setCursor(10, y);
    display->print("WiFi: ");
    display->setTextColor(COLOR_OK);
    display->println(lastWiFiStatus);
    y += 20;
    
    // IP Address
    display->setTextColor(COLOR_TEXT);
    display->setCursor(10, y);
    display->print("IP: ");
    display->setTextColor(COLOR_TITLE);
    display->println(lastIPAddress);
    y += 20;
    
    // Controllino Status
    display->setTextColor(COLOR_TEXT);
    display->setCursor(10, y);
    display->print("Controllino: ");
    if (lastConnectedStatus) {
      display->setTextColor(COLOR_OK);
      display->println("Connected");
    } else {
      display->setTextColor(COLOR_ERROR);
      display->println("Disconnected");
    }
    y += 20;
    
    // Last Response
    if (lastResponse.length() > 0) {
      display->setTextColor(COLOR_WARNING);
      display->setCursor(10, y);
      display->print("Response: ");
      display->setTextColor(COLOR_TEXT);
      String resp = lastResponse;
      if (resp.length() > 20) {
        resp = resp.substring(0, 17) + "...";
      }
      display->println(resp);
      y += 20;
    }
    
    // Uptime
    y = Config::Display::SCREEN_HEIGHT - 30;
    display->setTextColor(COLOR_GRAY);
    display->setCursor(10, y);
    unsigned long uptime = millis() / 1000;
    unsigned long minutes = uptime / 60;
    unsigned long seconds = uptime % 60;
    display->printf("Uptime: %lum %lus", minutes, seconds);
  }
};

// ============================================================================
// GLOBAL INSTANCES
// ============================================================================

SerialBridge serialBridge;
WiFiManager wifiManager;
DisplayManager displayManager;

// ============================================================================
// WEB SERVER HANDLERS
// ============================================================================

void handleRoot() {
  String html = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
    <title>FairFan Control</title>
    <style>
        * { margin: 0; padding: 0; box-sizing: border-box; }
        body {
            font-family: -apple-system, BlinkMacSystemFont, 'Segoe UI', Arial, sans-serif;
            background: linear-gradient(135deg, #667eea 0%, #764ba2 100%);
            min-height: 100vh;
            padding: 20px;
            color: #333;
        }
        .container {
            max-width: 600px;
            margin: 0 auto;
        }
        .header {
            background: white;
            border-radius: 15px;
            padding: 20px;
            margin-bottom: 20px;
            box-shadow: 0 10px 30px rgba(0,0,0,0.2);
            text-align: center;
        }
        h1 {
            color: #667eea;
            font-size: 28px;
            margin-bottom: 10px;
        }
        .status {
            display: inline-block;
            padding: 5px 15px;
            border-radius: 20px;
            font-size: 14px;
            font-weight: 500;
        }
        .status.connected { background: #10b981; color: white; }
        .status.disconnected { background: #ef4444; color: white; }
        
        .card {
            background: white;
            border-radius: 15px;
            padding: 20px;
            margin-bottom: 15px;
            box-shadow: 0 4px 15px rgba(0,0,0,0.1);
        }
        .card h2 {
            font-size: 18px;
            color: #667eea;
            margin-bottom: 15px;
            border-bottom: 2px solid #f0f0f0;
            padding-bottom: 10px;
        }
        
        .button-grid {
            display: grid;
            grid-template-columns: repeat(2, 1fr);
            gap: 10px;
        }
        .button-grid.single {
            grid-template-columns: 1fr;
        }
        
        button {
            padding: 15px;
            border: none;
            border-radius: 10px;
            font-size: 16px;
            font-weight: 600;
            cursor: pointer;
            transition: all 0.3s;
            color: white;
            touch-action: manipulation;
        }
        button:active {
            transform: scale(0.95);
        }
        
        .btn-primary { background: #667eea; }
        .btn-primary:hover { background: #5568d3; }
        
        .btn-success { background: #10b981; }
        .btn-success:hover { background: #059669; }
        
        .btn-danger { background: #ef4444; }
        .btn-danger:hover { background: #dc2626; }
        
        .btn-warning { background: #f59e0b; }
        .btn-warning:hover { background: #d97706; }
        
        .btn-secondary { background: #6b7280; }
        .btn-secondary:hover { background: #4b5563; }
        
        .degree-input {
            display: flex;
            gap: 10px;
            margin-bottom: 15px;
        }
        .degree-input input {
            flex: 1;
            padding: 12px;
            border: 2px solid #e5e7eb;
            border-radius: 8px;
            font-size: 16px;
        }
        .degree-input button {
            padding: 12px 20px;
        }
        
        .response {
            margin-top: 15px;
            padding: 12px;
            background: #f9fafb;
            border-left: 4px solid #667eea;
            border-radius: 5px;
            font-family: monospace;
            font-size: 13px;
            max-height: 200px;
            overflow-y: auto;
        }
        
        .footer {
            text-align: center;
            color: white;
            margin-top: 30px;
            font-size: 14px;
            opacity: 0.9;
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>🌀 FairFan Control</h1>
            <span class="status" id="status">Connecting...</span>
        </div>
        
        <div class="card">
            <h2>Motor 1</h2>
            <div class="degree-input">
                <input type="number" id="degInput" placeholder="Degrees (0-1080)" min="0" max="1080" value="180">
                <button class="btn-primary" onclick="setDegrees()">Set</button>
            </div>
            <div class="button-grid">
                <button class="btn-success" onclick="sendCommand('go1')">▶️ Go</button>
                <button class="btn-danger" onclick="sendCommand('stop1')">⏹️ Stop</button>
            </div>
        </div>
        
        <div class="card">
            <h2>Motor 2</h2>
            <div class="button-grid">
                <button class="btn-primary" onclick="sendCommand('home')">🏠 Home</button>
                <button class="btn-danger" onclick="sendCommand('stop2')">⏹️ Stop</button>
            </div>
        </div>
        
        <div class="card">
            <h2>Sequence</h2>
            <div class="button-grid">
                <button class="btn-success" onclick="sendCommand('seq1')">▶️ Start</button>
                <button class="btn-danger" onclick="sendCommand('stopseq')">⏹️ Stop</button>
                <button class="btn-warning" onclick="sendCommand('softstop')">⏸️ Soft Stop</button>
                <button class="btn-danger" onclick="sendCommand('stopall')">🛑 Stop All</button>
            </div>
        </div>
        
        <div class="card">
            <h2>Direction Mode</h2>
            <div class="button-grid">
                <button class="btn-secondary" onclick="sendCommand('sync')">↕️ Same</button>
                <button class="btn-secondary" onclick="sendCommand('opposite')">↔️ Opposite</button>
            </div>
        </div>
        
        <div class="card">
            <h2>Response</h2>
            <div class="response" id="response">Ready...</div>
        </div>
        
        <div class="footer">
            FairFan Motor Controller v1.0<br>
            ESP32-C6 Web Interface
        </div>
    </div>
    
    <script>
        const API_URL = window.location.origin;
        let isConnected = false;
        
        // Update status periodically
        setInterval(updateStatus, 1000);
        updateStatus();
        
        function updateStatus() {
            fetch(API_URL + '/api/status')
                .then(res => res.json())
                .then(data => {
                    isConnected = data.connected;
                    const statusEl = document.getElementById('status');
                    statusEl.textContent = isConnected ? 'Connected' : 'Disconnected';
                    statusEl.className = 'status ' + (isConnected ? 'connected' : 'disconnected');
                    
                    if (data.lastResponse) {
                        addResponse(data.lastResponse);
                    }
                })
                .catch(err => {
                    console.error('Status error:', err);
                    isConnected = false;
                });
        }
        
        function sendCommand(cmd) {
            addResponse('→ ' + cmd);
            
            fetch(API_URL + '/api/command', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify({ command: cmd })
            })
            .then(res => res.json())
            .then(data => {
                if (!data.success) {
                    addResponse('✗ Failed to send command');
                }
            })
            .catch(err => {
                console.error('Command error:', err);
                addResponse('✗ Error: ' + err.message);
            });
        }
        
        function setDegrees() {
            const deg = document.getElementById('degInput').value;
            if (deg < 0 || deg > 1080) {
                addResponse('✗ Degrees must be 0-1080');
                return;
            }
            sendCommand('deg' + deg);
        }
        
        function addResponse(text) {
            const responseEl = document.getElementById('response');
            const time = new Date().toLocaleTimeString();
            responseEl.innerHTML = `[${time}] ${text}<br>` + responseEl.innerHTML;
        }
    </script>
</body>
</html>
)rawliteral";
  
  server.send(200, "text/html", html);
}

void handleCommand() {
  if (!server.hasArg("plain")) {
    sendError("No body received");
    return;
  }
  
  String body = server.arg("plain");
  
  // Parse JSON
  JsonDocument doc;
  DeserializationError error = deserializeJson(doc, body);
  
  if (error) {
    sendError("Invalid JSON");
    return;
  }
  
  if (!doc.containsKey("command")) {
    sendError("Missing 'command' field");
    return;
  }
  
  String command = doc["command"].as<String>();
  
  // Send to Controllino
  bool success = serialBridge.sendCommand(command);
  
  // Respond
  JsonDocument response;
  response["success"] = success;
  response["command"] = command;
  
  String responseStr;
  serializeJson(response, responseStr);
  server.send(200, "application/json", responseStr);
}

void handleStatus() {
  JsonDocument doc;
  
  doc["connected"] = serialBridge.isConnected();
  doc["lastResponse"] = serialBridge.getLastResponse();
  doc["lastResponseTime"] = serialBridge.getLastResponseTime();
  doc["uptime"] = millis();
  doc["wifiStatus"] = wifiManager.getStatusString();
  doc["ipAddress"] = wifiManager.getIPAddress();
  doc["isAPMode"] = wifiManager.isAPMode();
  
  String response;
  serializeJson(doc, response);
  server.send(200, "application/json", response);
}

void sendError(const char* message) {
  JsonDocument doc;
  doc["error"] = message;
  
  String response;
  serializeJson(doc, response);
  server.send(400, "application/json", response);
}

// ============================================================================
// MAIN SETUP AND LOOP
// ============================================================================

void setup() {
  // Initialize USB serial for debugging
  Serial.begin(115200);
  delay(1000);
  
  Serial.println();
  Serial.println(F("========================================"));
  Serial.println(F("   FairFan ESP32-C6 Complete"));
  Serial.println(F("========================================"));
  Serial.println(F("ESP32-C6 + 1.47\" Display + Controllino"));
  Serial.println();
  
  // Initialize display first
  displayManager.begin();
  
  // Initialize serial bridge to Controllino
  serialBridge.begin();
  
  // Initialize WiFi
  wifiManager.begin();
  
  // Setup web server routes
  server.on("/", HTTP_GET, handleRoot);
  server.on("/api/command", HTTP_POST, handleCommand);
  server.on("/api/status", HTTP_GET, handleStatus);
  server.enableCORS(true);
  
  // Start web server
  server.begin();
  
  Serial.println();
  Serial.println(F("========================================"));
  Serial.println(F("   System Ready"));
  Serial.println(F("========================================"));
  Serial.println(wifiManager.getStatusString());
  Serial.printf("Web Interface: http://%s\n", wifiManager.getIPAddress().c_str());
  Serial.println(F("Controllino commands: go1, stop1, home, stop2,"));
  Serial.println(F("                     seq1, stopseq, softstop, stopall,"));
  Serial.println(F("                     sync, opposite, deg<number>"));
  Serial.println(F("========================================"));
  Serial.println();
}

void loop() {
  // Handle serial communication with Controllino
  serialBridge.update();
  
  // Handle web requests
  server.handleClient();
  
  // Update display
  displayManager.update(
    wifiManager.getStatusString(),
    wifiManager.getIPAddress(),
    serialBridge.isConnected(),
    serialBridge.getLastResponse()
  );
  
  // Small delay to prevent watchdog reset
  delay(1);
}