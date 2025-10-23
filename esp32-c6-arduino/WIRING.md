# ESP32-C6 to Controllino Wiring Guide

## Connection Overview

```
┌─────────────────┐        ┌──────────────────────┐
│   ESP32-C6      │        │  Controllino MAXI    │
│                 │        │                      │
│ GPIO4 (UART1TX) │────────│ Pin 0 (Serial RX)   │
│ GPIO5 (UART1RX) │────────│ Pin 1 (Serial TX)   │
│ GND             │────────│ GND                  │
│                 │        │                      │
│ [1.47" LCD]     │        │                      │
│ (built-in)      │        │                      │
└─────────────────┘        └──────────────────────┘
```

## Pin Details

### ESP32-C6 Side
- **GPIO4:** UART1 TX (transmit to Controllino)
- **GPIO5:** UART1 RX (receive from Controllino)  
- **GND:** Ground reference

### Controllino MAXI Side
- **Pin 0:** Serial RX (receive from ESP32-C6)
- **Pin 1:** Serial TX (transmit to ESP32-C6)
- **GND:** Ground (any GND pin)

## Critical Notes

### ⚠️ Serial Port Usage
- **Controllino uses Serial (USB), pins 0/1 - the main programming port**
- **WARNING: Cannot use Serial Monitor while ESP32 is connected**
- **Disconnect ESP32 from pins 0/1 when programming Controllino**

### ⚠️ Wire Crossover  
- ESP32 TX → Controllino RX
- ESP32 RX → Controllino TX
- TX always connects to RX (transmit to receive)

### ⚠️ Ground Connection
- **Essential:** Connect GND between both boards
- Without common ground, serial communication will fail

## Physical Connection

### Option 1: Dupont Wires
```
ESP32-C6 Pin    Wire Color    Controllino Pin
GPIO4           Red           Pin 0 (RX)
GPIO5           Yellow        Pin 1 (TX)  
GND             Black         GND
```

### Option 2: Custom Cable
- 3-wire cable with appropriate connectors
- Keep wires short (< 30cm) for reliable communication
- Use twisted pair if possible for TX/RX signals

## Verification Steps

### 1. Physical Check
- [ ] GPIO4 connected to Controllino pin 0 (RX)
- [ ] GPIO5 connected to Controllino pin 1 (TX)
- [ ] GND connected between boards
- [ ] No loose connections

### 2. Software Check
- [ ] Controllino code uses Serial (pins 0/1)
- [ ] ESP32-C6 code uses UART1 with correct pins
- [ ] Both devices set to 115200 baud

### 3. Communication Test
- [ ] Send "mode" command from ESP32
- [ ] Check for response in serial monitor
- [ ] Verify LCD shows connection status

## Troubleshooting

### No Response from Controllino
1. **Check wiring:** TX→RX crossover, GND connected
2. **Verify Serial:** Controllino must use Serial (pins 0/1)
3. **Baud rate:** Both devices must use 115200
4. **Power:** Ensure both devices are powered properly
5. **⚠️ Programming conflict:** Disconnect ESP32 when programming Controllino

### Garbled/Partial Response  
1. **Ground connection:** Most common cause
2. **Wire length:** Keep connections short
3. **Interference:** Separate from power wires
4. **Baud rate:** Verify 115200 on both ends

### ESP32-C6 PIN Conflicts
If your LCD uses different pins, you may need to change UART pins:
```cpp
// Alternative UART1 pins for ESP32-C6
const uint8_t SERIAL_RX_PIN = 20;  // Different RX pin
const uint8_t SERIAL_TX_PIN = 21;  // Different TX pin
```

Check your board documentation for available pins.

## Alternative Configurations

### ESP32-C6 Without LCD
If using plain ESP32-C6 without LCD:
- More GPIO pins available
- Can use standard pins like 16/17
- Update pin definitions in code accordingly

### Different ESP32-C6 Boards
Pin assignments may vary between manufacturers:
- Check your board's pinout diagram
- Update `SERIAL_RX_PIN` and `SERIAL_TX_PIN` in code
- Test with simple serial echo first

## Testing Procedure

### 1. Basic Connectivity
```
ESP32-C6 Serial Monitor:
[Bridge] Serial connection to Controllino initialized
[Bridge] RX: GPIO5, TX: GPIO4
[Bridge] Baud Rate: 115200
```

### 2. Send Test Command
- Open web interface
- Send "mode" command
- Should see response within 1-2 seconds

### 3. LCD Status
- LCD should show "CONNECTED" status
- Last response should appear on display
- Green status indicator in web interface

### 4. Continuous Operation
- Test multiple commands
- Verify consistent responses
- Check for timeouts or disconnections

Ready for reliable ESP32-C6 ↔ Controllino communication! 🔌