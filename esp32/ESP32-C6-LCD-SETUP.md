# ESP32-C6-LCD-1.47 Setup Guide

## Hardware Specifications

**Board:** ESP32-C6-LCD-1.47  
**Display:** 1.47" Color LCD (172x320, ST7789 driver)  
**WiFi:** 802.11 b/g/n  
**Bluetooth:** BLE 5.0  
**Flash:** 4MB  
**SRAM:** 512KB  

## Pin Connections

### To Controllino MAXI

```
ESP32-C6          Controllino MAXI
─────────────     ────────────────
GPIO4 (TX1)   →   Pin 17 (RX2)
GPIO5 (RX1)   ←   Pin 16 (TX2)
GND           ─   GND
```

**Important:** TX connects to RX (crossover connection)

### Built-in LCD (Internal)

The LCD is already connected internally:
- GPIO6: SPI MOSI
- GPIO7: SPI Clock
- GPIO8: Data/Command
- GPIO9: Reset
- GPIO10: Chip Select
- GPIO3: Backlight Control

## Software Setup

### 1. Install PlatformIO

If not already installed:
```bash
pip install platformio
```

### 2. Configure TFT_eSPI Library

The TFT_eSPI library needs board-specific configuration.

**Copy the configuration file:**
```bash
cd esp32
cp User_Setup.h ~/.platformio/lib/TFT_eSPI/User_Setup.h
```

**Or manually edit** `~/.platformio/lib/TFT_eSPI/User_Setup.h`:
- Set driver: `#define ST7789_DRIVER`
- Set size: `#define TFT_WIDTH 172` and `#define TFT_HEIGHT 320`
- Set pins: GPIO6, GPIO7, GPIO8, GPIO9, GPIO10, GPIO3

### 3. Build Firmware

```bash
cd esp32
platformio run -e esp32-c6-devkitc-1
```

### 4. Upload Firmware

Connect ESP32-C6 via USB:
```bash
platformio run -e esp32-c6-devkitc-1 --target upload
```

### 5. Monitor Serial Output

```bash
platformio device monitor --baud 115200
```

## Expected Behavior

### Startup Sequence

1. **USB Serial shows:**
   ```
   ========================================
      FairFan ESP32 Web Interface
   ========================================
   [Bridge] Serial connection to Controllino initialized
   [Bridge] RX: GPIO5, TX: GPIO4
   [WiFi] Starting Access Point...
   [WiFi] Access Point started
   [WiFi] SSID: FairFan-Control
   [WiFi] IP Address: 192.168.4.1
   [Web] Server started on port 80
   [Display] LCD initialized
   ========================================
      System Ready
   ========================================
   ```

2. **LCD Display shows:**
   - "FairFan" title (cyan, large font)
   - "Motor Control" subtitle
   - "Initializing..." message
   - Then switches to status screen

### Status Screen Layout

```
┌────────────────────┐
│   FairFan          │ ← Large cyan title
├────────────────────┤
│ WiFi: AP Mode      │ ← Green when connected
│ IP: 192.168.4.1    │ ← Cyan text
├────────────────────┤
│ Controllino: OK    │ ← Green if OK, Red if ---
│                    │
│ Motor 1: Started   │ ← Yellow text (last response)
│ 180°               │
│                    │
│ Up: 00h 05m 23s    │ ← Gray text (uptime)
└────────────────────┘
```

## Using the System

### 1. Connect Phone to WiFi

**Settings → WiFi:**
- Network: `FairFan-Control`
- Password: `fairfan2025`

### 2. Open Web Interface

**Browser:** `http://192.168.4.1`

### 3. Control Motors

**Web interface provides:**
- Motor 1 control (set degrees, go, stop)
- Motor 2 control (home, stop)
- Sequence control (start, stop, soft stop, stop all)
- Direction modes (sync, opposite)
- Real-time response log

### 4. Monitor LCD

**The LCD displays:**
- Current WiFi status
- IP address for web access
- Controllino connection status
- Last command/response
- System uptime

