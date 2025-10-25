# Quick Start Guide - ESP32 Web Interface

## 1️⃣ Hardware Connection

Connect ESP32 to Controllino:

```
ESP32 → Controllino
──────────────────────────
TX2 (17) → Serial2 RX
RX2 (16) → Serial2 TX
GND → GND

Add to Controllino:
Serial2.begin(115200);
```

## 2️⃣ Upload Firmware

```bash
cd esp32
platformio run --target upload
```

## 3️⃣ Connect to WiFi

On your phone:
1. Open WiFi settings
2. Connect to **"FairFan-Control"**
3. Password: **fairfan2025**

## 4️⃣ Open Web Interface

Open browser to: **http://192.168.4.1**

## 5️⃣ Control Your Motors!

- Set degrees and tap **Go**
- Use **Home** to calibrate Motor 2
- Start **Sequence** for oscillation
- Monitor live feedback

---

That's it! You're controlling FairFan from your phone! 📱
