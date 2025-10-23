/**
 * FairFan ESP32-C6 Web Interface (Arduino IDE Version - NO DISPLAY)
 * 
 * This firmware runs on ESP32-C6 and provides a WiFi web interface
 * for controlling the FairFan motor controller (Controllino MAXI).
 * 
 * Hardware: ESP32-C6 Development Board (NO DISPLAY - TFT_eSPI not compatible)
 * 
 * Hardware Connections:
 * - ESP32-C6 TX1 (GPIO4) → Controllino Serial RX (pin 0)
 * - ESP32-C6 RX1 (GPIO5) → Controllino Serial TX (pin 1)
 * - GND → GND
 * 
 * IMPORTANT: Controllino uses Serial (USB), NOT Serial2
 * WARNING: Disconnect ESP32 from pins 0/1 when programming Controllino
 * 
 * Features:
 * - WiFi Access Point (default) or Station mode
 * - Web-based control interface with responsive design
 * - Real-time serial bridge to Controllino
 * - RESTful API for commands and status
 * - Serial Monitor for status (no LCD display)
 * 
 * Required Libraries (install via Arduino IDE Library Manager):
 * - ArduinoJson by Benoit Blanchon (for JSON API)
 * 
 * Board Selection: ESP32C6 Dev Module
 * 
 * @author jomaar
 * @date 2025
 */

#include <WiFi.h>
#include <WebServer.h>
#include <ArduinoJson.h>

// ============================================================================
// CONFIGURATION
// ============================================================================

// WiFi Configuration
const char* AP_SSID = "FairFan-Control";
const char* AP_PASSWORD = "fairfan2025";  // Minimum 8 characters

// Station Mode (to connect to existing WiFi) - Leave empty to use AP mode only
const char* STA_SSID = "";
const char* STA_PASSWORD = "";

// Connection timeout
const unsigned long WIFI_CONNECT_TIMEOUT_MS = 10000;

// Serial Communication with Controllino (ESP32-C6 uses UART1)
const uint8_t SERIAL_RX_PIN = 5;  // GPIO5 for UART1 RX (connects to Controllino TX pin 1)
const uint8_t SERIAL_TX_PIN = 4;  // GPIO4 for UART1 TX (connects to Controllino RX pin 0)
const unsigned long SERIAL_BAUD_RATE = 115200;  // Match Controllino Serial baud rate
const unsigned long SERIAL_TIMEOUT_MS = 1000;

// Web Server
const uint16_t HTTP_PORT = 80;
const unsigned long WEB_UPDATE_INTERVAL_MS = 100;  // Status update frequency

// System
const unsigned long SERIAL_BUFFER_SIZE = 256;

// Status LED (optional - use built-in LED if available)
const uint8_t STATUS_LED_PIN = 2;  // GPIO2 is usually built-in LED on ESP32-C6

// ============================================================================
// GLOBAL OBJECTS AND VARIABLES
// ============================================================================

// Hardware objects
WebServer webServer(HTTP_PORT);
HardwareSerial controllinoSerial(1);  // ESP32-C6 UART1

// Communication variables
String serialReceiveBuffer;
String lastResponse;
unsigned long lastResponseTime = 0;
bool isControllinoConnected = false;

// WiFi variables
bool apMode = false;
String ipAddress = "";
String wifiStatusString = "";

// Status tracking
unsigned long lastStatusPrint = 0;
const unsigned long STATUS_PRINT_INTERVAL = 5000;  // Print status every 5 seconds

// ============================================================================
// STATUS LED MANAGEMENT
// ============================================================================

void initializeStatusLED() {
    pinMode(STATUS_LED_PIN, OUTPUT);
    digitalWrite(STATUS_LED_PIN, LOW);
}

void updateStatusLED() {
    static unsigned long lastBlink = 0;
    static bool ledState = false;
    
    unsigned long interval;
    
    if (!isControllinoConnected) {
        interval = 250;  // Fast blink when disconnected
    } else {
        interval = 2000; // Slow blink when connected
    }
    
    if (millis() - lastBlink > interval) {
        ledState = !ledState;
        digitalWrite(STATUS_LED_PIN, ledState);
        lastBlink = millis();
    }
}

// ============================================================================
// WIFI MANAGEMENT
// ============================================================================