## Customization

### Change WiFi Credentials

Edit `include/Config.h`:
```cpp
constexpr const char* AP_SSID = "YourName";
constexpr const char* AP_PASSWORD = "YourPassword";
```

### Adjust Display Brightness

Edit `include/Config.h`:
```cpp
constexpr uint8_t BRIGHTNESS = 255;  // 0-255 (255 = full brightness)
```

### Change Display Rotation

Edit `include/Config.h`:
```cpp
constexpr uint8_t ROTATION = 0;  // 0-3 (90° increments)
```

### Disable Display

Edit `include/Config.h`:
```cpp
constexpr bool ENABLED = false;  // Set to false to disable
```

## Troubleshooting

### LCD Not Working

**Issue:** Display is blank or showing garbage

**Solutions:**
1. Verify `User_Setup.h` is copied to TFT_eSPI library
2. Check pin definitions match your board
3. Try different rotation values (0-3)
4. Try swapping RGB order: `#define TFT_RGB_ORDER TFT_BGR` ↔ `TFT_RGB`

### LCD Shows Wrong Colors

**Solution:** Edit `User_Setup.h`:
```cpp
// Try changing from:
#define TFT_RGB_ORDER TFT_BGR
// To:
#define TFT_RGB_ORDER TFT_RGB
```

### Display Text Too Small/Large

**Solution:** Adjust text sizes in `include/DisplayManager.h`:
```cpp
tft->setTextSize(2);  // 1=small, 2=medium, 3=large
```

### Serial Communication Not Working

**Check:**
1. ✅ TX→RX crossover (GPIO4→Pin17, GPIO5→Pin16)
2. ✅ Common GND connection
3. ✅ Baud rate matches (115200)
4. ✅ Controllino firmware uploaded and running

**Test:** Send "mode" command via web, check LCD for response

### WiFi Connection Issues

**Solutions:**
1. Wait 10 seconds after power-on for AP to start
2. Forget network on phone and reconnect
3. Check password is exactly `fairfan2025`
4. Look for network "FairFan-Control"

### Web Interface Not Loading

**Check:**
1. Connected to correct WiFi network
2. Using correct IP: `192.168.4.1`
3. No VPN active on phone
4. Try different browser

### Upload Fails

**Solutions:**
1. Check USB cable (needs data lines, not just power)
2. Install CP2102 or CH340 drivers
3. Press and hold BOOT button during upload
4. Check correct serial port selected

## Display Pin Verification

If your board has different pins, check the silk screen or manufacturer documentation.

**To find correct pins:**
1. Look at board silk screen markings
2. Check manufacturer's GitHub or wiki
3. Common LCD pin configurations:
   - **Variant A:** MOSI=6, SCK=7, CS=10, DC=8, RST=9, BL=3
   - **Variant B:** MOSI=23, SCK=18, CS=5, DC=2, RST=4, BL=15

Edit `include/Config.h` with correct pins if different.

## Advanced: Touch Screen Support

If your display has touch capability (some variants do):

1. Install touch library:
   ```ini
   lib_deps = 
       ...existing...
       bodmer/TFT_eSPI@^2.5.43
       bodmer/TFT_Touch@^1.0.0
   ```

2. Add touch handling in `DisplayManager.h`
3. Implement touch buttons for direct control

## Performance

**Display Refresh:** 500ms (configurable)  
**Web Response:** <100ms  
**Serial Latency:** <50ms  
**WiFi Range:** ~30m indoor  
**Power:** ~250mA @ 5V  

## Support

**Issues:** Check GitHub issues or open new one  
**Documentation:** See `esp32/README.md` for full details  
**Controllino:** See `controllino/` folder for motor control code  

## Next Steps

Once working:
- Test all motor commands
- Verify LCD updates in real-time
- Try from multiple devices
- Consider enabling Station mode for existing WiFi

Enjoy your ESP32-C6-powered FairFan controller! 🌀
