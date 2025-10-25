# TTGO T-Display Upload Troubleshooting

## Issue: No USB Serial Port Detected

Your Mac isn't detecting the TTGO T-Display USB connection.

## Solutions

### 1. Check USB Connection

**Make sure:**
- USB-C cable is fully inserted into TTGO T-Display
- USB cable is **data-capable** (not just a charging cable)
- USB cable is connected to your Mac
- TTGO T-Display has power (you might see the backlight)

### 2. Install USB Drivers (if needed)

TTGO T-Display usually uses **CP210x** or **CH340** USB-to-serial chip.

**Check which chip your board has:**
- Look at the small chip near the USB connector
- CP2104 or CH340G marking

**Install drivers:**

**For CP210x:**
```bash
# Usually works without drivers on macOS, but if needed:
# Download from: https://www.silabs.com/developers/usb-to-uart-bridge-vcp-drivers
```

**For CH340:**
```bash
# Download CH340 driver for macOS:
# https://github.com/adrianmihalko/ch340g-ch34g-ch34x-mac-os-x-driver
```

### 3. Check for USB Device

```bash
# List all USB devices
system_profiler SPUSBDataType | grep -A 10 "CP210\|CH340\|USB Serial\|UART"
```

### 4. Try Different USB Port/Cable

- Try a different USB port on your Mac
- Try a different USB-C cable (must support data!)
- Some USB-C cables are charging-only

### 5. Put Board in Download Mode Manually

**For TTGO T-Display:**

1. **Hold BOOT button** (usually labeled BOOT or IO0)
2. While holding BOOT, **press and release RESET** button
3. Release BOOT button
4. Board is now in download mode
5. Try upload again: `pio run -t upload`

### 6. Check Port After Connection

Once connected, you should see something like:
```bash
/dev/cu.usbserial-*       # For CP210x
/dev/cu.wchusbserial*     # For CH340
/dev/cu.SLAB_USBtoUART    # For CP210x (alternate)
```

### 7. Manual Port Specification

Once you find the correct port, update `platformio.ini`:

```ini
[env:ttgo-t-display]
platform = espressif32
board = lilygo-t-display
framework = arduino
monitor_speed = 115200

; Specify the correct port
upload_port = /dev/cu.usbserial-*  ; Replace with your actual port
monitor_port = /dev/cu.usbserial-* ; Replace with your actual port
```

## Quick Test

**1. Connect TTGO T-Display via USB-C**

**2. Run this command:**
```bash
ls -la /dev/cu.* | grep -v "Bluetooth\|debug"
```

**3. You should see a new device appear like:**
- `/dev/cu.usbserial-0001`
- `/dev/cu.wchusbserial14320`
- `/dev/cu.SLAB_USBtoUART`

**4. If you see it, update platformio.ini with that port**

**5. If you DON'T see it:**
- Check cable (try different one)
- Check drivers are installed
- Try manual boot mode (hold BOOT + press RESET)

## Alternative: Use Arduino IDE for First Upload

If PlatformIO upload keeps failing:

1. Open Arduino IDE
2. Tools → Board → ESP32 → ESP32 Dev Module
3. Tools → Port → Select your TTGO port
4. File → Open → Browse to your `.pio/build/ttgo-t-display/firmware.bin`
5. Use ESP32 Sketch Data Upload tool

## Current Status

**What we know:**
- ✅ Project builds successfully
- ❌ USB port not detected
- Need to verify USB connection and drivers

**Next steps:**
1. Check USB cable is data-capable
2. Install drivers if needed (CH340 or CP210x)
3. Try manual boot mode (BOOT + RESET buttons)
4. Find the correct `/dev/cu.*` port
5. Update platformio.ini with correct port

---

**Once you see the USB port, let me know and I'll help configure it!**
