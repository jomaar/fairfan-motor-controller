# ESP32 Web Interface (TTGO T-Display)

WiFi-enabled web interface for FairFan Motor Controller with built-in 1.14" IPS display.

## Hardware

- **TTGO T-Display**: ESP32 with integrated 1.14" IPS LCD (ST7789, 135x240)
- Built-in display shows connection status and motor state
- USB-C for programming and power

## Features

- **WiFi Access Point**: Direct phone connection (no router needed)
- **Web Interface**: Mobile-friendly control panel
- **Serial Bridge**: Transparent forwarding to Controllino
- **Real-time Status**: Live connection monitoring
- **Built-in Display**: Visual feedback on 1.14" IPS LCD

## Hardware Setup

### TTGO T-Display Specifications

- ESP32-WROOM-32 (Dual-core, WiFi + Bluetooth)
- 1.14" IPS LCD Display (ST7789, 135x240)
- USB-C connector
- Built-in buttons (2)
- Integrated display backlight control

### Connections to Controllino

```
ESP32              Controllino MAXI
─────────────────  ──────────────────
GPIO17 (TX2)   →   RX (Serial1)
GPIO16 (RX2)   ←   TX (Serial1)
GND            ─   GND
```

**Note**: TTGO T-Display uses GPIO17/16 for Serial2 communication with Controllino.

## WiFi Configuration

### Default Access Point Mode
- **SSID**: `FairFan-Control`
- **Password**: `fairfan2025`
- **IP Address**: `192.168.4.1`

To connect:
1. Connect phone/laptop to WiFi network "FairFan-Control"
2. Enter password "fairfan2025"
3. Open browser to `http://192.168.4.1`

### Station Mode (Optional)
Edit `include/Config.h` to connect to existing WiFi:

```cpp
constexpr const char* STA_SSID = "YourWiFiName";
constexpr const char* STA_PASSWORD = "YourPassword";
```

## Building and Uploading

### Prerequisites
- [PlatformIO](https://platformio.org/) installed
- ESP32 board drivers installed

### Build
```bash
cd esp32
platformio run
```

### Upload
```bash
platformio run --target upload
```

### Monitor Serial
```bash
platformio device monitor --baud 115200
```

## Web Interface

The web interface provides:

### Motor 1 Control
- Set rotation degrees (0-1080°)
- Go / Stop buttons

### Motor 2 Control
- Home (find limit switches)
- Stop button

### Sequence Control
- Start sequence
- Stop immediately
- Soft stop (finish current movement)
- Emergency stop all

### Direction Mode
- Same direction (synchronized)
- Opposite direction (alternating)

### Real-time Feedback
- Connection status indicator
- Live response log from Controllino
- Command history

## API Endpoints

### GET /
Main web interface (HTML)

### POST /api/command
Send command to Controllino

**Request:**
```json
{
  "command": "go1"
}
```

**Response:**
```json
{
  "success": true,
  "command": "go1"
}
```

### GET /api/status
Get system status

**Response:**
```json
{
  "connected": true,
  "lastResponse": "Motor 1: Started 180°",
  "lastResponseTime": 12345,
  "uptime": 67890
}
```

## Configuration

All settings in `include/Config.h`:

### WiFi
- AP/Station credentials
- IP configuration
- Connection timeout

### Serial
- RX/TX pins (GPIO17/16 for TTGO T-Display)
- Baud rate (must match Controllino: 115200)
- Timeout

### Display
- TFT_eSPI library configuration
- ST7789 driver settings (135x240)
- Backlight control (GPIO4)

## Display Layout

The built-in 1.14" IPS display shows:

```
┌─────────────────────┐
│ FairFan Control     │
├─────────────────────┤
│ AP: FairFan-Control │
│ IP: 192.168.4.1     │
├─────────────────────┤
│ Controllino: OK     │
│ Motor 1: Started... │
└─────────────────────┘
```

## Troubleshooting

### Cannot connect to WiFi AP
- Check SSID/password
- Ensure phone WiFi is enabled
- Try forgetting network and reconnecting

### No response from Controllino
- Verify TX/RX connections (TX→RX, RX→TX)
- Check GND connection
- Verify baud rate matches (115200)
- Check Controllino is powered and running

### Display not working
- Verify TFT_eSPI User_Setup.h configuration
- Check ST7789 driver settings
- Ensure display dimensions set to 135x240
- Verify backlight GPIO4 is configured

### Upload fails
- Check USB-C cable connection
- Verify correct COM port selected
- Hold BOOT button during upload if needed

## Memory Usage

- **Flash**: ~600KB (depends on libraries)
- **RAM**: ~50KB (WebServer + buffers)

## Security Note

Default AP password is weak. For production use:
- Change password in `Config.h`
- Use WPA2 encryption
- Consider implementing authentication

## License

MIT License - see main project LICENSE file

## Authors

jomaar
