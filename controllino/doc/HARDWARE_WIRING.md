# Hardware Verdrahtung - FairFan Motor Controller

## Übersicht

Dieses Dokument beschreibt die komplette Verdrahtung des FairFan Motor Controllers mit Controllino MAXI Automation.

---

## Controllino MAXI Automation Pinout

### Motor 1 (Main Motor - Stepper)

| Funktion | Controllino Pin | Pin Typ | Anschluss |
|----------|----------------|---------|-----------|
| STEP | Digital Pin 6 | Output | Stepper Driver STEP |
| DIR | Digital Pin 4 | Output | Stepper Driver DIR |
| (Enable) | - | - | Optional über Driver |

**Konfiguration in Config.h:**
```cpp
Motor1::STEP_PIN = 6
Motor1::DIR_PIN = 4
```

**Stepper Driver Einstellungen:**
- Steps per Revolution: 200 (1.8° Motor)
- Microsteps: 8 (1/8)
- Gear Ratio: 20:1

---

### Motor 2 (Oscillation Motor - Stepper mit Endschaltern)

| Funktion | Controllino Pin | Pin Typ | Anschluss |
|----------|----------------|---------|-----------|
| STEP | Digital Pin 7 | Output | Stepper Driver STEP |
| DIR | Digital Pin 8 | Output | Stepper Driver DIR |
| ENABLE | Digital Pin 9 | Output | Stepper Driver EN (LOW=aktiv) |
| LEFT Switch | CONTROLLINO_DI1 | Input | Linker Endschalter (NC) |
| RIGHT Switch | CONTROLLINO_DI0 | Input | Rechter Endschalter (NC) |

**Konfiguration in Config.h:**
```cpp
Motor2::STEP_PIN = 7
Motor2::DIR_PIN = 8
Motor2::ENABLE_PIN = 9
Motor2::LEFT_SWITCH_PIN = CONTROLLINO_DI1
Motor2::RIGHT_SWITCH_PIN = CONTROLLINO_DI0
```

**Stepper Driver Einstellungen:**
- Steps per Revolution: 200 (1.8° Motor)
- Microsteps: 8 (1/8)
- Gear Ratio: 50:1

**Endschalter:**
- Typ: Normally Closed (NC)
- Signal: HIGH = nicht gedrückt, LOW = gedrückt
- Debounce: 5ms (Software)

---

### FRAM (Position Tracking)

| Funktion | Controllino Pin | FRAM Pin | Beschreibung |
|----------|----------------|----------|--------------|
| VCC | 5V | VCC | Spannungsversorgung |
| GND | GND | GND | Masse |
| SDA | Pin 20 (SDA) | SDA | I2C Daten |
| SCL | Pin 21 (SCL) | SCL | I2C Clock |
| WP | - | (NC) | Write Protect (nicht verbunden) |

**FRAM Modul:**
- Typ: MB85RC256V (32KB)
- I2C-Adresse: 0x50 (Standard)
- Spannung: 5V
- Modul: z.B. Adafruit #1895

**Konfiguration in Config.h:**
```cpp
FRAM::I2C_ADDRESS = 0x50
```

**Hinweise:**
- Pull-Up Widerstände: Meist auf FRAM-Modul vorhanden (10kΩ)
- Falls extern: 4.7kΩ - 10kΩ zwischen SDA/SCL und 5V
- WP-Pin: Auf GND für Write-Enable (oder nicht verbunden)

---

## Stromversorgung

### Controllino MAXI Automation

| Anschluss | Spannung | Verwendung |
|-----------|----------|------------|
| 24V IN | 12-24V DC | Hauptversorgung Controllino |
| 5V OUT | 5V (max 1A) | FRAM-Modul |
| GND | Common Ground | Alle Komponenten |

### Stepper Driver

| Anschluss | Spannung | Hinweis |
|-----------|----------|---------|
| VDD/Logic | 5V | Von Controllino 5V |
| VM/Motor | 12-24V | Je nach Motor |
| GND | Common Ground | Mit Controllino verbinden |

**Wichtig:** Common Ground zwischen Controllino und Stepper Drivers!

---

## Verkabelungsschema

```
Controllino MAXI Automation
├── 24V Power Supply
│   ├── 24V → Controllino 24V IN
│   └── GND → Controllino GND
│
├── Motor 1 Stepper Driver
│   ├── Pin 6 → STEP
│   ├── Pin 4 → DIR
│   ├── 5V → VDD (Logic)
│   ├── 24V → VM (Motor Power)
│   └── GND → GND (Common)
│
├── Motor 2 Stepper Driver
│   ├── Pin 7 → STEP
│   ├── Pin 8 → DIR
│   ├── Pin 9 → EN
│   ├── 5V → VDD (Logic)
│   ├── 24V → VM (Motor Power)
│   └── GND → GND (Common)
│
├── Motor 2 Limit Switches
│   ├── DI0 → Right Switch (NC)
│   ├── DI1 → Left Switch (NC)
│   └── GND → Switch Common
│
├── FRAM MB85RC256V
│   ├── 5V → VCC
│   ├── GND → GND
│   ├── Pin 20 (SDA) → SDA
│   └── Pin 21 (SCL) → SCL
│
└── USB/Serial
    └── Serial Monitor (115200 baud)
```

