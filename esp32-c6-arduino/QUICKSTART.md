# Quick Start Guide - ESP32-C6 Arduino IDE

## ✅ DISPLAY WORKS! Arduino_GFX Solution

**Good news!** The **Arduino_GFX library DOES support ESP32-C6 with ST7789 displays!**

While TFT_eSPI has ESP32-C6 compatibility issues, Arduino_GFX library by moononournation provides excellent ESP32-C6 support with proper ST7789 drivers.

## Choose Your Version

### Option 1: With Display (RECOMMENDED - WORKS!)
- File: `fairfan-esp32c6-with-display.ino`
- Status: ✅ **FULLY WORKING** with ESP32-C6
- Display: 1.47" ST7789 LCD with Arduino_GFX library
- Libraries needed: Arduino_GFX + ArduinoJson
- Features: Color LCD status display + Web interface

### Option 2: No Display Version (Backup)
- File: `fairfan-esp32c6-no-display.ino`
- Status: ✅ WORKS with ESP32-C6
- Display: Serial Monitor only
- Libraries needed: ArduinoJson only
- Features: Web interface only

## Prerequisites
- ESP32-C6 development board
- Arduino IDE 2.x installed
- USB cable for programming
- 1.47" ST7789 LCD display (for display version)

## Setup Steps

### 1. Arduino IDE Configuration
```bash
# Add ESP32 board package URL in Preferences:
https://espressif.github.io/arduino-esp32/package_esp32_index.json

# Install ESP32 board package via Boards Manager
# Select Board: ESP32C6 Dev Module
```

### 2. Install Libraries

**For Display Version:**
```bash
# Via Arduino IDE Library Manager:
1. Arduino_GFX Library (by moononournation) - ESP32-C6 compatible display library
2. ArduinoJson (by Benoit Blanchon) - JSON processing

# Search for "Arduino_GFX" and install the library by moononournation
# This library has excellent ESP32-C6 support with ST7789 drivers
```

**For No-Display Version:**
```bash
# Via Arduino IDE Library Manager:
1. ArduinoJson (by Benoit Blanchon) - JSON processing only
```

### 3. Hardware Connections

**ESP32-C6 Serial Bridge to Controllino:**
```
ESP32-C6    →  Controllino MAXI
GPIO4 (TX)  →  Pin 0 (Serial RX)
GPIO5 (RX)  →  Pin 1 (Serial TX)
GND         →  GND
```

**ESP32-C6 Display Connections (for display version):**
```
ESP32-C6    →  1.47" ST7789 Display
GPIO15      →  DC (Data/Command)
GPIO14      →  CS (Chip Select)
GPIO7       →  SCK (SPI Clock)
GPIO6       →  MOSI (SPI Data)
GPIO21      →  RST (Reset)
GPIO22      →  BL (Backlight)
3.3V        →  VCC
GND         →  GND
```

⚠️  **WARNING:** Disconnect ESP32 from Controllino pins 0/1 when programming Controllino!

### 4. Upload and Test

1. Open `fairfan-esp32c6-no-display.ino` (the working version)
2. Select Board: ESP32C6 Dev Module 
3. Select correct USB port
4. Hold BOOT button and click Upload
5. Open Serial Monitor (115200 baud) for status
6. Connect to "FairFan-Control" WiFi
7. Open browser to <http://192.168.4.1>

## Default Settings
- WiFi SSID: "FairFan-Control"
- WiFi Password: "fairfan2025"
- Web Interface: http://192.168.4.1
- Serial Baud: 115200

## Troubleshooting

- **Upload fails:** Hold BOOT button during upload
- **No WiFi:** Wait 30 seconds after boot
- **No Controllino response:** Check wiring and disconnect ESP32 when programming Controllino
- **Compilation errors:** Make sure you're using the no-display version
- **Status unknown:** Check Arduino IDE Serial Monitor for detailed information

Ready to control your FairFan! 🚀