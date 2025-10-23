# FairFan ESP32-C6 Arduino IDE Project

ESP32-C6 firmware with 1.47" LCD display for controlling the FairFan motor controller via web interface. This version is specifically designed for Arduino IDE compatibility and Apple Silicon Macs.

## Hardware Requirements

### ESP32-C6 Development Board
- **ESP32-C6 with built-in 1.47" ST7789 LCD display** (recommended)
- Display resolution: 240x240 pixels
- Alternative: ESP32-C6 + separate 1.47" ST7789 display module

### Connections to Controllino MAXI

```
ESP32-C6          →  Controllino MAXI
GPIO4 (TX1)       →  Pin 0 (Serial RX)
GPIO5 (RX1)       →  Pin 1 (Serial TX)  
GND               →  GND

⚠️  WARNING: Disconnect ESP32 from pins 0/1 when programming Controllino!
```

## Arduino IDE Setup

### 1. Install Arduino IDE
- Download Arduino IDE 2.x from https://www.arduino.cc/en/software
- Supports Apple Silicon Macs natively

### 2. Install ESP32 Board Package
1. Open Arduino IDE
2. Go to **File → Preferences**
3. Add this URL to "Additional boards manager URLs":
   ```
   https://espressif.github.io/arduino-esp32/package_esp32_index.json
   ```
4. Go to **Tools → Board → Boards Manager**
5. Search for "esp32"
6. Install **"esp32" by Espressif Systems** (version 3.0.0 or later)

### 3. Select Board
1. Go to **Tools → Board → ESP32 Arduino**
2. Select **"ESP32C6 Dev Module"**

### 4. Install Required Libraries
Go to **Tools → Manage Libraries** and install:

1. **TFT_eSPI** by Bodmer
   - Version 2.5.43 or later
   - For LCD display support

2. **ArduinoJson** by Benoit Blanchon  
   - Version 6.21.3 or later
   - For JSON API communication

3. **WebSockets** by Markus Sattler (optional)
   - Version 2.4.1 or later
   - For future real-time features

### 5. Configure TFT_eSPI Library

**CRITICAL STEP:** The TFT_eSPI library needs configuration for your specific display.

#### Option A: Copy User_Setup.h (Recommended)
1. Locate your Arduino libraries folder:
   - **macOS:** `~/Documents/Arduino/libraries/`
   - **Windows:** `Documents\Arduino\libraries\`
2. Copy the included `User_Setup.h` file to:
   ```
   ~/Documents/Arduino/libraries/TFT_eSPI/User_Setup.h
   ```
3. Replace the existing file when prompted

#### Option B: Manual Configuration
1. Navigate to `~/Documents/Arduino/libraries/TFT_eSPI/`
2. Open `User_Setup_Select.h`
3. Comment out the default setup line
4. Add this line:
   ```cpp
   #include <User_Setups/Setup25_TTGO_T_Display.h>  // Example setup
   ```
5. Edit the setup file with the pin definitions from `User_Setup.h`

## Pin Configuration

The `User_Setup.h` file contains default pin assignments. **You may need to adjust these based on your specific ESP32-C6 LCD board:**

```cpp
#define TFT_MOSI 6   // SDA/MOSI pin
#define TFT_SCLK 7   // SCL/SCLK pin  
#define TFT_CS   10  // Chip Select pin
#define TFT_DC   8   // Data/Command pin
#define TFT_RST  9   // Reset pin
#define TFT_BL   3   // Backlight control pin
```

**Check your board's schematic/documentation for the correct pins!**

## Upload and Run

### 1. Open the Project
1. Open Arduino IDE
2. Open `fairfan-esp32c6.ino`

### 2. Configure Upload Settings
1. **Tools → Board:** ESP32C6 Dev Module
2. **Tools → Port:** Select your ESP32-C6 port (e.g., `/dev/cu.usbserial-***`)
3. **Tools → Upload Speed:** 921600 (or lower if upload fails)

### 3. Upload
1. Press and hold the **BOOT** button on your ESP32-C6
2. Click **Upload** in Arduino IDE
3. Release **BOOT** when "Connecting..." appears
4. Wait for upload to complete

### 4. Monitor Serial Output
1. **Tools → Serial Monitor** 
2. Set baud rate to **115200**
3. You should see startup messages

## Configuration

### WiFi Settings
Edit these constants in the Arduino sketch:

```cpp
// Access Point Mode (default)
const char* AP_SSID = "FairFan-Control";
const char* AP_PASSWORD = "fairfan2025";

