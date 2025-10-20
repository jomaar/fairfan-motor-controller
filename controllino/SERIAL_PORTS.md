# Controllino Serial Port Reference

## Serial Ports on Controllino MAXI Automation (ATmega2560)

The Controllino MAXI Automation is based on the ATmega2560 microcontroller, which has **4 hardware serial ports**:

### Serial Port Overview

| Port | Arduino Pins | ATmega2560 Pins | Usage |
|------|--------------|-----------------|-------|
| **Serial** (Serial0) | TX0=1, RX0=0 | PE1, PE0 | **USB/PC Communication** - Used for programming |
| **Serial1** | TX1=18, RX1=19 | PD3, PD2 | Available for RS485 or custom use |
| **Serial2** | TX2=16, RX2=17 | PH1, PH0 | **Recommended for ESP32** ⭐ |
| **Serial3** | TX3=14, RX3=15 | PJ1, PJ0 | Available for custom use |

## Recommended Setup: Use Serial2

For the ESP32 web interface, **we recommend Serial2**:

### Why Serial2?
- ✅ **Not used by USB** (Serial0 is reserved)
- ✅ **Standard pins** (Arduino pins 16/17)
- ✅ **Widely available** on most ATmega2560 boards
- ✅ **Well documented** in Arduino ecosystem

### Controllino Code Setup

Add to your Controllino firmware setup:

```cpp
void setup() {
    Serial.begin(115200);   // USB/PC (for debugging)
    Serial2.begin(115200);  // ESP32 communication
    
    // Your existing setup code...
}
```

### Finding Serial2 on Controllino MAXI

**Important:** The Controllino MAXI may not have Arduino pins 16/17 directly labeled on screw terminals. You have several options:

#### Option 1: Use ICSP Header (Most Likely)
Check if your Controllino has an ICSP or expansion header with access to these pins.

#### Option 2: Check Screw Terminals
Some Controllino variants expose serial ports on labeled terminals (e.g., "TX2", "RX2", or "SER2").

#### Option 3: Use Serial1 or Serial3 Instead
If Serial2 pins are not accessible, you can use Serial1 or Serial3:

**For Serial1 (pins 18/19):**
```cpp
// Controllino:
Serial1.begin(115200);

// ESP32 Config.h - change to:
// (No changes needed - ESP32 doesn't care which Controllino serial port)
```

**For Serial3 (pins 14/15):**
```cpp
// Controllino:
Serial3.begin(115200);
```

## Physical Pin Location

**Check your Controllino documentation** for exact screw terminal locations. Look for:
- Labels like "TX2", "RX2", "SER2", or "SERIAL2"
- Expansion headers (may be unpopulated pin headers)
- Technical drawing in the datasheet

## Wiring Table (Generic ATmega2560)

If using standard Arduino pin numbers:

| Controllino | Arduino Pin | Signal | ESP32 |
|-------------|-------------|--------|-------|
| Digital 16 or TX2 | 16 | TX2 | GPIO16 (RX2) |
| Digital 17 or RX2 | 17 | RX2 | GPIO17 (TX2) |
| GND | - | GND | GND |

**Remember:** TX → RX crossover!

## Code Example: Any Serial Port

The ESP32 doesn't need to know which Controllino serial port you use. Just ensure both sides use the same baud rate (115200).

### Controllino Firmware (Example: Using Serial2)

```cpp
void setup() {
    Serial.begin(115200);    // USB debugging
    Serial2.begin(115200);   // ESP32 communication
}

void loop() {
    // Read from ESP32
    if (Serial2.available()) {
        String command = Serial2.readStringUntil('\n');
        processCommand(command);
    }
    
    // Send response to ESP32
    Serial2.println("Motor 1: Started 180°");
}
```

### Alternative: Serial1

```cpp
void setup() {
    Serial.begin(115200);    // USB debugging
    Serial1.begin(115200);   // ESP32 communication
}

void loop() {
    if (Serial1.available()) {
        String command = Serial1.readStringUntil('\n');
        processCommand(command);
    }
    Serial1.println("Motor 1: Started 180°");
}
```

## Testing Without ESP32

Before connecting the ESP32, test your chosen serial port with a USB-to-Serial adapter:

1. Upload Controllino firmware with Serial2 (or Serial1/Serial3)
2. Connect USB-Serial adapter to TX2/RX2 pins
3. Open serial monitor on that COM port (115200 baud)
4. Send test commands

This confirms the serial port is working before adding the ESP32.

## Important Notes

⚠️ **DO NOT use Serial (Serial0)** - This is reserved for USB communication and programming!

✅ **Recommended order:** Try Serial2 first, then Serial1, then Serial3

🔍 **Consult your Controllino datasheet** for exact pin locations on your specific model

## Common Issues

### "Can't find Serial2 pins on screw terminals"
→ Check for expansion headers or use Serial1 instead

### "Serial2.println() not working"
→ Ensure you called `Serial2.begin(115200)` in setup()

### "Getting garbage characters"
→ Baud rates don't match - both must be 115200

### "Commands not received"
→ Check TX/RX crossover (TX → RX, RX → TX)

## References

- [ATmega2560 Datasheet](https://ww1.microchip.com/downloads/en/devicedoc/atmel-2549-8-bit-avr-microcontroller-atmega640-1280-1281-2560-2561_datasheet.pdf)
- [Arduino Mega 2560 Pinout](https://docs.arduino.cc/resources/pinouts/A000067-full-pinout.pdf)
- [Controllino Resources](https://www.controllino.com/resources/)

---

**Summary:** Use **Serial2** (pins 16/17) for ESP32 communication. If not accessible, use Serial1 (pins 18/19) or Serial3 (pins 14/15) instead.
