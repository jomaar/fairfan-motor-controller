# Hardware Wiring Guide

## System Overview

```
┌─────────────┐         ┌──────────────────┐
│   ESP32     │ Serial  │   Controllino    │
│  Web Bridge │◄───────►│   MAXI Auto      │
│             │         │  Motor Control   │
└─────────────┘         └──────────────────┘
      │                          │
      │                          │
   WiFi                    Stepper Motors
      │                          │
      ▼                          ▼
  📱 Phone              Motor1 + Motor2
```

## Connections

### ESP32 ↔ Controllino Serial

| ESP32 Pin | Signal | Controllino Pin |
|-----------|--------|-----------------|
| GPIO 17   | TX2    | Serial1 RX      |
| GPIO 16   | RX2    | Serial1 TX      |
| GND       | Ground | GND             |

**Important:** 
- TX connects to RX (crossover)
- Both devices must share common ground
- Serial baud rate: 115200 (configured in both devices)

### Optional: OLED Display (ESP32)

| ESP32 Pin | Signal | Display Pin |
|-----------|--------|-------------|
| GPIO 21   | SDA    | SDA         |
| GPIO 22   | SCL    | SCL         |
| 3.3V      | Power  | VCC         |
| GND       | Ground | GND         |

**Display Type:** SSD1306 128x64 OLED (I2C)

## Power Requirements

### ESP32
- **Power**: USB cable (5V) or external 5V supply
- **Current**: ~250mA (WiFi active)
- **I/O**: 3.3V logic levels

### Controllino MAXI
- **Power**: 12-24V DC via terminal block
- **Current**: Depends on motor load
- **I/O**: 5V/24V logic levels (Serial is 5V tolerant)

**Note:** ESP32 uses 3.3V logic but Controllino serial pins are 5V tolerant. Direct connection is safe.

## Connection Checklist

- [ ] ESP32 TX2 (GPIO17) → Controllino Serial1 RX
- [ ] ESP32 RX2 (GPIO16) → Controllino Serial1 TX
- [ ] Common GND between ESP32 and Controllino
- [ ] ESP32 powered via USB
- [ ] Controllino powered via DC input
- [ ] (Optional) OLED display connected to ESP32 I2C
- [ ] Verify no shorts or crossed wires

## Testing Connection

1. **Upload Controllino firmware** first
2. **Upload ESP32 firmware**
3. **Open ESP32 serial monitor** (115200 baud)
4. Look for: `[Bridge] Serial connection to Controllino initialized`
5. **Connect phone** to "FairFan-Control" WiFi
6. **Open web interface** at http://192.168.4.1
7. **Send test command** (e.g., "mode")
8. Watch for response in serial monitor and web interface

## Troubleshooting

### No serial communication
- ✅ Check TX/RX crossover (TX→RX, RX→TX)
- ✅ Verify GND connection
- ✅ Confirm baud rate matches (115200)
- ✅ Check both devices are powered

### Display not working
- ✅ Verify I2C address (0x3C or 0x3D)
- ✅ Check SDA/SCL connections
- ✅ Ensure 3.3V power (not 5V for most OLEDs)

### Can't connect to WiFi
- ✅ SSID: "FairFan-Control"
- ✅ Password: "fairfan2025"
- ✅ Check ESP32 has power and booted
- ✅ Look for WiFi network in phone settings

## Safety Notes

⚠️ **Important Safety Considerations:**

1. **Separate Power Supplies**: ESP32 and Controllino use separate power sources
2. **No Voltage Sharing**: Only signal (TX/RX) and GND are connected
3. **Motor Power**: Motors powered from Controllino, not ESP32
4. **Isolation**: ESP32 does not control motors directly
5. **Emergency Stop**: Physical e-stop still recommended for safety

## Bill of Materials

| Component | Qty | Est. Cost |
|-----------|-----|-----------|
| ESP32 Dev Board (ESP32-WROOM-32) | 1 | $8-12 |
| SSD1306 OLED Display 128x64 (optional) | 1 | $5-8 |
| USB Cable (Micro or Type-C) | 1 | $3-5 |
| Jumper Wires (M-M or M-F) | 3-6 | $2-3 |
| **Total** | | **$13-20** |

## Recommended Products

### ESP32 Board
- Look for: ESP32-WROOM-32 with USB interface
- Must have: GPIO 16, 17, 21, 22 broken out
- Examples: ESP32 DevKit V1, NodeMCU-32S

### OLED Display
- Model: SSD1306 128x64
- Interface: I2C (4-pin)
- Voltage: 3.3V compatible
- Address: 0x3C (default)

## Advanced: Controllino Serial Port

The Controllino MAXI has **Serial1** available on specific pins. Check your Controllino documentation for the exact pin locations of Serial1 TX and RX.

**Typical mapping:**
- Serial1 can be accessed via specific screw terminals
- May require enabling Serial1 in code with `Serial1.begin(115200)`

Consult the [Controllino MAXI documentation](https://www.controllino.com/controllino-maxi/) for your specific model.
