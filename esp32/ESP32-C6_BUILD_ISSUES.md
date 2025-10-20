# ESP32-C6 Build Issues & Solutions

## Current Status

**Problem:** ESP32-C6 support in PlatformIO/Arduino is still maturing, especially on macOS ARM64 (Apple Silicon).

**Error:** `UnknownPackageError: Could not find the package with 'espressif/toolchain-riscv32-esp @ 13.2.0+20240530' requirements for your system 'darwin_arm64'`

## Important: Check Your Board

**Is your board actually ESP32-C6 or ESP32-S3?**

Many 1.47" round LCD boards use **ESP32-S3**, not ESP32-C6!

### How to Check:

1. Look at the chip markings on your board
2. Check the product page/documentation
3. ESP32-S3: More common, better support
4. ESP32-C6: Newer, limited toolchain on macOS ARM

## Solution Options

### Option 1: ESP32-S3 (Recommended if your board is S3)

Most ESP32-LCD-1.47 boards actually use ESP32-S3. If yours does, use this configuration:

**In `platformio.ini`:**
```ini
[env:esp32s3-devkitc-1]
platform = espressif32
board = esp32-s3-devkitc-1
framework = arduino
```

**Pin changes in `include/Config.h`:**
```cpp
// Serial (ESP32-S3 uses same pins as ESP32)
constexpr uint8_t RX_PIN = 16;  // Or check your board schematic
constexpr uint8_t TX_PIN = 17;
```

### Option 2: ESP32-C6 (if confirmed C6 chip)

If your board is definitely ESP32-C6, you have these options:

####  A. Use Arduino IDE or ESP-IDF directly (not PlatformIO)

Arduino IDE 2.x has better ESP32-C6 support:
1. Install Arduino IDE 2.x
2. Add ESP32 board support: https://espressif.github.io/arduino-esp32/package_esp32_index.json
3. Select "ESP32C6 Dev Module"
4. Compile and upload

#### B. Wait for PlatformIO toolchain update

Track this issue: https://github.com/platformio/platform-espressif32/issues

#### C. Use Linux or Windows (better toolchain support)

The ESP32-C6 toolchain is more stable on x86_64 platforms.

### Option 3: Use Original ESP32 (WROOM-32)

If you have a standard ESP32 board, that works great with external OLED:

**In `platformio.ini`:**
```ini
[env:esp32dev]
platform = espressif32
board = esp32dev
framework = arduino
```

**Back to Config.h pins:**
```cpp
constexpr uint8_t RX_PIN = 16;  // Serial2 RX
constexpr uint8_t TX_PIN = 17;  // Serial2 TX
```

## Recommended Action

1. **Check your actual chip:** ESP32-S3 or ESP32-C6?
2. **If ESP32-S3:** Update config (easy fix!)
3. **If ESP32-C6:** Consider using Arduino IDE for now
4. **Alternative:** Use any ESP32/ESP32-S3 board you have

## ESP32-S3 Configuration (Most Likely)

If your board is ESP32-S3, here's the complete update:

### platformio.ini
```ini
[env:esp32s3-lcd]
platform = espressif32
board = esp32-s3-devkitc-1
framework = arduino
monitor_speed = 115200

build_flags = 
    -DCORE_DEBUG_LEVEL=3
    -DARDUINO_USB_CDC_ON_BOOT=1
    -DARDUINO_USB_MODE=1
    -DESP32_S3_LCD
    -DUSER_SETUP_LOADED=1
    -DST7789_DRIVER=1
    -DTFT_WIDTH=172
    -DTFT_HEIGHT=320
    -DTFT_MOSI=6
    -DTFT_SCLK=7
    -DTFT_CS=10
    -DTFT_DC=8
    -DTFT_RST=9
    -DTFT_BL=3
    -DSPI_FREQUENCY=40000000
    -DTFT_BACKLIGHT_ON=HIGH
    -DTFT_RGB_ORDER=TFT_BGR
    -DLOAD_GLCD=1
    -DLOAD_FONT2=1
    -DLOAD_FONT4=1
    -DLOAD_GFXFF=1
    -DSMOOTH_FONT=1

lib_deps = 
    bblanchon/ArduinoJson@^6.21.3
    links2004/WebSockets@^2.4.1
    bodmer/TFT_eSPI@^2.5.43

board_build.filesystem = littlefs
```

### Config.h (ESP32-S3)
```cpp
namespace Serial {
    // ESP32-S3 UART1 pins (check your specific board!)
    constexpr uint8_t RX_PIN = 18;  // Common S3 UART1 RX
    constexpr uint8_t TX_PIN = 17;  // Common S3 UART1 TX
    constexpr unsigned long BAUD_RATE = 115200;
    constexpr unsigned long TIMEOUT_MS = 1000;
}
```

## Next Steps

**Please check your board and let me know:**
- Chip type (S3 or C6)?
- Board model number?
- Silk screen markings?

Then I'll update the configuration accordingly!

## Why This Happened

ESP32-C6 is very new (2023) and:
- macOS ARM toolchains are still catching up
- PlatformIO support is experimental
- Most LCD boards still use ESP32-S3

ESP32-S3 is more mature and widely supported.
