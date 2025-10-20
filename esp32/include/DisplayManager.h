#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>
#include "Config.h"

#ifdef ESP32_C6_LCD
  #include <TFT_eSPI.h>
#endif

/**
 * DisplayManager - Handles the ESP32-C6-LCD-1.47 built-in display
 * 
 * This class manages the 1.47" color LCD display showing:
 * - WiFi status and IP address
 * - Controllino connection status
 * - Last command/response
 * - System uptime
 */
class DisplayManager {
private:
#ifdef ESP32_C6_LCD
    TFT_eSPI* tft;
#endif
    unsigned long lastUpdate;
    String lastWiFiStatus;
    String lastIPAddress;
    bool lastConnectedStatus;
    String lastResponse;
    
    // Colors (RGB565 format)
    const uint16_t COLOR_BG = 0x0000;        // Black
    const uint16_t COLOR_TEXT = 0xFFFF;      // White
    const uint16_t COLOR_TITLE = 0x07FF;     // Cyan
    const uint16_t COLOR_OK = 0x07E0;        // Green
    const uint16_t COLOR_ERROR = 0xF800;     // Red
    const uint16_t COLOR_WARNING = 0xFFE0;   // Yellow
    const uint16_t COLOR_GRAY = 0x8410;      // Gray
    
public:
    DisplayManager() 
        : lastUpdate(0),
          lastWiFiStatus(""),
          lastIPAddress(""),
          lastConnectedStatus(false),
          lastResponse("") {
#ifdef ESP32_C6_LCD
        tft = nullptr;
#endif
    }
    
    /**
     * Initialize display
     */
    bool begin() {
#ifdef ESP32_C6_LCD
        if (!Config::Display::ENABLED) {
            Serial.println(F("[Display] Disabled in config"));
            return false;
        }
        
        tft = new TFT_eSPI();
        tft->init();
        tft->setRotation(Config::Display::ROTATION);
        tft->fillScreen(COLOR_BG);
        
        // Set backlight
        pinMode(Config::Display::TFT_BL, OUTPUT);
        analogWrite(Config::Display::TFT_BL, Config::Display::BRIGHTNESS);
        
        // Display startup screen
        showStartup();
        
        Serial.println(F("[Display] LCD initialized"));
        return true;
#else
        Serial.println(F("[Display] Not compiled for ESP32-C6-LCD"));
        return false;
#endif
    }
    
    /**
     * Update display with current status
     */
    void update(const String& wifiStatus, const String& ipAddress, 
                bool isConnected, const String& response) {
#ifdef ESP32_C6_LCD
        if (!Config::Display::ENABLED || tft == nullptr) return;
        
        unsigned long now = millis();
        if (now - lastUpdate < Config::Display::UPDATE_INTERVAL_MS) {
            return;  // Not time to update yet
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
#endif
    }
    
private:
#ifdef ESP32_C6_LCD
    /**
     * Show startup screen
     */
    void showStartup() {
        tft->fillScreen(COLOR_BG);
        tft->setTextColor(COLOR_TITLE);
        tft->setTextSize(2);
        
        // Center title
        int16_t x = (Config::Display::SCREEN_WIDTH - 12 * 8) / 2;  // 12 chars * 8 pixels
        tft->setCursor(x, 60);
        tft->println("FairFan");
        
        tft->setTextSize(1);
        x = (Config::Display::SCREEN_WIDTH - 13 * 6) / 2;  // 13 chars * 6 pixels
        tft->setCursor(x, 90);
        tft->setTextColor(COLOR_TEXT);
        tft->println("Motor Control");
        
        tft->setCursor(x - 20, 120);
        tft->setTextColor(COLOR_WARNING);
        tft->println("Initializing...");
        
        delay(2000);
    }
    
    /**
     * Draw current status
     */
    void drawStatus() {
        tft->fillScreen(COLOR_BG);
        
        int16_t y = 10;
        
        // Title
        tft->setTextSize(2);
        tft->setTextColor(COLOR_TITLE);
        tft->setCursor(10, y);
        tft->println("FairFan");
        y += 30;
        
        // Divider line
        tft->drawLine(0, y, Config::Display::SCREEN_WIDTH, y, COLOR_GRAY);
        y += 10;
        
        // WiFi Status
        tft->setTextSize(1);
        tft->setTextColor(COLOR_TEXT);
        tft->setCursor(10, y);
        tft->print("WiFi: ");
        tft->setTextColor(COLOR_OK);
        tft->println(lastWiFiStatus);
        y += 20;
        
        // IP Address
        tft->setTextColor(COLOR_TEXT);
        tft->setCursor(10, y);
        tft->print("IP: ");
        tft->setTextColor(COLOR_TITLE);
        
        // Word wrap IP if too long
        if (lastIPAddress.length() > 15) {
            tft->println();
            tft->setCursor(20, y + 15);
            tft->println(lastIPAddress);
            y += 35;
        } else {
            tft->println(lastIPAddress);
            y += 20;
        }
        
        // Divider line
        tft->drawLine(0, y, Config::Display::SCREEN_WIDTH, y, COLOR_GRAY);
        y += 10;
        
        // Controllino Status
        tft->setTextColor(COLOR_TEXT);
        tft->setCursor(10, y);
        tft->print("Controllino: ");
        if (lastConnectedStatus) {
            tft->setTextColor(COLOR_OK);
            tft->println("OK");
        } else {
            tft->setTextColor(COLOR_ERROR);
            tft->println("---");
        }
        y += 25;
        
        // Last Response (word wrapped)
        if (lastResponse.length() > 0) {
            tft->setTextColor(COLOR_WARNING);
            tft->setCursor(10, y);
            
            // Word wrap response (20 chars per line)
            String resp = lastResponse;
            int maxChars = 20;
            while (resp.length() > 0 && y < Config::Display::SCREEN_HEIGHT - 20) {
                String line = resp.substring(0, min((int)resp.length(), maxChars));
                tft->println(line);
                y += 15;
                tft->setCursor(10, y);
                resp = resp.substring(min((int)resp.length(), maxChars));
            }
        }
        
        // Uptime at bottom
        y = Config::Display::SCREEN_HEIGHT - 20;
        tft->setTextColor(COLOR_GRAY);
        tft->setCursor(10, y);
        unsigned long uptime = millis() / 1000;
        unsigned long hours = uptime / 3600;
        unsigned long minutes = (uptime % 3600) / 60;
        unsigned long seconds = uptime % 60;
        tft->printf("Up: %02luh %02lum %02lus", hours, minutes, seconds);
    }
#endif
};

#endif // DISPLAY_MANAGER_H
