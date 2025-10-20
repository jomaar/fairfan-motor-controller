/**
 * FairFan ESP32 Web Interface
 * 
 * This firmware runs on ESP32 and provides a WiFi web interface
 * for controlling the FairFan motor controller (Controllino MAXI).
 * 
 * Hardware Connections:
 * - ESP32 TX2 (GPIO17) → Controllino RX
 * - ESP32 RX2 (GPIO16) → Controllino TX
 * - GND → GND
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

// Optional display support
#ifdef CONFIG_DISPLAY_ENABLED
  #include <Wire.h>
  #include <Adafruit_GFX.h>
  #include <Adafruit_SSD1306.h>
#endif

// Global objects
WiFiManager wifiManager;
SerialBridge serialBridge;
WebServerManager* webServer = nullptr;

#ifdef CONFIG_DISPLAY_ENABLED
  Adafruit_SSD1306* display = nullptr;
  unsigned long lastDisplayUpdate = 0;
#endif

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
    
    #ifdef CONFIG_DISPLAY_ENABLED
    if (Config::Display::ENABLED) {
        // Initialize display
        Wire.begin(Config::Display::I2C_SDA, Config::Display::I2C_SCL);
        display = new Adafruit_SSD1306(
            Config::Display::SCREEN_WIDTH,
            Config::Display::SCREEN_HEIGHT,
            &Wire,
            -1
        );
        
        if (display->begin(SSD1306_SWITCHCAPVCC, Config::Display::I2C_ADDRESS)) {
            Serial.println(F("[Display] OLED initialized"));
            display->clearDisplay();
            display->setTextSize(1);
            display->setTextColor(SSD1306_WHITE);
            display->setCursor(0, 0);
            display->println(F("FairFan Control"));
            display->println();
            display->println(wifiManager.getStatusString());
            display->println(wifiManager.getIPAddress());
            display->display();
        } else {
            Serial.println(F("[Display] OLED not found"));
            delete display;
            display = nullptr;
        }
    }
    #endif
    
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
    
    #ifdef CONFIG_DISPLAY_ENABLED
    // Update display periodically
    if (display != nullptr && Config::Display::ENABLED) {
        if (millis() - lastDisplayUpdate > Config::Display::UPDATE_INTERVAL_MS) {
            lastDisplayUpdate = millis();
            
            display->clearDisplay();
            display->setCursor(0, 0);
            
            // Title
            display->setTextSize(1);
            display->println(F("FairFan Control"));
            display->drawLine(0, 10, 127, 10, SSD1306_WHITE);
            
            // WiFi status
            display->setCursor(0, 15);
            display->print(wifiManager.isAPMode() ? "AP: " : "WiFi: ");
            display->println(wifiManager.isAPMode() ? 
                Config::WiFi::AP_SSID : Config::WiFi::STA_SSID);
            
            // IP address
            display->print(F("IP: "));
            display->println(wifiManager.getIPAddress());
            
            // Connection status
            display->drawLine(0, 35, 127, 35, SSD1306_WHITE);
            display->setCursor(0, 40);
            display->print(F("Controllino: "));
            display->println(serialBridge.isConnected() ? "OK" : "---");
            
            // Last response
            String lastResp = serialBridge.getLastResponse();
            if (lastResp.length() > 0) {
                display->setCursor(0, 52);
                if (lastResp.length() > 21) {
                    lastResp = lastResp.substring(0, 18) + "...";
                }
                display->print(lastResp);
            }
            
            display->display();
        }
    }
    #endif
    
    // Small delay to prevent watchdog reset
    delay(1);
}
