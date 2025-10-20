#ifndef ESP32_CONFIG_H
#define ESP32_CONFIG_H

#include <Arduino.h>

namespace Config {
    // WiFi Configuration
    namespace WiFi {
        // Access Point Mode (for direct phone connection)
        constexpr const char* AP_SSID = "FairFan-Control";
        constexpr const char* AP_PASSWORD = "fairfan2025";  // Minimum 8 characters
        constexpr IPAddress AP_IP = IPAddress(192, 168, 4, 1);
        constexpr IPAddress AP_GATEWAY = IPAddress(192, 168, 4, 1);
        constexpr IPAddress AP_SUBNET = IPAddress(255, 255, 255, 0);
        
        // Station Mode (to connect to existing WiFi)
        // Leave empty to use AP mode only
        constexpr const char* STA_SSID = "";
        constexpr const char* STA_PASSWORD = "";
        
        // Connection timeout
        constexpr unsigned long CONNECT_TIMEOUT_MS = 10000;
    }
    
    // Serial Communication with Controllino
    namespace Serial {
        // ESP32-C6 uses UART1 (not Serial2 like original ESP32)
        constexpr uint8_t RX_PIN = 5;   // ESP32-C6 UART1 RX (connects to Controllino TX2 pin 16)
        constexpr uint8_t TX_PIN = 4;   // ESP32-C6 UART1 TX (connects to Controllino RX2 pin 17)
        constexpr unsigned long BAUD_RATE = 115200;
        constexpr unsigned long TIMEOUT_MS = 1000;
    }
    
    // Web Server
    namespace Web {
        constexpr uint16_t HTTP_PORT = 80;
        constexpr uint16_t WEBSOCKET_PORT = 81;
        constexpr unsigned long UPDATE_INTERVAL_MS = 100;  // Status update frequency
    }
    
    // Display (ESP32-C6-LCD-1.47 Built-in Display)
    namespace Display {
        constexpr bool ENABLED = true;
        
        // LCD Pins (check your specific board - these are typical)
        constexpr uint8_t TFT_CS = 10;      // Chip select
        constexpr uint8_t TFT_DC = 8;       // Data/Command
        constexpr uint8_t TFT_RST = 9;      // Reset
        constexpr uint8_t TFT_MOSI = 6;     // SPI MOSI
        constexpr uint8_t TFT_SCK = 7;      // SPI Clock
        constexpr uint8_t TFT_BL = 3;       // Backlight
        
        // Display specifications
        constexpr uint16_t SCREEN_WIDTH = 172;   // 1.47" round display width
        constexpr uint16_t SCREEN_HEIGHT = 320;  // 1.47" round display height
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
