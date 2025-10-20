#ifndef ESP32_CONFIG_H
#define ESP32_CONFIG_H

#include <Arduino.h>

namespace Config {
    // WiFi Configuration
    namespace WiFi {
        // Access Point Mode (for direct phone connection)
        constexpr const char* AP_SSID = "FairFan-Control";
        constexpr const char* AP_PASSWORD = "fairfan2025";  // Minimum 8 characters
        // Note: IPAddress cannot be constexpr, these are defined in WiFiManager.h instead
        // AP_IP = IPAddress(192, 168, 4, 1)
        // AP_GATEWAY = IPAddress(192, 168, 4, 1)
        // AP_SUBNET = IPAddress(255, 255, 255, 0)
        
        // Station Mode (to connect to existing WiFi)
        // Leave empty to use AP mode only
        constexpr const char* STA_SSID = "";
        constexpr const char* STA_PASSWORD = "";
        
        // Connection timeout
        constexpr unsigned long CONNECT_TIMEOUT_MS = 10000;
    }
    
    // Serial Communication with Controllino
    namespace Serial {
        // TTGO T-Display uses Serial2 (ESP32 has Serial0, Serial1, Serial2)
        constexpr uint8_t RX_PIN = 22;  // GPIO22 for Serial2 RX (connects to Controllino TX2 pin 16)
        constexpr uint8_t TX_PIN = 21;  // GPIO21 for Serial2 TX (connects to Controllino RX2 pin 17)
        constexpr unsigned long BAUD_RATE = 9600;
        constexpr unsigned long TIMEOUT_MS = 1000;
    }
    
    // Web Server
    namespace Web {
        constexpr uint16_t HTTP_PORT = 80;
        constexpr uint16_t WEBSOCKET_PORT = 81;
        constexpr unsigned long UPDATE_INTERVAL_MS = 100;  // Status update frequency
    }
    
    // Display (TTGO T-Display 1.14" Built-in Display)
    namespace Display {
        constexpr bool ENABLED = true;
        
        // NOTE: LCD pins are configured in platformio.ini build_flags
        // TFT_MOSI=19, TFT_SCLK=18, TFT_CS=5, TFT_DC=16, TFT_RST=23, TFT_BL=4
        
        // Display specifications
        constexpr uint16_t SCREEN_WIDTH = 135;   // TTGO T-Display 1.14" width
        constexpr uint16_t SCREEN_HEIGHT = 240;  // TTGO T-Display 1.14" height
        constexpr uint8_t ROTATION = 0;          // Screen rotation (0-3)
        constexpr uint8_t BRIGHTNESS = 255;      // Backlight brightness (0-255)
        
        constexpr unsigned long UPDATE_INTERVAL_MS = 500;  // Display refresh rate
    }
    
    // System
    namespace System {
        constexpr unsigned long SERIAL_BUFFER_SIZE = 256;
        constexpr unsigned long WATCHDOG_TIMEOUT_MS = 5000;
    }
}

#endif // ESP32_CONFIG_H
