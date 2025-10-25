# FairFan WiFi Bridge - Final Wiring Guide

## ✅ SIMPLIFIED SOLUTION: Use Controllino USB Serial Pins!

### Why This Works Better:

The Controllino MAXI's USB serial port (Serial0) uses **pins 0 and 1**, which are:
- ✅ **Accessible on X1 screw terminals**
- ✅ **Already used by your existing firmware** (no code changes needed!)
- ✅ **FTDI chip handles voltage conversion** (no voltage divider needed!)
- ✅ **Higher baud rate possible** (115200 instead of 9600)

When the USB cable is **NOT connected**, pins 0/1 are free for the ESP32!

---

## Final Wiring

⚠️ **WARNING: Disconnect Controllino USB cable before connecting ESP32 wires!**

```
ESP32 TTGO T-Display       Controllino MAXI X1
────────────────────       ────────────────────
GPIO 21 (TX)          →    Pin 0 (RX0)    [Direct Connection - SAFE]
GPIO 22 (RX)          ←    Pin 1 (TX0)    [Direct Connection - SAFE]
GND                   →    GND

Optional:
5V                    →    5V (if powering ESP32 from Controllino)
```

### NO Voltage Divider Needed! 🎉

The FTDI USB-to-Serial chip on the Controllino already handles voltage level conversion between the ATmega2560 (5V) and external devices. The serial lines on pins 0/1 are safe for 3.3V devices like the ESP32.

---

## Pin Locations

### Controllino MAXI X1 Terminal Block:
```
Pin 0 (RX0)  - Digital I/O screw terminal
Pin 1 (TX0)  - Digital I/O screw terminal
GND          - Ground screw terminal
```

### ESP32 TTGO T-Display:
```
GPIO 21 - Labeled on board
GPIO 22 - Labeled on board
GND     - Ground pin
```

---

## Configuration Summary

### ESP32 Settings (UPDATED):
- **Serial Port**: Serial2 (GPIO 21/22)
- **Baud Rate**: 115200 (matches Controllino)
- **Config File**: `esp32/include/Config.h`

### Controllino Settings (UNCHANGED):
- **Serial Port**: Serial (pins 0/1)
- **Baud Rate**: 115200
- **Config File**: `include/Config.h`

No firmware changes needed for Controllino! ✅

---

## How It Works

```
┌─────────────┐                    ┌──────────────┐                  ┌─────────┐
│   Phone/    │                    │  ESP32 TTGO  │                  │Controllino│
│   Tablet    │  WiFi Connection   │  T-Display   │  Serial Bridge   │  MAXI   │
│             │◄──────────────────►│              │◄────────────────►│         │
│             │   192.168.4.1      │ GPIO 21/22   │   Pins 0/1 (X1)  │         │
│ Web Browser │                    │ Serial2      │   Serial0        │ Motors  │
└─────────────┘                    │ @115200      │   @115200        └─────────┘
                                   └──────────────┘
```

**Flow:**
1. Phone connects to `FairFan-Control` WiFi (password: `fairfan2025`)
2. Opens `http://192.168.4.1` in browser
3. Clicks "M1START" button
4. ESP32 receives command via WebSocket
5. ESP32 sends `M1START\n` to Serial2 (GPIO 21)
6. Controllino receives on Serial0 (Pin 0)
7. Controllino starts Motor 1
8. Controllino sends response on Serial0 (Pin 1)
9. ESP32 receives on Serial2 (GPIO 22)
10. ESP32 forwards response to web browser
11. Phone displays "Motor 1: Started 180°"

---

## Upload Instructions

### 1. Upload ESP32 Firmware:
```bash
cd /Users/ja/Library/CloudStorage/OneDrive-Personal/Projects/FairFan/fairfanpio01/esp32
~/.platformio/penv/bin/platformio run -t upload
```

### 2. No Upload Needed for Controllino! 
Your existing firmware already uses Serial0 at 115200 baud. Just wire it up!

---

## Testing Procedure

### Step 1: Power Up
1. Connect ESP32 to USB (for power and monitoring)
2. Connect Controllino to USB (for power) OR use external 24V supply
3. Verify ESP32 boots (display shows color test)
4. Verify Controllino boots (motors initialize)

### Step 2: Connect the Bridge
1. **DISCONNECT both USB cables!** (Important - prevents ground loops)
2. Wire GPIO21 → Pin 0 (X1)
3. Wire GPIO22 → Pin 1 (X1) 
4. Wire GND → GND
5. Reconnect ESP32 USB (for power)
6. Reconnect Controllino power (USB or 24V, but NOT USB if using for programming)

### Step 3: Test WiFi
1. On phone/tablet, connect to WiFi: `FairFan-Control`
2. Password: `fairfan2025`
3. Open browser to: `http://192.168.4.1`
4. Should see motor control interface

### Step 4: Test Commands
1. Click "M1START" button
2. Motor 1 should start moving
3. Web interface should show response: "Motor 1: Started 180°"
4. Try other commands: M1STOP, SQ1, etc.