// Station Mode (connect to existing WiFi)
const char* STA_SSID = "";        // Enter your WiFi name
const char* STA_PASSWORD = "";    // Enter your WiFi password
```

### Serial Communication
Adjust if your board uses different pins:

```cpp
const uint8_t SERIAL_RX_PIN = 5;  // UART1 RX (connects to Controllino TX pin 1)
const uint8_t SERIAL_TX_PIN = 4;  // UART1 TX (connects to Controllino RX pin 0)
```

**⚠️ IMPORTANT:** Using Controllino's main Serial port (pins 0/1) means you cannot use the Serial Monitor while the ESP32 is connected. Disconnect the ESP32 from pins 0/1 when programming the Controllino.

## Usage

### 1. Power On
- ESP32-C6 boots and shows startup screen on LCD
- LCD displays WiFi status and IP address
- Default: Creates "FairFan-Control" WiFi hotspot

### 2. Connect to Web Interface
1. **WiFi Hotspot Mode (default):**
   - Connect phone/computer to "FairFan-Control" WiFi
   - Password: "fairfan2025"
   - Open browser to: `http://192.168.4.1`

2. **WiFi Station Mode:**
   - Configure `STA_SSID` and `STA_PASSWORD` in code
   - ESP32-C6 connects to your existing WiFi
   - Check LCD for IP address
   - Open browser to displayed IP

### 3. Control Interface
The web interface provides:
- **Motor 1:** Set degrees (0-1080), Go, Stop
- **Motor 2:** Home, Stop  
- **Sequence:** Start, Stop, Soft Stop, Stop All
- **Direction:** Same/Opposite mode
- **Status:** Real-time connection and response display

### 4. LCD Display
Shows real-time status:
- WiFi connection info
- IP address for web access
- Controllino connection status
- Last command response
- System uptime

## Troubleshooting

### Display Issues

**Display not working:**
1. Check `User_Setup.h` pin definitions match your board
2. Try different `SPI_FREQUENCY` (lower = more stable)
3. Change `TFT_RGB_ORDER` from `TFT_RGB` to `TFT_BGR`
4. Verify TFT_eSPI library is properly configured

**Wrong colors:**
- Change `TFT_RGB_ORDER` in `User_Setup.h`

**Display flickers:**
- Lower `SPI_FREQUENCY` in `User_Setup.h`

### Upload Issues

**Upload fails:**
1. Hold BOOT button during upload
2. Try lower upload speed (460800 or 115200)
3. Check USB cable (data capable, not just power)
4. Install CP210x or CH340 drivers if needed

**Port not found:**
1. Install USB-to-serial drivers
2. Check cable connection
3. Try different USB port

### WiFi Issues

**Can't connect to hotspot:**
1. Wait 30 seconds after boot
2. Check WiFi password: "fairfan2025"
3. Try turning WiFi off/on on your device

**Can't access web interface:**
1. Ensure connected to "FairFan-Control" WiFi  
2. Try IP: `192.168.4.1`
3. Check Serial Monitor for actual IP address

### Serial Communication Issues

**No response from Controllino:**
1. Verify wiring: TX→RX, RX→TX, GND→GND
2. Check Controllino is using Serial (pins 0/1)
3. Confirm baud rate 115200 on both devices
4. Test with simple commands like "mode"
5. **⚠️ Disconnect ESP32 when programming Controllino**

**Partial responses:**
1. Check GND connection
2. Ensure clean power supply
3. Try lower baud rate

## Development Notes

### Code Structure
- **Single file design** for Arduino IDE simplicity
- **Modular functions** for easy modification
- **Comprehensive error handling**
- **Real-time status updates**

### Customization
The code is designed to be easily customizable:
- Modify pin assignments at the top
- Adjust display layout in `drawStatusScreen()`
- Add new commands in `handleCommand()`
- Change web interface in `getWebInterfaceHTML()`

### Memory Usage
- Optimized for ESP32-C6 memory constraints
- Uses static JSON documents for predictable memory usage
- String buffering with overflow protection

## Hardware Variants

### Tested Boards
- ESP32-C6-DevKitC-1 with external 1.47" ST7789
- Generic ESP32-C6 LCD development boards

### Alternative Displays
The code can be adapted for other displays:
- 1.3" ST7789 (240x240)
- 1.54" ST7789 (240x240)
- 2.0" ST7789 (320x240)

Just update the resolution and pin definitions in `User_Setup.h`.

## Support

For hardware-specific pin mappings, consult your board's documentation:
- Check product listings for pin diagrams
- Look for board schematics
- Test with simple TFT_eSPI examples first

The code includes extensive serial debug output to help diagnose issues.

## Version History

- **v1.0:** Initial Arduino IDE release
- Consolidated single-file design
- ESP32-C6 optimization
- 1.47" LCD support
- Apple Silicon Mac compatibility