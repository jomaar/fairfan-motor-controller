# ESP32-C6 vs ESP32-S3 Changes

## Overview
This directory contains the ESP32-C6 version of the FairFan controller firmware, specifically adapted for Arduino IDE use with a 1.47" LCD display.

## Key Differences from ESP32-S3 Version

### Hardware Changes

#### Microcontroller
- **Old:** ESP32-S3 (dual-core, WiFi + Bluetooth)
- **New:** ESP32-C6 (single-core, WiFi 6 + Bluetooth 5, Zigbee support)

#### Display
- **Old:** 1.14" ST7735 LCD (240x135) on TTGO T-Display
- **New:** 1.47" ST7789 LCD (240x240) standalone module
- **Benefit:** Larger, square display with better readability

#### UART Configuration  
- **Old:** Uses Serial2 (UART2) on GPIO16/17
- **New:** Uses Serial1 (UART1) on GPIO4/5, connects to Controllino Serial (pins 0/1)
- **Reason:** ESP32-C6 has fewer hardware UARTs
- **⚠️ Important:** Cannot use Serial Monitor while ESP32 is connected to Controllino

### Software Changes

#### Development Environment
- **Old:** PlatformIO (not compatible with Apple Silicon)
- **New:** Arduino IDE (full Apple Silicon support)
- **Benefit:** Native ARM64 performance, easier setup

#### Code Structure
- **Old:** Multiple header files (Config.h, WiFiManager.h, etc.)
- **New:** Single .ino file with all code included
- **Benefit:** Arduino IDE compatibility, easier deployment

#### Pin Definitions
```cpp
// ESP32-S3 (old) - connects to Controllino Serial2
#define SERIAL_RX_PIN 16  // Serial2 RX → Controllino pin 17
#define SERIAL_TX_PIN 17  // Serial2 TX → Controllino pin 16

// ESP32-C6 (new) - connects to Controllino main Serial
#define SERIAL_RX_PIN 5   // Serial1 RX → Controllino pin 1 (TX)
#define SERIAL_TX_PIN 4   // Serial1 TX → Controllino pin 0 (RX)
```

#### Display Configuration
```cpp
// ESP32-S3 (old) - TTGO T-Display built-in
#define TFT_WIDTH  240
#define TFT_HEIGHT 135
#define ROTATION   1      // Landscape

// ESP32-C6 (new) - 1.47" module
#define TFT_WIDTH  240  
#define TFT_HEIGHT 240
#define ROTATION   0      // Portrait
```

### Feature Improvements

#### Enhanced Web Interface
- Added device identification (ESP32-C6 with 1.47" LCD)
- Improved responsive design for mobile devices
- Better error handling and status reporting

#### Better Display Layout
- Optimized for square 240x240 format
- More information density
- Improved text wrapping for long responses
- Better status visualization

#### Simplified Installation
- Single file deployment
- No complex build configuration
- Library dependencies clearly documented
- Step-by-step setup instructions

### Performance Considerations

#### Memory Usage
- **ESP32-S3:** 512KB SRAM, 384KB ROM
- **ESP32-C6:** 512KB SRAM, 320KB ROM  
- **Impact:** Slightly less ROM, but sufficient for application

#### Processing Power
- **ESP32-S3:** Dual-core Xtensa LX7 @ 240MHz
- **ESP32-C6:** Single-core RISC-V @ 160MHz
- **Impact:** Lower performance, but adequate for this application

#### Power Consumption
- **ESP32-C6:** Generally lower power consumption
- **Benefit:** Better for battery-powered applications

### Compatibility Matrix

| Feature | ESP32-S3 | ESP32-C6 | Notes |
|---------|----------|----------|-------|
| WiFi | 802.11 b/g/n | 802.11 ax (WiFi 6) | C6 has newer standard |
| Bluetooth | 5.0 | 5.0 LE | Similar capability |
| Zigbee | No | Yes | C6 adds IoT protocol |
| PlatformIO | ✅ | ⚠️ | Limited ARM64 support |
| Arduino IDE | ✅ | ✅ | Full support both |
| Serial UARTs | 3 (0,1,2) | 2 (0,1) | C6 has fewer UARTs |
| GPIO Count | 45 | 22 | C6 has fewer pins |

## Migration Benefits

### For Apple Silicon Mac Users
1. **Native performance:** Arduino IDE runs natively on ARM64
2. **No virtualization:** Direct hardware compilation support
3. **Better integration:** Native macOS toolchain

### For Development
1. **Simpler deployment:** Single file instead of project structure
2. **Easier debugging:** All code in one place
3. **Faster iteration:** No complex build system

### For Hardware
1. **Modern connectivity:** WiFi 6 support for future networks
2. **Lower power:** Better battery life potential
3. **More standards:** Zigbee support for IoT expansion

## Backward Compatibility

### Controllino Interface
- **Identical:** Same serial protocol and commands
- **Compatible:** Works with existing Controllino firmware
- **Seamless:** Drop-in replacement for ESP32-S3

### Web Interface
- **Enhanced:** Same functionality with improvements
- **Compatible:** Same API endpoints and JSON format
- **Upgraded:** Better mobile experience

## Recommendations

### When to Use ESP32-C6 Version
- ✅ Apple Silicon Mac development
- ✅ Prefer Arduino IDE over PlatformIO
- ✅ Want latest WiFi standards
- ✅ Need simpler deployment
- ✅ Value lower power consumption

### When to Keep ESP32-S3 Version
- ✅ Already working PlatformIO setup
- ✅ Need maximum processing power
- ✅ Require many GPIO pins
- ✅ Use Bluetooth extensively
- ✅ Existing hardware investment

## Future Considerations

### ESP32-C6 Advantages
- Newer silicon with ongoing support
- WiFi 6 future-proofing
- Growing ecosystem support
- Better power efficiency

### Migration Path
1. Test ESP32-C6 version alongside existing setup
2. Verify all functionality works
3. Gradually migrate production units
4. Keep ESP32-S3 as backup/alternative

Both versions maintain full feature parity for FairFan control! 🚀