### Step 5: Monitor Serial (Optional)
Connect ESP32 Serial Monitor:
```bash
cd esp32
~/.platformio/penv/bin/platformio device monitor --baud 115200
```

You should see:
```
[Web] Command received: M1START
[Bridge] Sent to Controllino: M1START
[Bridge] Response from Controllino: Motor 1: Started 180°
[Web] Sent response to client
```

---

## Troubleshooting

### Problem: No WiFi AP visible
- Check ESP32 is powered
- Check serial output for errors
- Try power cycling ESP32

### Problem: Web page won't load
- Verify connected to correct WiFi: `FairFan-Control`
- Try `http://192.168.4.1` (not https)
- Check ESP32 display shows IP address

### Problem: Commands don't work
- Verify wiring: GPIO21→Pin0, GPIO22→Pin1, GND→GND
- Check both devices share common ground
- Monitor ESP32 serial output for errors
- Verify Controllino is powered and initialized

### Problem: Responses not showing
- Check GPIO22 (RX) wire connection
- Verify baud rates match (both 115200)
- Check serial monitor on ESP32 for incoming data

### Problem: Ground loop / unstable behavior
- **Disconnect USB from Controllino** if also connected to ESP32
- Use only ONE USB connection (ESP32) + Controllino external power
- OR use WiFi power bank for ESP32 (no USB)

---

## Important Notes

⚠️ **CRITICAL: Never connect both ESP32 wires AND Controllino USB simultaneously!**

**Why this is dangerous:**
1. **Signal Conflict**: FTDI chip and ESP32 both try to drive the same pins (0/1)
2. **Ground Loop**: Creates circular ground path through two USB connections
3. **Communication Corruption**: Neither serial channel works properly
4. **Potential Damage**: Signal contention can stress or damage chips

**Safe Operating Modes:**

| ESP32 USB | Controllino USB | Wires | Status |
|-----------|-----------------|-------|--------|
| ✅ On | ❌ Off | ✅ Connected | ✅ **SAFE** - Production mode |
| ❌ Off | ✅ On | ❌ Disconnected | ✅ **SAFE** - Debug mode |
| ✅ On | ✅ On | ❌ Disconnected | ✅ **SAFE** - Independent |
| ✅ On | ✅ On | ✅ Connected | ❌ **DANGER** - Don't do this! |

**Workflow:**

**For Normal Operation (ESP32 Bridge):**
1. Disconnect Controllino USB cable
2. Connect ESP32 wires to Controllino pins 0/1
3. Power Controllino via 24V external supply
4. ESP32 USB can stay connected (for monitoring/power)

**For Controllino Programming/Debugging:**
1. Disconnect ESP32 wires from Controllino pins 0/1
2. Connect Controllino USB cable
3. Upload firmware / use Serial Monitor
4. When done: Disconnect USB, reconnect ESP32 wires

⚠️ **When to disconnect USB:**
- If using ESP32 USB AND Controllino USB simultaneously → potential ground loop
- Solution: Power Controllino via 24V external supply, or power ESP32 via battery

⚠️ **Serial0 vs USB conflict:**
- When ESP32 is connected to Pin 0/1, you CANNOT use USB serial monitor on Controllino
- They share the same pins!
- For debugging Controllino: Disconnect ESP32 wires, connect USB, upload/monitor
- For normal operation: ESP32 connected, Controllino USB disconnected (or only for power)

✅ **Safe operation mode:**
- ESP32: Powered via USB (or battery)
- Controllino: Powered via 24V supply
- Both connected via GPIO21/22 ↔ Pin0/1 + GND
- Only ESP32 USB connected for monitoring

---

## Advanced: Power Options

### Option A: Both USB (Simple, but potential issues)
```
ESP32: USB → Computer
Controllino: USB → Computer
Wires: GPIO21↔Pin0, GPIO22↔Pin1, GND↔GND
Risk: Ground loops, serial conflicts
```

### Option B: ESP32 USB + Controllino 24V (RECOMMENDED)
```
ESP32: USB → Computer  
Controllino: 24V external supply
Wires: GPIO21↔Pin0, GPIO22↔Pin1, GND↔GND
Benefits: Clean, no conflicts, stable
```

### Option C: ESP32 Battery + Controllino 24V (PRODUCTION)
```
ESP32: LiPo battery / USB power bank
Controllino: 24V external supply
Wires: GPIO21↔Pin0, GPIO22↔Pin1, GND↔GND
Benefits: Fully wireless ESP32, portable
```

---

## Next Steps

1. ✅ Wire the connections
2. ✅ Upload ESP32 firmware (if not already done)
3. ✅ Test web interface
4. ✅ Enjoy wireless motor control! 🎉

## Support

If issues persist, check:
- Serial monitor output on ESP32
- Display status on TTGO screen
- Controllino behavior via separate USB connection (with ESP32 wires disconnected)
