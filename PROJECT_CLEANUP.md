# FairFan Motor Controller - Project Cleanup Summary

**Date:** October 25, 2025  
**Branch:** feature/web-interface

## Overview

Cleaned up project after abandoning ESP32-C6 development due to missing ARM64 toolchain support. Project now focuses on:

- **Controllino MAXI Automation** - Main motor controller
- **TTGO T-Display (ESP32)** - WiFi web interface with built-in 1.14" IPS display

## Changes Made

### 1. Removed ESP32-C6 Configuration

**File:** `platformio.ini` (root)

Removed the `[env:esp32-c6-devkitc-1]` environment section that was attempting to build for ESP32-C6 with ESP-IDF framework.

**Before:**
- Had two environments: `controllino_maxi_automation` and `esp32-c6-devkitc-1`

**After:**
- Single environment: `controllino_maxi_automation`

### 2. Deleted ESP32-C6 Documentation

Removed obsolete documentation files from `esp32/` folder:
- `NEXT_STEPS.md` - ESP32-C6 specific next steps
- `PHASE1_COMPLETE.md` - ESP32-C6 phase completion notes  
- `PLATFORMIO_ARM64_SUPPORT.md` - ARM64 toolchain issues with C6

### 3. Removed ESP32-C6 Board Definition

Deleted:
- `esp32/boards/esp32c6-lcd.json` - Custom board definition for ESP32-C6
- `esp32/boards/` directory (now empty)

### 4. Cleaned Up Build Artifacts

Attempted to remove (if existed):
- `.pio/libdeps/esp32-c6-devkitc-1/` - ESP32-C6 library dependencies

### 5. Updated Documentation

**Main README.md:**
- Clarified that ESP32 web interface uses **TTGO T-Display** (ESP32, not ESP32-C6)
- Added hardware specifications for TTGO T-Display
- Updated setup instructions

**esp32/README.md:**
- Updated title to "ESP32 Web Interface (TTGO T-Display)"
- Added TTGO T-Display hardware specifications
- Removed references to optional OLED display
- Updated to reflect built-in 1.14" IPS LCD display
- Clarified GPIO pins (17/16 for Serial2)
- Updated display configuration section for TFT_eSPI library

## Current Project Structure

```
fairfan-motor-controller/
├── platformio.ini              # Controllino MAXI config only
├── README.md                   # Updated with TTGO T-Display info
├── WIRING_GUIDE.md            # TTGO T-Display wiring (already correct)
├── REFACTORING_SUMMARY.md     # OOP refactoring documentation
├── LICENSE                     # MIT License
├── include/                    # Controllino headers
│   ├── CommandHandler.h
│   ├── Config.h
│   ├── MainMotor.h
│   ├── OscillationMotor.h
│   ├── SequenceStateMachine.h
│   └── StepperMotor.h
├── src/                        # Controllino source (empty - needs main file)
├── lib/                        # PlatformIO libraries
├── test/                       # Test files
├── controllino/                # Controllino-specific docs
│   ├── SERIAL_PORTS.md
│   └── SERIAL2_UPDATE_NEEDED.md
└── esp32/                      # TTGO T-Display web interface
    ├── platformio.ini          # TTGO T-Display config
    ├── README.md               # Updated for TTGO T-Display
    ├── QUICKSTART.md
    ├── WIRING.md
    ├── TTGO-DISPLAY.md
    ├── TTGO-DISPLAY-TROUBLESHOOTING.md
    ├── UPLOAD_TROUBLESHOOTING.md
    ├── include/
    │   ├── Config.h
    │   ├── DisplayManager.h
    │   ├── SerialBridge.h
    │   ├── WebServerManager.h
    │   └── WiFiManager.h
    └── src/
        └── main.cpp            # TTGO T-Display firmware
```

## Hardware Configuration

### Controllino MAXI Automation
- Platform: ATmega2560
- Motor 1: Step=6, Dir=4
- Motor 2: Step=7, Dir=8, Enable=9
- Limit switches: DI0, DI1
- Serial: USB (pins 0/1) at 115200 baud

### TTGO T-Display (ESP32)
- Platform: ESP32-WROOM-32
- Display: 1.14" IPS LCD (ST7789, 135x240)
- Serial Bridge: GPIO17 (TX2), GPIO16 (RX2) at 115200 baud
- WiFi AP: "FairFan-Control" / "fairfan2025"
- Web Server: http://192.168.4.1

## Wiring Between Devices

```
ESP32 TTGO T-Display       Controllino MAXI X1
────────────────────       ────────────────────
GPIO 17 (TX2)         →    Pin 0 (RX0)
GPIO 16 (RX2)         ←    Pin 1 (TX0)
GND                   →    GND
```

⚠️ **Important:** Disconnect Controllino USB when ESP32 is wired to pins 0/1!

## What Was NOT Changed

- Controllino firmware headers in `include/`
- ESP32 web interface firmware in `esp32/src/main.cpp`
- TTGO T-Display platformio.ini (already correct)
- Wiring guide (already documented TTGO T-Display)
- Library dependencies for either platform

## Next Steps

### To Build Controllino Firmware:
1. Ensure Controllino source code exists in `src/` folder (e.g., `fairfanpio.cpp`)
2. Run: `platformio run -e controllino_maxi_automation`

### To Build ESP32 Web Interface:
1. Navigate to `esp32/` folder
2. Run: `platformio run -e ttgo-t-display`

### To Upload:
```bash
# Controllino
platformio run -e controllino_maxi_automation --target upload

# ESP32 (from esp32/ folder)
cd esp32
platformio run -e ttgo-t-display --target upload
```

## Why ESP32-C6 Was Abandoned

The ESP32-C6 uses a RISC-V architecture that requires a newer toolchain (`toolchain-riscv32-esp @ 13.2.0+20240530`) which is not yet available for ARM64 macOS. PlatformIO fails to download/compile the toolchain on Apple Silicon Macs.

**Solution:** Use TTGO T-Display with original ESP32 chip, which has excellent PlatformIO support on all platforms including ARM64 macOS.

## Benefits of TTGO T-Display

✅ Built-in 1.14" IPS color display (no additional wiring)  
✅ Compact form factor  
✅ USB-C connector  
✅ Works perfectly with PlatformIO on ARM64 Mac  
✅ Lower cost than ESP32-C6 boards  
✅ Mature libraries and support  
✅ Dual-core, more RAM than C6  

## Cleanup Checklist

- [x] Remove ESP32-C6 environment from root platformio.ini
- [x] Delete ESP32-C6 documentation files
- [x] Delete ESP32-C6 board definition
- [x] Update main README.md
- [x] Update esp32/README.md  
- [x] Create this cleanup summary document

## Files That May Need Attention

### Controllino Source Code
The `src/` folder is currently empty. You may need to:
- Copy Controllino firmware from a backup
- Or create a new main file if starting fresh

### Controllino Documentation
The `controllino/` folder has notes about serial ports:
- `SERIAL_PORTS.md` - Information about Controllino serial ports
- `SERIAL2_UPDATE_NEEDED.md` - Notes about Serial2 updates

These may be outdated if you're using Serial0 (USB pins 0/1) as documented in WIRING_GUIDE.md.

---

**Cleanup completed successfully!** 🎉

Project is now focused on Controllino MAXI + TTGO T-Display architecture.