---

## I2C Bus

**Controllino MAXI Automation I2C Pins:**
- SDA: Pin 20 (auch als D20 bezeichnet)
- SCL: Pin 21 (auch als D21 bezeichnet)

**I2C-Geräte am Bus:**
| Gerät | Adresse | Beschreibung |
|-------|---------|--------------|
| FRAM MB85RC256V | 0x50 | Position Storage |

**Erweiterungsmöglichkeiten:**
- 0x51-0x57: Weitere FRAMs (via A0-A2 Pins)
- 0x68: RTC (DS3231)
- 0x76/0x77: BME280 Sensor
- usw.

---

## Mechanischer Aufbau

### Motor 1 (Main Motor)
- **Montage:** Basis/Gehäuse
- **Welle:** Trägt Motor 2
- **Kabel:** Motor 2 Kabel wickeln sich um Welle!
- **Rotation:** ±1080° max (3 Umdrehungen)
- **Markierung:** Am Gehäuse für Nullposition

### Motor 2 (Oscillation Motor)
- **Montage:** Auf Welle von Motor 1
- **Endschalter:** Links und Rechts
- **Kabel:** Wickeln sich um Motor 1 Welle
- **Homing:** Fährt zu Endschaltern

**⚠️ WICHTIG:**
Motor 2 Kabel sind das kritische Element! Daher:
- FRAM Position Tracking für Motor 1
- Auto-Recovery nach Stromausfall
- Strict Limits (±1080°)

---

## Kabelführung

### Empfohlene Kabeltypen

| Verbindung | Kabel | Länge | Hinweis |
|------------|-------|-------|---------|
| Stepper Driver → Motor | 4-adrig, geschirmt | So kurz wie möglich | Twisted pairs |
| Controllino → Driver (Step/Dir) | Twisted pair | <3m | Signal-Kabel |
| Limit Switches | 2-3 adrig | Flexibel | NC-Verkabelung |
| FRAM I2C | 4-adrig | <30cm | Kurz halten! |
| Power | Dimensioniert für Motorstrom | - | Separate Masse |

### Motor 2 Kabel (kritisch!)
- **Problem:** Wickeln sich um Motor 1 Welle
- **Lösung:** 
  - Flexible Litze
  - Zugentlastung
  - Reservelänge für 3+ Umdrehungen
  - FRAM Position Tracking!

---

## Test-Prozedur beim Anschluss

### 1. Basistest (ohne Motoren)
```
1. Controllino mit 24V versorgen
2. USB anschließen
3. Serial Monitor öffnen (115200 baud)
4. Bootmeldung prüfen
```

### 2. FRAM-Test
```
1. FRAM anschließen (5V, GND, SDA, SCL)
2. System neu starten
3. Serial: "FRAM MB85RC256V initialized"
4. Befehl: framinfo
5. Ausgabe: "Initialized: YES"
```

### 3. Motor 1 Test (ohne Last)
```
1. Driver anschließen, Motor verbinden
2. Befehl: m1cw90
3. Motor sollte sich 90° drehen
4. Befehl: gotohome1
5. Motor fährt zurück
```

### 4. Motor 2 Test
```
1. Driver anschließen, Motor verbinden
2. Endschalter anschließen
3. Befehl: home
4. Homing-Sequenz sollte laufen
5. Motor findet beide Endschalter
```

### 5. Vollständiger Test
```
1. Beide Motoren verbunden
2. Befehl: seq1
3. Oszillation sollte laufen
4. Stromausfall simulieren
5. Nach Neustart: Auto-Recovery prüfen
```

---

## Troubleshooting Hardware

### FRAM nicht erkannt
```
ERROR: FRAM not found at 0x50!
```
**Prüfen:**
- [ ] 5V und GND korrekt?
- [ ] SDA/SCL vertauscht?
- [ ] Pull-Up Widerstände vorhanden?
- [ ] I2C-Scan durchführen (Wire.scan())

### Motor dreht nicht
**Prüfen:**
- [ ] Step-Signale mit Oszilloskop?
- [ ] Enable-Pin richtig (LOW = aktiv)?
- [ ] Motorstrom richtig eingestellt?
- [ ] Verkabelung A+/A- und B+/B-?

### Endschalter triggern nicht
**Prüfen:**
- [ ] NC (Normally Closed) Verkabelung?
- [ ] Durchgang mit Multimeter messen
- [ ] Debounce-Zeit ausreichend? (5ms)
- [ ] Mechanische Auslösung funktioniert?

### Common Ground Probleme
**Symptome:**
- Motoren drehen sporadisch
- USB-Verbindung instabil
- Resets

