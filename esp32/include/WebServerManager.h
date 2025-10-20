#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#include <Arduino.h>
#include <WebServer.h>
#include <ArduinoJson.h>
#include "Config.h"
#include "SerialBridge.h"

/**
 * WebServerManager - Handles HTTP requests and serves the web interface
 */
class WebServerManager {
private:
    WebServer server;
    SerialBridge& bridge;
    
public:
    WebServerManager(SerialBridge& serialBridge) 
        : server(Config::Web::HTTP_PORT), bridge(serialBridge) {}
    
    /**
     * Initialize web server and routes
     */
    void begin() {
        // Root page - serve web interface
        server.on("/", HTTP_GET, [this]() {
            handleRoot();
        });
        
        // API endpoint - send command to Controllino
        server.on("/api/command", HTTP_POST, [this]() {
            handleCommand();
        });
        
        // API endpoint - get status
        server.on("/api/status", HTTP_GET, [this]() {
            handleStatus();
        });
        
        // Handle CORS for API requests
        server.enableCORS(true);
        
        // Start server
        server.begin();
        Serial.print(F("[Web] Server started on port "));
        Serial.println(Config::Web::HTTP_PORT);
    }
    
    /**
     * Update - call from main loop
     */
    void update() {
        server.handleClient();
    }
    
private:
    /**
     * Serve the main web interface
     */
    void handleRoot() {
        String html = getWebInterface();
        server.send(200, "text/html", html);
    }
    
    /**
     * Handle command POST request
     * Expected JSON: {"command": "go1"}
     */
    void handleCommand() {
        if (!server.hasArg("plain")) {
            sendError("No body received");
            return;
        }
        
        String body = server.arg("plain");
        
        // Parse JSON
        StaticJsonDocument<200> doc;
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
        bool success = bridge.sendCommand(command);
        
        // Respond
        StaticJsonDocument<200> response;
        response["success"] = success;
        response["command"] = command;
        
        String responseStr;
        serializeJson(response, responseStr);
        server.send(200, "application/json", responseStr);
    }
    
    /**
     * Handle status GET request
     */
    void handleStatus() {
        StaticJsonDocument<300> doc;
        
        doc["connected"] = bridge.isConnected();
        doc["lastResponse"] = bridge.getLastResponse();
        doc["lastResponseTime"] = bridge.getLastResponseTime();
        doc["uptime"] = millis();
        
        String response;
        serializeJson(doc, response);
        server.send(200, "application/json", response);
    }
    
    /**
     * Send error response
     */
    void sendError(const char* message) {
        StaticJsonDocument<100> doc;
        doc["error"] = message;
        
        String response;
        serializeJson(doc, response);
        server.send(400, "application/json", response);
    }
    
    /**
     * Generate web interface HTML
     */
    String getWebInterface() {
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
            ESP32 Web Interface
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
    }
};

#endif // WEB_SERVER_H
