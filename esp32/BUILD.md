# Build Instructions

## Easy Way: Use VS Code PlatformIO Extension

1. **Open project in VS Code:**
   - File → Open Folder → Select `fairfanpio01/esp32`

2. **Click PlatformIO icon** in left sidebar

3. **Expand "esp32-c6-devkitc-1"**

4. **Click "Build"** under Project Tasks

5. **Click "Upload"** to flash to board

## Command Line (if PlatformIO installed)

### Build:
```bash
cd esp32
platformio run -e esp32-c6-devkitc-1
```

### Upload:
```bash
platformio run -e esp32-c6-devkitc-1 --target upload
```

### Monitor:
```bash
platformio device monitor --baud 115200
```

## First Time Setup

### Install PlatformIO (choose one):

**Option 1: VS Code Extension (Recommended)**
1. Open VS Code
2. Extensions (Cmd+Shift+X)
3. Search "PlatformIO IDE"
4. Click Install

**Option 2: Command Line**
```bash
pip3 install platformio
```

## TFT_eSPI Configuration

**Good news:** No manual configuration needed!

The TFT_eSPI display driver is configured automatically through `platformio.ini` build flags. All pin definitions and settings are included.

## Troubleshooting

### "Command not found: platformio"

**Solution:** Use VS Code PlatformIO extension instead, or install:
```bash
pip3 install platformio
```

### "Library not found"

**Solution:** PlatformIO will download libraries automatically on first build. Just wait for it to complete.

### Build fails

**Check:**
1. ✅ Internet connection (needs to download libraries)
2. ✅ Correct folder (must be in `esp32/` directory)
3. ✅ VS Code PlatformIO extension installed

## Build Output

**First build:** ~2-3 minutes (downloading libraries)  
**Subsequent builds:** ~30-45 seconds  

**Expected libraries downloaded:**
- ArduinoJson
- WebSockets
- TFT_eSPI
- ESP32 core libraries

## Success Indicators

✅ **Build success:** "SUCCESS" message, .bin file created  
✅ **Upload success:** "Writing at..." progress, "Hard resetting via RTS pin..."  
✅ **Running:** LCD shows "FairFan Initializing..."

## Need Help?

See full documentation:
- `ESP32-C6-LCD-SETUP.md` - Complete setup guide
- `ESP32-C6_NOTES.md` - Technical details
- `README.md` - General information
