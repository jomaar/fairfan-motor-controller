# FairFan ESP32-C6 PlatformIO Project

## 🚀 Native Apple Silicon Development Environment

This project uses **PlatformIO** with **native Apple Silicon support** for ESP32-C6 development - no more Arduino IDE workarounds!

### ✅ Confirmed Working Configuration

- **Board**: ESP32-C6-DevKitC-1
- **Display**: 1.47" ST7789 LCD (172x320, Portrait Mode)
- **Toolchain**: Native Apple Silicon compilation
- **Framework**: Arduino with GFX Library

### 📌 Pin Configuration (Tested & Working)

```
SCK:  GPIO7   (SPI Clock)
MOSI: GPIO6   (SPI Data)
CS:   GPIO14  (Chip Select)
DC:   GPIO15  (Data/Command)
RST:  GPIO21  (Reset)
BL:   GPIO22  (Backlight)
```

### 🛠️ Setup Instructions

1. **Install PlatformIO** (already done):
   ```bash
   # PlatformIO IDE extension should be installed in VSCode
   ```

2. **Open this project**:
   ```bash
   cd fairfan-esp32c6-platformio
   # Open in VSCode with PlatformIO extension
   ```

3. **Update WiFi credentials** in `src/main.cpp`:
   ```cpp
   const char* ssid = "your_wifi_network";
   const char* password = "your_wifi_password";
   ```

4. **Build the project**:
   - Use PlatformIO: `PlatformIO: Build` (Cmd+Shift+P)
   - Or use terminal: `pio run`

5. **Upload to ESP32-C6**:
   - Use PlatformIO: `PlatformIO: Upload` (Cmd+Shift+P)
   - Or use terminal: `pio run --target upload`

### 🎯 Key Advantages over Arduino IDE

- **Native Apple Silicon compilation** - no Rosetta 2 needed
- **Advanced IntelliSense** with proper autocomplete
- **Built-in debugging support** with ESP32-C6
- **Library management** with automatic dependency resolution
- **Better build system** with caching and parallel builds
- **Integrated serial monitor** with filtering
- **No emoji compilation issues** - proper Unicode support

### 📦 Dependencies (Auto-installed)

- `moononournation/GFX Library for Arduino@^1.4.7`
- `bblanchon/ArduinoJson@^7.0.4`

### 🌐 Web Interface Features

- Real-time fan control with emoji icons
- Speed control slider
- Mode switching (manual/auto)
- Oscillation toggle
- System status monitoring
- Responsive design with modern gradient UI

### 🔧 Development Commands

```bash
# Build project
pio run

# Upload to device
pio run --target upload

# Open serial monitor
pio device monitor

# Clean build files
pio run --target clean

# Update libraries
pio pkg update
```

### 📱 Display Features

- **Portrait mode**: 172x320 pixels
- **Real-time updates** every 2 seconds
- **Status indicators**: WiFi, fan state, speed bar
- **Color-coded information**: Green=good, Red=error, Yellow=info
- **Platform identification**: Shows "PlatformIO" and "Apple Silicon Native"

### 🔍 Debugging

PlatformIO includes built-in debugging support:

```bash
# Start debug session
pio debug
```

### 📊 Monitoring

```bash
# Monitor with ESP32 exception decoder
pio device monitor --filter esp32_exception_decoder
```

### 🎉 Success Indicators

When everything is working, you should see:
- Display showing colors and text
- Web server accessible at ESP32's IP address
- Serial output confirming initialization
- Native compilation without Rosetta 2 warnings

---

## 🆚 Arduino IDE vs PlatformIO Comparison

| Feature | Arduino IDE | PlatformIO |
|---------|-------------|------------|
| Apple Silicon Support | Limited (Rosetta 2) | **Native** |
| IntelliSense | Basic | **Advanced** |
| Debugging | None | **Built-in** |
| Library Management | Manual | **Automatic** |
| Build Speed | Slow | **Fast** |
| Unicode/Emoji Support | Problematic | **Full Support** |
| Project Structure | Single file | **Professional** |

**Recommendation**: Use PlatformIO for serious ESP32-C6 development!