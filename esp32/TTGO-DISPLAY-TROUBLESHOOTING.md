# TTGO T-Display Black Screen Troubleshooting

## Symptoms
Display backlight is on but screen remains black.

## Quick Checks

### 1. Press RESET Button
- Press the **RIGHT button** on the TTGO T-Display
- Watch for color flashes (red → green → blue)
- Check serial monitor for detailed init messages

### 2. Check Rotation
The display might be rotated incorrectly. Try different rotations:

In `include/Config.h`, change:
```cpp
constexpr uint8_t ROTATION = 0;  // Try 0, 1, 2, or 3
```

Values:
- `0` = Portrait (0°)
- `1` = Landscape (90°)
- `2` = Portrait inverted (180°)
- `3` = Landscape inverted (270°)

### 3. Check TFT_eSPI Configuration
The TFT_eSPI library uses build flags from `platformio.ini`.

Current settings:
```ini
-DTFT_WIDTH=135
-DTFT_HEIGHT=240
-DTFT_MOSI=19
-DTFT_SCLK=18
-DTFT_CS=5
-DTFT_DC=16
-DTFT_RST=23
-DTFT_BL=4
```

### 4. Test with Simple Code
Run the test file `test-display.cpp`:

```bash
# Temporarily rename main.cpp
mv src/main.cpp src/main.cpp.backup

# Copy test file
cp test-display.cpp src/main.cpp

# Upload
pio run -t upload

# Watch serial monitor
pio device monitor
```

You should see:
1. Red screen for 1 second
2. Green screen for 1 second  
3. Blue screen for 1 second
4. Black screen with white text "TTGO T-Display" and "Test OK!"

### 5. Check Backlight
The backlight is on GPIO 4. Verify it's actually on:

```cpp
pinMode(4, OUTPUT);
digitalWrite(4, HIGH);  // ON
// or
digitalWrite(4, LOW);   // OFF (try toggling)
```

### 6. Verify Board Variant
Some TTGO T-Display boards have slightly different configurations:

**Original T-Display:**
- 1.14" LCD (135x240)
- ST7789 driver
- No touch

**T-Display-S3:**
- 1.9" LCD (170x320)
- Different pin configuration

Make sure you have the original T-Display, not T-Display-S3.

### 7. Common Issues

**Issue:** Backlight on but no image
- **Cause:** Wrong SPI pins or chip select
- **Fix:** Verify pins match your board schematic

**Issue:** Display works but upside down
- **Cause:** Wrong rotation setting
- **Fix:** Change `ROTATION` in Config.h

**Issue:** Display shows garbage/noise
- **Cause:** Wrong driver or color order
- **Fix:** Check `-DST7789_DRIVER=1` in build flags

## Expected Serial Output (After Fix)

```
[Display] Initializing TFT LCD...
[Display] TFT_eSPI object created
[Display] TFT init() called
[Display] Rotation set to: 0
[Display] TTGO T-Display backlight ON (GPIO 4)
[Display] Testing with color fills...
[Display] Color test complete
[Display] Showing startup screen...
[Display] LCD initialized successfully
```

## What to Report

If still not working, please provide:

1. **Serial monitor output** after pressing RESET
2. **Board markings** - what's written on the PCB?
3. **Screen behavior** - completely black? Backlight visible? Any flicker?
4. **Button test** - do the two buttons (BOOT/RESET) work?

## Quick Fix Commands

```bash
# Rebuild and upload with verbose output
cd esp32
pio run -t upload -v

# Monitor with filter for Display messages only
pio device monitor | grep Display
```

---

**Most likely cause:** Rotation setting or the display needs a hard power cycle. Try unplugging the USB cable, wait 5 seconds, plug back in.
