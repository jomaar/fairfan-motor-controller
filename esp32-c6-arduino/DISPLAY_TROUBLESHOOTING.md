# ESP32-C6 Display Troubleshooting Guide

## Current Status
✅ **Code compiles and uploads successfully**  
✅ **Web server works fine**  
❌ **Display is black/not working**

## Pin Configuration (Config 3 - Previously Working)
```
SCK:  GPIO7   (SPI Clock)
MOSI: GPIO6   (SPI Data)  
CS:   GPIO14  (Chip Select)
DC:   GPIO15  (Data/Command)
RST:  GPIO21  (Reset)
BL:   GPIO22  (Backlight)
```

## Troubleshooting Steps

### 1. Check Serial Monitor Output
After uploading, check the Serial Monitor (115200 baud) for these messages:

**Expected Output:**
```
=== FairFan ESP32-C6 - WORKING CONFIGURATION ===
Using confirmed working pins:
SCK: 7, MOSI: 6, CS: 14, DC: 15, RST: 21, BL: 22
Testing pin configurations...
Testing backlight...
Performing hardware reset...
Calling display->begin()...
SUCCESS: Display initialized!
Forcing display test pattern...
Starting showStartupScreen()...
Filling screen with colors for testing...
```

**If you see "ERROR: Display initialization failed!"** → Hardware connection issue

### 2. Physical Connection Check
Verify these connections on your ESP32-C6:

| Display Pin | ESP32-C6 GPIO | Wire Color (typical) |
|-------------|---------------|---------------------|
| VCC         | 3.3V          | Red                 |
| GND         | GND           | Black               |
| SCL         | GPIO7         | Yellow/Orange       |
| SDA         | GPIO6         | Green/Blue          |
| CS          | GPIO14        | Purple              |
| DC          | GPIO15        | Gray                |
| RST         | GPIO21        | White               |
| BL          | GPIO22        | Brown               |

### 3. Backlight Test
The code now tests both backlight polarities:
- If display shows content but is very dim → Backlight issue
- Try manually connecting BL pin to 3.3V or GND

### 4. Display Test Patterns
The enhanced code now shows:
1. **Color test sequence**: Red → Green → Blue → White → Black
2. **Startup screen** with text and pin information  
3. **Periodic test patterns** every 10 seconds

### 5. Alternative Pin Configurations to Try

If Config 3 doesn't work, try these alternatives:

**Config A (Hardware SPI):**
```cpp
#define TFT_SCK  18   // SPI Clock
#define TFT_MOSI 19   // SPI Data
#define TFT_CS   5    // Chip Select
#define TFT_DC   16   // Data/Command
#define TFT_RST  23   // Reset
#define TFT_BL   4    // Backlight
```

**Config B (Software SPI):**
```cpp
#define TFT_SCK  2    // SPI Clock
#define TFT_MOSI 3    // SPI Data
#define TFT_CS   10   // Chip Select
#define TFT_DC   6    // Data/Command
#define TFT_RST  7    // Reset
#define TFT_BL   8    // Backlight
```

### 6. Common Issues & Solutions

#### Issue: Display completely black
**Solutions:**
1. Check power connections (VCC/GND)
2. Verify all pin connections
3. Try different backlight polarity
4. Check if display requires 5V instead of 3.3V

#### Issue: Display shows colors but no text
**Solutions:**
1. Display is working, font/text issue
2. Check text color isn't same as background
3. Verify cursor positioning

#### Issue: Flickering or unstable display  
**Solutions:**
1. Check power supply stability
2. Add capacitors near display
3. Shorten connection wires
4. Try lower SPI speed

### 7. Hardware Reset Sequence
The code now performs proper reset:
```cpp
digitalWrite(TFT_RST, LOW);   // Reset active
delay(100);
digitalWrite(TFT_RST, HIGH);  // Reset inactive
delay(100);
```

### 8. Next Steps

1. **Upload the enhanced code** and check Serial Monitor
2. **Report what you see** in the serial output
3. **Check physical connections** against the pin table
4. **Try backlight test** - connect BL pin directly to 3.3V
5. **If still black**, try alternative pin configurations

### 9. Diagnostic Questions

1. Do you see the test colors (red, green, blue, white) during startup?
2. What does the Serial Monitor show?
3. Is the backlight turning on (screen getting brighter/dimmer)?
4. Are all connections secure?
5. Is your display exactly 1.47" ST7789 172x320?

The enhanced code will provide much more diagnostic information to help identify the issue! 🔍