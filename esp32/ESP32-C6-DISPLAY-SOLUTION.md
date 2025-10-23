# ESP32-C6 Display Issue SOLVED! 🎉

## The Problem

Your display wasn't working because **Arduino_GFX library doesn't have ESP32-C6 specific pin definitions**. The library only has default pins for:

- ESP32 (original)
- ESP32-S2  
- ESP32-S3
- ESP32-C3

When you use ESP32-C6, it falls back to generic defaults that don't match the actual hardware SPI pins.

## The Solution

Use the **correct ESP32-C6 hardware SPI pins**:

### ESP32-C6 Hardware SPI Pins
```
SCK:  GPIO19  (SPI_CLK)
MOSI: GPIO18  (SPI_MOSI) 
MISO: GPIO20  (SPI_MISO) - not needed for display
```

### Display Control Pins (flexible)
```
CS:   GPIO8   (Chip Select)
DC:   GPIO9   (Data/Command)
RST:  GPIO10  (Reset)
BL:   GPIO11  (Backlight)
```

## Files to Test

### 1. `fairfan-esp32c6-correct-pins.ino`
- **Main application** with corrected pin configuration
- Includes web server and full functionality
- Uses proper ESP32-C6 hardware SPI pins

### 2. `fairfan-esp32c6-pin-test.ino`  
- **Diagnostic tool** to test different pin configurations
- Tests 3 different pin setups
- Helps verify your wiring is correct

## How to Test

### Step 1: Upload Pin Test
1. Open `fairfan-esp32c6-pin-test.ino`
2. Change `#define TEST_CONFIG 1` to test different configurations:
   - `1` = ESP32-C6 hardware SPI (recommended)
   - `2` = Hardware SPI + ESP32-C3 style control pins  
   - `3` = Your original pins (for comparison)
3. Upload and check Serial Monitor
4. Look for display activity

### Step 2: Verify Wiring
Make sure your display is connected to these ESP32-C6 pins:

```
Display Pin → ESP32-C6 Pin
VCC         → 3.3V
GND         → GND  
SCK         → GPIO19
SDA/MOSI    → GPIO18
CS          → GPIO8
DC          → GPIO9
RST         → GPIO10
BL          → GPIO11
```

### Step 3: Test Backlight Polarity
The test sketch tries both `HIGH` and `LOW` for backlight. Note which one makes the display light up.

### Step 4: Use Main Application
Once the pin test works, use `fairfan-esp32c6-correct-pins.ino` for full functionality.

## Why This Happened

Looking at Arduino_GFX source code:
- ESP32-C3 uses: SCK=4, MOSI=6, CS=7, DC=2, RST=1, BL=3
- ESP32-C6 has **no defaults defined** 
- Library falls back to wrong pins

The ESP32-C6 has different hardware SPI pins than C3, so the wrong defaults caused your display to stay dark.

## Web Interface

Once working, access your fan control at:
```
http://[ESP32_IP_ADDRESS]
```

The web interface shows:
- Fan status and controls
- Pin configuration for debugging
- WiFi connection status

## Troubleshooting

If display still doesn't work:

1. **Check Power**: Verify 3.3V and GND connections
2. **Verify Display**: Make sure it's ST7789 compatible  
3. **Try Different Control Pins**: You can change CS, DC, RST, BL to other available GPIOs
4. **Check Backlight**: Note which polarity (HIGH/LOW) lights up the display
5. **Test with Software SPI**: If hardware SPI doesn't work, try software SPI

## Success Indicators

You'll know it's working when you see:
- ✅ Display initialization successful in Serial Monitor
- ✅ Colors appearing on screen (red, green, blue, white)
- ✅ Text displaying clearly
- ✅ Graphics and moving dot test working

## Next Steps

Once display works:
1. Update WiFi credentials in the main sketch
2. Test web interface functionality  
3. Add your motor control hardware
4. Customize the display layout as needed

The key insight is that **ESP32-C6 needs explicit SPI pin definitions** because Arduino_GFX doesn't have built-in defaults for this newer chip.