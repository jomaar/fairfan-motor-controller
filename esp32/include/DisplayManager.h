#ifndef DISPLAY_MANAGER_H
#define DISPLAY_MANAGER_H

#include <Arduino.h>
#include "Config.h"

#if defined(TTGO_T_DISPLAY) || defined(ESP32_C6_LCD) || defined(ESP32_S3_LCD)
  #include <TFT_eSPI.h>
  #define HAS_TFT_DISPLAY
#endif

/**
 * DisplayManager - Handles TFT LCD displays (TTGO T-Display, ESP32-C6-LCD, etc.)
 * 
 * This class manages the 1.47" color LCD display showing:
 * - WiFi status and IP address
 * - Controllino connection status
 * - Last command/response
 * - System uptime
 */
class DisplayManager {
private:
#ifdef HAS_TFT_DISPLAY
    TFT_eSPI* tft;
#endif
    unsigned long lastUpdate;
    String lastWiFiStatus;
    String lastIPAddress;
    bool lastConnectedStatus;
    String lastCommand;
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
          lastCommand(""),
          lastResponse("") {
#ifdef HAS_TFT_DISPLAY
        tft = nullptr;
#endif
    }
    
    /**
     * Initialize display
     */
    bool begin() {
#ifdef HAS_TFT_DISPLAY
        if (!Config::Display::ENABLED) {
            Serial.println(F("[Display] Disabled in config"));
            return false;
        }
        
        Serial.println(F("[Display] Initializing TFT LCD..."));
        
        tft = new TFT_eSPI();
        Serial.println(F("[Display] TFT_eSPI object created"));
        
        // Turn on backlight FIRST (critical for TTGO T-Display)
        #ifdef TTGO_T_DISPLAY
        Serial.println(F("[Display] Setting up TTGO T-Display backlight..."));
        pinMode(4, OUTPUT);
        digitalWrite(4, HIGH);  // Turn on backlight
        Serial.println(F("[Display] Backlight GPIO 4 set HIGH"));
        delay(100);  // Give backlight time to power up
        #else
        pinMode(Config::Display::TFT_BL, OUTPUT);
        analogWrite(Config::Display::TFT_BL, Config::Display::BRIGHTNESS);
        #endif
        
        tft->init();
        Serial.println(F("[Display] TFT init() called"));
        
        tft->setRotation(Config::Display::ROTATION);
        Serial.print(F("[Display] Rotation set to: "));
        Serial.println(Config::Display::ROTATION);
        
        // Just fill with a color to test - NO TEXT
        Serial.println(F("[Display] Filling screen with cyan..."));
        tft->fillScreen(COLOR_TITLE);  // Cyan
        delay(1000);
        tft->fillScreen(COLOR_BG);     // Black
        
        Serial.println(F("[Display] LCD initialized successfully"));
        return true;
#else
        Serial.println(F("[Display] Not compiled with TFT display support"));
        Serial.println(F("[Display] Define TTGO_T_DISPLAY, ESP32_C6_LCD, or ESP32_S3_LCD"));
        return false;
#endif
    }
    
    /**
     * Update display with current status
     */
    void update(const String& wifiStatus, const String& ipAddress, 
                bool isConnected, const String& command, const String& response) {
#ifdef HAS_TFT_DISPLAY
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
                          (command != lastCommand) ||
                          (response != lastResponse);
        
        if (!needsRedraw) return;
        
        // Store current values
        lastWiFiStatus = wifiStatus;
        lastIPAddress = ipAddress;
        lastConnectedStatus = isConnected;
        lastCommand = command;
        lastResponse = response;
        
        // Redraw screen
        drawStatus();
#endif
    }
    
private:
#ifdef HAS_TFT_DISPLAY
    /**
     * Show startup screen
     */
    void showStartup() {
        tft->fillScreen(COLOR_BG);
        
        // Use simple text rendering with background color to avoid font issues
        // Title in landscape mode
        tft->setTextSize(4);
        tft->setTextColor(COLOR_TITLE, COLOR_BG);
        tft->setCursor(40, 30);
        tft->print("FairFan");
        
        tft->setTextSize(4);
        tft->setTextColor(COLOR_TEXT, COLOR_BG);
        tft->setCursor(35, 70);
        tft->print("Motor Control");
        
        tft->setCursor(55, 95);
        tft->setTextColor(COLOR_WARNING, COLOR_BG);
        tft->print("Starting...");
        
        delay(2000);
    }
    
    /**
     * Draw current status (using drawString instead of print)
     */
    void drawStatus() {
        tft->fillScreen(COLOR_BG);
        
        // Try drawString instead of print
        tft->setTextColor(COLOR_TITLE);
        tft->drawString("FairFan", 10, 10, 4);  // x, y, font
        
        tft->setTextColor(COLOR_OK);
        tft->drawString("WiFi OK", 10, 35, 4);
        
        tft->setTextColor(COLOR_TITLE);
        tft->drawString("192.168.4.1", 10, 60, 4);
        
        if (lastConnectedStatus) {
            tft->setTextColor(COLOR_OK);
            tft->drawString("Ctrl: OK", 10, 85, 4);
        } else {
            tft->setTextColor(COLOR_ERROR);
            tft->drawString(lastCommand, 10, 85, 4);
        }
    }
#endif
};

#endif // DISPLAY_MANAGER_H
