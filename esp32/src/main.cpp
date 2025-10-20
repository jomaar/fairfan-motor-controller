/**
 * FairFan ESP32 Web Interface
 * 
 * This firmware runs on ESP32 and provides a WiFi web interface
 * for controlling the FairFan motor controller (Controllino MAXI).
 * 
 * Hardware Connections:
 * - ESP32 TX2 (GPIO17) → Controllino Serial2 RX
 * - ESP32 RX2 (GPIO16) → Controllino Serial2 TX
 * - GND → GND
 * 
 * IMPORTANT: Controllino must use Serial2, NOT Serial (USB)
 * 
 * Features:
 * - WiFi Access Point (default) or Station mode
 * - Web-based control interface
 * - Real-time serial bridge to Controllino
 * - Optional OLED display support
 * 
 * @author jomaar
 * @date 2025
 */

#include <Arduino.h>
#include "Config.h"
#include "WiFiManager.h"
#include "SerialBridge.h"
#include "WebServerManager.h"
#include "DisplayManager.h"

// Global objects
WiFiManager wifiManager;
SerialBridge serialBridge;
WebServerManager* webServer = nullptr;
DisplayManager displayManager;

/**
 * Setup - Initialize all components
 */
void setup() {
    // Initialize USB serial for debugging
    Serial.begin(115200);
    delay(500);
    
    Serial.println();
    Serial.println(F("========================================"));
    Serial.println(F("   FairFan ESP32 Web Interface"));
    Serial.println(F("========================================"));
    Serial.println();
    
    // Initialize serial bridge to Controllino
    serialBridge.begin();
    
    // Initialize WiFi
    wifiManager.begin();
    
    // Initialize web server
    webServer = new WebServerManager(serialBridge);
    webServer->begin();
    
    // Initialize display (ESP32-C6-LCD-1.47)
    displayManager.begin();
    
    Serial.println();
    Serial.println(F("========================================"));
    Serial.println(F("   System Ready"));
    Serial.println(F("========================================"));
    Serial.println(wifiManager.getStatusString());
    Serial.print(F("Web Interface: http://"));
    Serial.println(wifiManager.getIPAddress());
    Serial.println(F("========================================"));
    Serial.println();
}

/**
 * Main Loop
 */
void loop() {
    // Handle serial communication
    serialBridge.update();
    
    // Handle web requests
    if (webServer != nullptr) {
        webServer->update();
    }
    
    // Update display (ESP32-C6-LCD-1.47)
    displayManager.update(
        wifiManager.getStatusString(),
        wifiManager.getIPAddress(),
        serialBridge.isConnected(),
        serialBridge.getLastResponse()
    );
    
    // Small delay to prevent watchdog reset
    delay(1);
}
