# ESP32-C6 Compatibility Notes

## Important Differences

The code was originally written for **ESP32 (WROOM-32)** but can be adapted for **ESP32-C6**.

### Key Changes Needed:

#### 1. Serial Port Configuration
**ESP32 (original):**
- Has Serial, Serial1, Serial2 (3 hardware UARTs)
- Code uses Serial2 on GPIO16/17

**ESP32-C6:**
- Has only UART0 (USB) and UART1 (GPIO pins)
- Must use UART1 (Serial1) instead of Serial2

**Change needed in `include/Config.h`:**
```cpp
// OLD (ESP32):
constexpr uint8_t RX_PIN = 16;  // Serial2 RX
constexpr uint8_t TX_PIN = 17;  // Serial2 TX

// NEW (ESP32-C6):
constexpr uint8_t RX_PIN = 4;   // UART1 RX (GPIO4)
constexpr uint8_t TX_PIN = 5;   // UART1 TX (GPIO5)
```

**Change needed in `include/SerialBridge.h` and `src/main.cpp`:**
```cpp
// OLD:
HardwareSerial& controllinoSerial;
controllinoSerial(Serial2)

// NEW:
HardwareSerial& controllinoSerial;
controllinoSerial(Serial1)
```

#### 2. I2C Pins (if using external OLED)
**ESP32-C6 default I2C:**
- SDA: GPIO6
- SCL: GPIO7

**Change in `include/Config.h`:**
```cpp
// OLD:
constexpr uint8_t I2C_SDA = 21;
constexpr uint8_t I2C_SCL = 22;

// NEW (ESP32-C6):
constexpr uint8_t I2C_SDA = 6;
constexpr uint8_t I2C_SCL = 7;
```

#### 3. PlatformIO Configuration
**Change in `platformio.ini`:**
```ini
# OLD:
[env:esp32dev]
platform = espressif32
board = esp32dev

# NEW (ESP32-C6):
[env:esp32-c6-devkitc-1]
platform = espressif32
board = esp32-c6-devkitc-1
```

## Board-Specific Recommendations

### Option 1: ESP32-C6-LCD-1.47 (RECOMMENDED)

**Advantages:**
- Built-in 1.47" color LCD (240x240, ST7789 driver)
- Better than OLED - color, backlight, larger
- More professional looking

**Pin Mapping (typical for this board):**
- UART1 TX: GPIO4
- UART1 RX: GPIO5
- Available GPIOs: Check your specific board schematic

**Display Changes Needed:**
- Replace Adafruit_SSD1306 library with TFT_eSPI or Adafruit_ST7789
- Adjust display code for color LCD
- Use 240x240 resolution instead of 128x64

### Option 2: Super Mini ESP32-C6

**Advantages:**
- Smaller form factor
- More GPIO available
- Simpler (no built-in display)

**Pin Mapping:**
- UART1 TX: GPIO4 or GPIO16
- UART1 RX: GPIO5 or GPIO17
- I2C SDA: GPIO6 (if adding OLED)
- I2C SCL: GPIO7 (if adding OLED)

## Quick Decision Guide

**Choose ESP32-C6-LCD-1.47 if:**
- ✅ You want a built-in display
- ✅ You want a more polished look
- ✅ You're okay with slightly more code changes

**Choose Super Mini ESP32-C6 if:**
- ✅ You want smallest size
- ✅ You don't need a display
- ✅ You want minimal code changes

## Recommended Approach

**Use the ESP32-C6-LCD-1.47!** Here's why:

1. **Built-in display** - No need for separate OLED
2. **Color LCD** - Better than monochrome OLED
3. **More professional** - Integrated solution
4. **Worth the effort** - Display changes are straightforward

The LCD can show:
- WiFi SSID and IP (color coded)
- Connection status with icons
- Last command/response
- Uptime and statistics
- Even buttons for touch control (if touch-enabled)

## ✅ ESP32-C6-LCD-1.47 Configuration Complete!

All files have been updated for the ESP32-C6-LCD-1.47 board:

### Changed Files:
- ✅ `platformio.ini` - ESP32-C6 board and TFT_eSPI library
- ✅ `include/Config.h` - UART1 pins (GPIO4/5) and LCD config
- ✅ `include/SerialBridge.h` - Changed Serial2 to Serial1
- ✅ `include/DisplayManager.h` - NEW: LCD display manager
- ✅ `src/main.cpp` - Updated for LCD display
- ✅ `User_Setup.h` - NEW: TFT_eSPI configuration

### Pin Mapping (ESP32-C6-LCD-1.47):

**Serial to Controllino:**
- GPIO4 (TX1) → Controllino RX2 (pin 17)
- GPIO5 (RX1) → Controllino TX2 (pin 16)
- GND → GND

**Built-in LCD (internal connections):**
- GPIO6: MOSI
- GPIO7: SCK
- GPIO8: DC
- GPIO9: RST
- GPIO10: CS
- GPIO3: Backlight

### Build and Upload:

```bash
cd esp32
platformio run -e esp32-c6-devkitc-1 --target upload
platformio device monitor --baud 115200
```

### TFT_eSPI Library Setup:

**Important:** Copy `User_Setup.h` to your TFT_eSPI library:

**Option 1 (Recommended):**
```bash
cp User_Setup.h ~/.platformio/lib/TFT_eSPI/User_Setup.h
```

**Option 2:** 
Edit `.platformio/lib/TFT_eSPI/User_Setup_Select.h` and uncomment the line for your setup.

### Expected Display:

The 1.47" color LCD will show:
- **Title:** "FairFan" in cyan
- **WiFi Status:** AP name or WiFi network
- **IP Address:** For web access
- **Controllino:** Connection status (OK/---)
- **Last Response:** Command feedback
- **Uptime:** System runtime

### Testing Checklist:

1. ✅ Upload firmware to ESP32-C6
2. ✅ Check LCD shows "FairFan Initializing..."
3. ✅ Connect to "FairFan-Control" WiFi
4. ✅ LCD shows IP address (192.168.4.1)
5. ✅ Open web browser to 192.168.4.1
6. ✅ Connect ESP32 to Controllino (TX/RX/GND)
7. ✅ Send test command (e.g., "mode")
8. ✅ LCD shows response
9. ✅ Web interface shows response

### Troubleshooting:

**LCD not working:**
- Check TFT_eSPI User_Setup.h is copied
- Verify pin definitions match your board
- Try different TFT_RGB_ORDER (TFT_RGB vs TFT_BGR)

**Serial not working:**
- Verify TX→RX crossover (TX goes to RX)
- Check GND connection
- Confirm baud rate 115200

**WiFi issues:**
- Wait 10 seconds for AP to start
- SSID: "FairFan-Control"
- Password: "fairfan2025"

Ready to build and test! 🚀