bool connectToWiFiStation() {
    Serial.println(F("[WiFi] Connecting to WiFi..."));
    Serial.print(F("[WiFi] SSID: "));
    Serial.println(STA_SSID);
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(STA_SSID, STA_PASSWORD);
    
    unsigned long startTime = millis();
    while (WiFi.status() != WL_CONNECTED) {
        if (millis() - startTime > WIFI_CONNECT_TIMEOUT_MS) {
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
    Serial.print(F("[WiFi] IP Address: "));
    Serial.println(ipAddress);
    
    apMode = false;
    wifiStatusString = "WiFi: " + String(STA_SSID);
    return true;
}

void startAccessPoint() {
    Serial.println(F("[WiFi] Starting Access Point..."));
    
    WiFi.mode(WIFI_AP);
    IPAddress apIP(192, 168, 4, 1);
    IPAddress gateway(192, 168, 4, 1);
    IPAddress subnet(255, 255, 255, 0);
    WiFi.softAPConfig(apIP, gateway, subnet);
    WiFi.softAP(AP_SSID, AP_PASSWORD);
    
    ipAddress = WiFi.softAPIP().toString();
    
    Serial.println(F("[WiFi] Access Point started"));
    Serial.print(F("[WiFi] SSID: "));
    Serial.println(AP_SSID);
    Serial.print(F("[WiFi] Password: "));
    Serial.println(AP_PASSWORD);
    Serial.print(F("[WiFi] IP Address: "));
    Serial.println(ipAddress);
    
    apMode = true;
    wifiStatusString = "AP: " + String(AP_SSID);
}

void initializeWiFi() {
    WiFi.mode(WIFI_OFF);
    delay(100);
    
    // Try Station mode if credentials provided
    if (strlen(STA_SSID) > 0) {
        if (connectToWiFiStation()) {
            return;
        }
    }
    
    // Fall back to Access Point mode
    startAccessPoint();
}

// ============================================================================
// SERIAL COMMUNICATION
// ============================================================================

void initializeSerial() {
    controllinoSerial.begin(SERIAL_BAUD_RATE, SERIAL_8N1, SERIAL_RX_PIN, SERIAL_TX_PIN);
    
    Serial.println(F("[Bridge] Serial connection to Controllino initialized"));
    Serial.print(F("[Bridge] RX: GPIO"));
    Serial.print(SERIAL_RX_PIN);
    Serial.print(F(", TX: GPIO"));
    Serial.println(SERIAL_TX_PIN);
    Serial.print(F("[Bridge] Baud Rate: "));
    Serial.println(SERIAL_BAUD_RATE);
    
    serialReceiveBuffer.reserve(SERIAL_BUFFER_SIZE);
}

bool sendCommandToControllino(const String& command) {
    if (command.length() == 0) {
        return false;
    }
    
    controllinoSerial.println(command);
    Serial.print(F("[Bridge] Sent: "));
    Serial.println(command);
    
    return true;
}

void processSerialResponse(const String& response) {
    lastResponse = response;
    lastResponseTime = millis();
    isControllinoConnected = true;
    
    Serial.print(F("[Bridge] Received: "));
    Serial.println(response);
}

void updateSerial() {
    // Read all available data
    while (controllinoSerial.available()) {
        char c = controllinoSerial.read();
        
        if (c == '\n' || c == '\r') {
            if (serialReceiveBuffer.length() > 0) {
                processSerialResponse(serialReceiveBuffer);
                serialReceiveBuffer = "";
            }
        } else {
            serialReceiveBuffer += c;
            
            // Prevent buffer overflow
            if (serialReceiveBuffer.length() >= SERIAL_BUFFER_SIZE) {
                Serial.println(F("[Bridge] Warning: Buffer overflow, clearing"));
                serialReceiveBuffer = "";
            }
        }
    }
    
    // Check connection timeout
    isControllinoConnected = (millis() - lastResponseTime) < SERIAL_TIMEOUT_MS;
}

// ============================================================================
// STATUS MONITORING
// ============================================================================

void printSystemStatus() {
    if (millis() - lastStatusPrint < STATUS_PRINT_INTERVAL) {
        return;
    }
    lastStatusPrint = millis();
    
    Serial.println();
    Serial.println(F("=== FairFan ESP32-C6 Status ==="));
    Serial.print(F("WiFi: "));
    Serial.println(wifiStatusString);
    Serial.print(F("IP: "));
    Serial.println(ipAddress);
    Serial.print(F("Controllino: "));
    Serial.println(isControllinoConnected ? "CONNECTED" : "DISCONNECTED");
    if (lastResponse.length() > 0) {
        Serial.print(F("Last Response: "));
        Serial.println(lastResponse);
    }
    Serial.print(F("Uptime: "));
    Serial.print(millis() / 1000);
    Serial.println(F(" seconds"));
    Serial.println(F("=============================="));
    Serial.println();
}

// ============================================================================
// WEB SERVER
// ============================================================================

void handleRoot() {
    String html = getWebInterfaceHTML();
    webServer.send(200, "text/html", html);
}

void handleCommand() {
    if (!webServer.hasArg("plain")) {
        sendErrorResponse("No body received");
        return;
    }
    
    String body = webServer.arg("plain");
    
    // Parse JSON
    StaticJsonDocument<200> doc;
    DeserializationError error = deserializeJson(doc, body);
    
    if (error) {
        sendErrorResponse("Invalid JSON");
        return;
    }
    
    if (!doc.containsKey("command")) {
        sendErrorResponse("Missing 'command' field");
        return;
    }
    
    String command = doc["command"].as<String>();
    
    // Send to Controllino
    bool success = sendCommandToControllino(command);
    
    // Respond
    StaticJsonDocument<200> response;
    response["success"] = success;
    response["command"] = command;
    
    String responseStr;
    serializeJson(response, responseStr);
    webServer.send(200, "application/json", responseStr);
}

void handleStatus() {
    StaticJsonDocument<300> doc;
    
    doc["connected"] = isControllinoConnected;
    doc["lastResponse"] = lastResponse;
    doc["lastResponseTime"] = lastResponseTime;
    doc["uptime"] = millis();
    doc["wifiMode"] = apMode ? "AP" : "STA";
    doc["ipAddress"] = ipAddress;
    doc["freeHeap"] = ESP.getFreeHeap();
    
    String response;
    serializeJson(doc, response);
    webServer.send(200, "application/json", response);
}

void sendErrorResponse(const char* message) {
    StaticJsonDocument<100> doc;
    doc["error"] = message;
    
    String response;
    serializeJson(doc, response);
    webServer.send(400, "application/json", response);
}

void initializeWebServer() {
    // Root page - serve web interface
    webServer.on("/", HTTP_GET, handleRoot);
    
    // API endpoint - send command to Controllino
    webServer.on("/api/command", HTTP_POST, handleCommand);
    
    // API endpoint - get status
    webServer.on("/api/status", HTTP_GET, handleStatus);
    
    // Handle CORS for API requests
    webServer.enableCORS(true);
    
    // Start server
    webServer.begin();
    Serial.print(F("[Web] Server started on port "));
    Serial.println(HTTP_PORT);
}

// ============================================================================
// WEB INTERFACE HTML
// ============================================================================

String getWebInterfaceHTML() {
    return R"rawliteral(
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
        
        .device-info {
            margin-top: 10px;
            font-size: 12px;
            color: #666;
        }
        
        .notice {
            background: #fef3c7;
            border: 1px solid #f59e0b;
            border-radius: 8px;
            padding: 10px;
            margin-top: 10px;
            font-size: 12px;
            color: #92400e;
        }
    </style>
</head>
<body>
    <div class="container">
        <div class="header">
            <h1>🌀 FairFan Control</h1>
            <span class="status" id="status">Connecting...</span>
            <div class="device-info">
                ESP32-C6 (No Display - Serial Monitor Only)
            </div>
            <div class="notice">
                ⚠️ Check Arduino IDE Serial Monitor for detailed status information
            </div>
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
            <div class="response" id="response">Ready... (Check Serial Monitor for details)</div>
        </div>
        
        <div class="footer">
            FairFan Motor Controller v1.0<br>
            ESP32-C6 Web Interface (No Display Version)
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
                    const statusEl = document.getElementById('status');
                    statusEl.textContent = 'Connection Error';
                    statusEl.className = 'status disconnected';
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
}

// ============================================================================
// ARDUINO SETUP AND LOOP
// ============================================================================

void setup() {
    // Initialize USB serial for debugging
    Serial.begin(115200);
    delay(500);
    
    Serial.println();
    Serial.println(F("========================================"));
    Serial.println(F("   FairFan ESP32-C6 Web Interface"));
    Serial.println(F("   Arduino IDE Version (No Display)"));
    Serial.println(F("========================================"));
    Serial.println();
    
    // Initialize status LED
    initializeStatusLED();
    
    // Initialize serial bridge to Controllino
    initializeSerial();
    
    // Initialize WiFi
    initializeWiFi();
    
    // Initialize web server
    initializeWebServer();
    
    Serial.println();
    Serial.println(F("========================================"));
    Serial.println(F("   System Ready"));
    Serial.println(F("========================================"));
    Serial.println(wifiStatusString);
    Serial.print(F("Web Interface: http://"));
    Serial.println(ipAddress);
    Serial.println(F("Status: Check Serial Monitor for updates"));
    Serial.println(F("========================================"));
    Serial.println();
}

void loop() {
    // Handle serial communication
    updateSerial();
    
    // Handle web requests
    webServer.handleClient();
    
    // Update status LED
    updateStatusLED();
    
    // Print periodic status updates
    printSystemStatus();
    
    // Small delay to prevent watchdog reset
    delay(1);
}