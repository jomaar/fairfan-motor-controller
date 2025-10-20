# Building ESP32-C6 Project with Arduino IDE

## Why Arduino IDE?
PlatformIO doesn't have ESP32-C6 toolchain for macOS ARM64, but Arduino IDE does!

## Installation Steps

### 1. Install Arduino IDE 2.x
Download from: https://www.arduino.cc/en/software  
Choose: **macOS (Apple Silicon)**

### 2. Add ESP32 Board Support
1. Open Arduino IDE
2. Go to: **Preferences** (⌘,)
3. In "Additional boards manager URLs", add:
   ```
   https://espressif.github.io/arduino-esp32/package_esp32_index.json
   ```
4. Click **OK**

### 3. Install ESP32 Boards
1. Go to: **Tools → Board → Boards Manager**
2. Search for: **esp32**
3. Install: **esp32 by Espressif Systems** (version 3.0.0 or newer)
4. Wait for installation to complete

### 4. Install Required Libraries
1. Go to: **Sketch → Include Library → Manage Libraries**
2. Install these libraries:
   - **TFT_eSPI** by Bodmer
   - **ArduinoJson** by Benoit Blanchon (version 7.x)
   - **WebSockets** by Markus Sattler

### 5. Configure TFT_eSPI Library
1. Find the library location:
   - macOS: `~/Documents/Arduino/libraries/TFT_eSPI/`
2. Copy our configuration:
   ```bash
   cp /Users/ja/Library/CloudStorage/OneDrive-Personal/Projects/FairFan/fairfanpio01/esp32/User_Setup.h ~/Documents/Arduino/libraries/TFT_eSPI/User_Setup.h
   ```

### 6. Prepare Project for Arduino IDE
Create a sketch folder with the right structure:

```bash
cd ~/Documents/Arduino
mkdir -p FairFanESP32/
cp /Users/ja/Library/CloudStorage/OneDrive-Personal/Projects/FairFan/fairfanpio01/esp32/src/main.cpp ~/Documents/Arduino/FairFanESP32/FairFanESP32.ino
```

Copy include files:
```bash
cp -r /Users/ja/Library/CloudStorage/OneDrive-Personal/Projects/FairFan/fairfanpio01/esp32/include ~/Documents/Arduino/FairFanESP32/
```

### 7. Open and Configure Project
1. Open Arduino IDE
2. File → Open: `~/Documents/Arduino/FairFanESP32/FairFanESP32.ino`
3. Tools → Board → esp32 → **ESP32C6 Dev Module**
4. Tools → Port → Select your ESP32-C6 USB port
5. Tools → Upload Speed → **921600**
6. Tools → Flash Size → **8MB**

### 8. Build and Upload
1. Click **Verify** (✓) to compile
2. Click **Upload** (→) to flash to board
3. Open **Serial Monitor** (Tools → Serial Monitor)
4. Set baud rate to **115200**

## Expected Output

Serial Monitor should show:
```
Initializing display...
Display initialized
Starting WiFi AP: FairFan-Control
WiFi AP started!
IP Address: 192.168.4.1
Web server started
Serial bridge started
```

LCD should display:
```
FairFan
WiFi AP Mode
SSID: FairFan-Control
IP: 192.168.4.1
Status: Waiting...
```

## Troubleshooting

### "Board not found" error
- Make sure ESP32-C6 board is selected
- Check USB cable supports data (not just power)
- Press BOOT button while uploading

### Compilation errors
- Verify all libraries are installed
- Check TFT_eSPI User_Setup.h was copied correctly
- Make sure Arduino ESP32 boards version is 3.0.0+

### Upload fails
- Hold BOOT button during upload
- Try lower upload speed (460800)
- Check correct port is selected

## Next Steps

Once uploaded:
1. Connect phone to WiFi: **FairFan-Control** (password: **fairfan123**)
2. Open browser: **http://192.168.4.1**
3. Test motor commands!

---

**Note:** This is a one-time setup. After this, you can use Arduino IDE for all ESP32-C6 development on your Mac!
