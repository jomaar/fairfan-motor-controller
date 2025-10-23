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
        
        tft->init();
        Serial.println(F("[Display] TFT init() called"));
        
        tft->setRotation(Config::Display::ROTATION);
        Serial.print(F("[Display] Rotation set to: "));
        Serial.println(Config::Display::ROTATION);
        
        // Set backlight FIRST (for TTGO T-Display, pin 4)
        #ifdef TTGO_T_DISPLAY
        pinMode(4, OUTPUT);
        digitalWrite(4, HIGH);  // Turn on backlight
        Serial.println(F("[Display] TTGO T-Display backlight ON (GPIO 4)"));
        #else
        pinMode(Config::Display::TFT_BL, OUTPUT);
        analogWrite(Config::Display::TFT_BL, Config::Display::BRIGHTNESS);
        #endif
        
        // Test with color fills
        Serial.println(F("[Display] Testing with color fills..."));
        tft->fillScreen(TFT_RED);
        delay(500);
        tft->fillScreen(TFT_GREEN);
        delay(500);
        tft->fillScreen(TFT_BLUE);
        delay(500);
        tft->fillScreen(COLOR_BG);  // Black
        Serial.println(F("[Display] Color test complete"));
        
        // Skip startup screen - go directly to black screen
        Serial.println(F("[Display] Skipping startup screen (stability)"));
        delay(1000);  // Brief pause to see color test
        
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
                bool isConnected, const String& response) {
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
#ifdef HAS_TFT_DISPLAY
    /**
     * Show startup screen
     */
    void showStartup() {
        tft->fillScreen(COLOR_BG);
        
        // Use simple text rendering with background color to avoid font issues
        // Title in landscape mode
        tft->setTextSize(3);
        tft->setTextColor(COLOR_TITLE, COLOR_BG);
        tft->setCursor(40, 30);
        tft->print("FairFan");
        
        tft->setTextSize(1);
        tft->setTextColor(COLOR_TEXT, COLOR_BG);
        tft->setCursor(35, 70);
        tft->print("Motor Control");
        
        tft->setCursor(55, 95);
        tft->setTextColor(COLOR_WARNING, COLOR_BG);
        tft->print("Starting...");
        
        delay(2000);
    }
    
    /**
     * Draw current status
     */
    void drawStatus() {
        tft->fillScreen(COLOR_BG);
        
        int16_t y = 5;
        
        // Title (smaller for landscape)
        tft->setTextSize(2);
        tft->setTextColor(COLOR_TITLE, COLOR_BG);
        tft->setCursor(5, y);
        tft->print("FairFan");
        y += 20;
        
        // Divider line
        tft->drawLine(0, y, Config::Display::SCREEN_WIDTH, y, COLOR_GRAY);
        y += 3;
        
        // WiFi Status
        tft->setTextSize(1);
        tft->setTextColor(COLOR_TEXT, COLOR_BG);
        tft->setCursor(5, y);
        tft->print("WiFi: ");
        tft->setTextColor(COLOR_OK, COLOR_BG);
        tft->print(lastWiFiStatus);
        y += 12;
        
        // IP Address
        tft->setTextColor(COLOR_TEXT, COLOR_BG);
        tft->setCursor(5, y);
        tft->print("IP: ");
        tft->setTextColor(COLOR_TITLE, COLOR_BG);
        tft->print(lastIPAddress);
        y += 12;
        
        // Divider line
        tft->drawLine(0, y, Config::Display::SCREEN_WIDTH, y, COLOR_GRAY);
        y += 3;
        
        // Controllino Status
        tft->setTextColor(COLOR_TEXT, COLOR_BG);
        tft->setCursor(5, y);
        tft->print("Ctrl: ");
        if (lastConnectedStatus) {
            tft->setTextColor(COLOR_OK, COLOR_BG);
            tft->print("OK");
        } else {
            tft->setTextColor(COLOR_ERROR, COLOR_BG);
            tft->print("---");
        }
        y += 12;
        
        // Last Response (single line, truncated if needed)
        if (lastResponse.length() > 0) {
            tft->setTextColor(COLOR_WARNING, COLOR_BG);
            tft->setCursor(5, y);
            tft->print("Resp: ");
            String resp = lastResponse;
            if (resp.length() > 30) {
                resp = resp.substring(0, 27) + "...";
            }
            tft->print(resp);
            y += 12;
        }
        
        // Uptime at bottom
        y = Config::Display::SCREEN_HEIGHT - 15;
        tft->setTextColor(COLOR_GRAY, COLOR_BG);
        tft->setCursor(5, y);
        unsigned long uptime = millis() / 1000;
        unsigned long minutes = uptime / 60;
        unsigned long seconds = uptime % 60;
        tft->print("Up:");
        tft->print(minutes);
        tft->print("m ");
        tft->print(seconds);
        tft->print("s");
    }
#endif
};

#endif // DISPLAY_MANAGER_H