**Lösung:**
- Alle GND zusammenführen (Star-Ground)
- Netzteil ausreichend dimensioniert
- Entstörkondensatoren an Motoren

---

## Sicherheitshinweise

⚠️ **Spannungen:**
- 24V DC kann bei Kurzschluss Brandgefahr darstellen
- Absicherung verwenden (Fuse)
- Korrekte Kabelquerschnitte

⚠️ **Motoren:**
- Motoren entwickeln Drehmoment!
- Einklemmgefahr beachten
- NOT-AUS implementieren (`stopall`)

⚠️ **Endschalter:**
- NC (Normally Closed) für Fail-Safe
- Bei Kabelbruch: Motor stoppt
- Regelmäßig testen

⚠️ **Kabelwicklung Motor 2:**
- Maximal 3 Umdrehungen (1080°)
- FRAM Position Tracking kritisch!
- Regelmäßig auf Verschleiß prüfen

---

## Wartung

### Regelmäßig (täglich bei Messebetrieb)
- [ ] Kabel auf Verschleiß prüfen
- [ ] Endschalter-Funktion testen
- [ ] FRAM Position mit `framinfo` prüfen
- [ ] Re-Kalibrierung mit `setzero`

### Gelegentlich
- [ ] Stepper Driver Kühlkörper prüfen
- [ ] Motorstrom nachmessen
- [ ] Lager auf Leichtgängigkeit prüfen
- [ ] Verbindungen auf festen Sitz

### Vor jedem Transport
- [ ] Motor 1 auf Position 0 fahren (`gotohome1`)
- [ ] System ausschalten
- [ ] Motoren mechanisch sichern

---

## Pinout-Referenz Controllino MAXI Automation

**Digital Outputs (verwendet):**
- D4: Motor 1 DIR
- D6: Motor 1 STEP
- D7: Motor 2 STEP
- D8: Motor 2 DIR
- D9: Motor 2 ENABLE

**Digital Inputs (verwendet):**
- DI0 (CONTROLLINO_DI0): Motor 2 RIGHT Switch
- DI1 (CONTROLLINO_DI1): Motor 2 LEFT Switch

**I2C (verwendet):**
- Pin 20 (SDA): FRAM Data
- Pin 21 (SCL): FRAM Clock

**Serial (verwendet):**
- Serial0 (USB): 115200 baud - Kommandos & Debug

**Verfügbar für zukünftige Erweiterungen:**
- D0-D3, D5, D10-D23: Digital I/O
- DI2-DI15: Digital Inputs
- AI0-AI11: Analog Inputs
- AO0-AO3: Analog Outputs (0-10V)
- Serial1-3: Weitere serielle Schnittstellen
- Relays: R0-R9 (für Lasten)

---

## Ersatzteile & Bestellliste

### Kritische Komponenten
| Teil | Bezeichnung | Bestellnummer | Menge |
|------|-------------|---------------|-------|
| FRAM | MB85RC256V Breakout | Adafruit #1895 | 1 |
| Endschalter | NC Limit Switch | - | 2 |
| Stepper Driver | z.B. DRV8825 / TMC2208 | - | 2 |
| Stepper Motor 1 | 200 steps, 20:1 Gear | - | 1 |
| Stepper Motor 2 | 200 steps, 50:1 Gear | - | 1 |

### Kabel & Stecker
| Teil | Spezifikation | Verwendung |
|------|--------------|------------|
| Stepper Kabel | 4-adrig, geschirmt | Motor-Anschluss |
| Signalkabel | Twisted pair, 0.25mm² | Step/Dir Signale |
| I2C Kabel | 4-adrig, kurz | FRAM Anschluss |
| Litze flexibel | 0.5-1mm², mehrere Farben | Motor 2 Kabel |

---

## Schaltplan-Skizze

```
                    ┌─────────────────────────────────┐
                    │  Controllino MAXI Automation    │
                    │                                  │
    24V PSU ────────┤ 24V IN                      5V  ├─────┐
       │            │                                  │     │
       │            │  D4  ─────────────────────> DIR │     │
       │            │  D6  ─────────────────────> STEP│     │
       │            │                              Motor 1  │
       │            │                              Driver   │
       │            │  D7  ─────────────────────> STEP│     │
       │            │  D8  ─────────────────────> DIR │     │
       │            │  D9  ─────────────────────> EN  │     │
       │            │                              Motor 2  │
       │            │                              Driver   │
       │            │  DI0 <───────── Right Switch     │     │
       │            │  DI1 <───────── Left Switch      │     │
       │            │                                  │     │
       │            │  Pin 20 (SDA) <────> SDA        │     │
       │            │  Pin 21 (SCL) <────> SCL    FRAM│<────┘
       │            │                             VCC │
       │            │                                  │
       └────────────┤ GND                         GND ├─────┴─── Common GND
                    └─────────────────────────────────┘
```

---

**Stand:** November 2025  
**System:** FairFan Motor Controller v1.0  
**Hardware:** Controllino MAXI Automation + FRAM MB85RC256V
