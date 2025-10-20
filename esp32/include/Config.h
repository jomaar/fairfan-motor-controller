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
        constexpr uint8_t RX_PIN = 16;  // ESP32 RX2 (connects to Controllino TX)
        constexpr uint8_t TX_PIN = 17;  // ESP32 TX2 (connects to Controllino RX)
        constexpr unsigned long BAUD_RATE = 115200;
        constexpr unsigned long TIMEOUT_MS = 1000;
    }
    
    // Web Server
    namespace Web {
        constexpr uint16_t HTTP_PORT = 80;
        constexpr uint16_t WEBSOCKET_PORT = 81;
        constexpr unsigned long UPDATE_INTERVAL_MS = 100;  // Status update frequency
    }
    
    // Display (Optional - SSD1306 128x64 OLED)
    namespace Display {
        constexpr bool ENABLED = true;
        constexpr uint8_t I2C_SDA = 21;
        constexpr uint8_t I2C_SCL = 22;
        constexpr uint8_t I2C_ADDRESS = 0x3C;
        constexpr uint8_t SCREEN_WIDTH = 128;
        constexpr uint8_t SCREEN_HEIGHT = 64;
        constexpr unsigned long UPDATE_INTERVAL_MS = 500;  // Display refresh rate
    }
    
    // System
    namespace System {
        constexpr unsigned long SERIAL_BUFFER_SIZE = 256;
        constexpr unsigned long WATCHDOG_TIMEOUT_MS = 5000;
    }
}

#endif // ESP32_CONFIG_H
