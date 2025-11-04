# FRAM Position Tracking System

## Übersicht

Das System verwendet ein **MB85RC256V FRAM** (32KB, I2C-Adresse 0x50) für persistente Speicherung der Motor1-Position. Dies verhindert Kabelschäden durch unbekannte Motorposition nach Stromausfall.

## Problem

Motor2 ist auf der Welle von Motor1 montiert. Die Kabel von Motor2 wickeln sich bei Rotation von Motor1 um die Welle. Bei Stromausfall ohne Positions-Tracking würde das System nicht wissen, wo Motor1 steht, und könnte die Kabel durch weitere Rotation abreißen.

## Lösung

### Hardware
- **FRAM:** MB85RC256V (I2C 0x50)
- **Anschluss:** Controllino I2C (SDA/SCL)
- **Schreibzyklen:** 10^14 (praktisch unbegrenzt für Messebetrieb)

### Software-Features

#### 1. Dual-Slot Ping-Pong Storage
```
Slot A: 0x0000-0x0007 (Magic + Position + CRC16)
Slot B: 0x0008-0x000F (Magic + Position + CRC16)
```
- Wechselt zwischen Slots bei jedem Schreibvorgang
- Schutz gegen Datenverlust während Schreibvorgang
- CRC16-Checksummen für Integritätsprüfung

#### 2. Position Tracking
- **Signed Position:** + = CW, - = CCW
- **Auflösung:** Alle ~0.5° (44 steps)
- **Update:** Bei Bewegungsende + periodisch während Bewegung
- **Limits:** Soft-Limit 900°, Hard-Limit 1080° (3 Umdrehungen)

#### 3. Auto-Recovery nach Stromausfall
1. Boot → FRAM auslesen
2. Position validieren (Dual-Slot + CRC)
3. Automatische Rückfahrt zu Home (Position 0)
4. Erst dann Motor2-Homing

## Neue Befehle

### Position Management
- **`setzero`** - Aktuelle Position als Home (0) setzen
- **`getpos1`** - Zeige aktuelle Position von Motor1
- **`gotohome1`** - Fahre Motor1 zu Home-Position (0)
- **`poslimit`** - Zeige Rotationslimits und verbleibende Kapazität

### FRAM Management
- **`framinfo`** - Zeige FRAM-Status und gespeicherte Position
- **`framclear`** - FRAM zurücksetzen (Factory Reset)

## Konfiguration (Config.h)

```cpp
namespace Motor1 {
    constexpr float MAX_DEGREES = 1080.0f;           // 3 Umdrehungen max
    constexpr float SOFT_LIMIT_DEGREES = 900.0f;     // Warnung bei 2.5 Umdrehungen
    constexpr long FRAM_UPDATE_INTERVAL_STEPS = 44;  // ~0.5° Auflösung
}

namespace FRAM {
    constexpr uint8_t I2C_ADDRESS = 0x50;
    constexpr bool ENABLE_AUTO_RECOVERY = true;      // Auto-Rückfahrt nach Boot
    constexpr bool RECOVERY_BEFORE_HOMING = true;    // Vor Motor2-Homing
}
```

## Workflow

### Initial Setup
1. System starten
2. Motor1 manuell zur mechanischen Markierung fahren
3. Befehl `setzero` eingeben
4. FRAM speichert neue Nullposition

### Normalbetrieb
1. Motor1 bewegt sich
2. Position wird automatisch alle ~0.5° ins FRAM geschrieben
3. Bei Bewegungsende: finale Position speichern
4. Position bleibt nach Stromausfall erhalten

### Nach Stromausfall
1. System bootet
2. FRAM auslesen: z.B. +720 steps (2 Umdrehungen CW)
3. **Automatisch:** Motor1 fährt 720 steps CCW zurück zu Home
4. **Bestätigung:** Position = 0, FRAM aktualisiert
5. Dann erst Motor2-Homing
6. Normalbetrieb

### Re-Kalibrierung (empfohlen alle paar Betriebstage)
1. Motor1 manuell zur mechanischen Markierung fahren
2. Über Web-Interface oder Serial: `setzero`
3. Drift wird ausgeglichen

## Sicherheitsfeatures

### Soft-Limits (900°)
```
WARNING: Approaching cable limit! Current: 850.0°, After move: 950.0°
```
- Warnung wird ausgegeben
- Bewegung wird **nicht** blockiert
- Operateur kann entscheiden

### Hard-Limits (1080°)
```
ERROR: Movement blocked! Would exceed 1080° limit
Use 'gotohome1' to return to zero position
```
- Bewegung wird **blockiert**
- Verhindert Kabelschaden
- Nur Rückfahrt zu Home erlaubt

## Technische Details

### Position Berechnung
```cpp
Motor1: 20 gear × 200 steps × 8 microsteps = 32000 steps/360°

Position = ±steps
  + = Clockwise (CW)
  - = Counter-Clockwise (CCW)

Beispiel:
  +32000 steps = +360° = 1 Umdrehung CW
  -16000 steps = -180° = halbe Umdrehung CCW
```

### FRAM Schreibrate
```
Bei 0.5° Intervallen:
  1080° max / 0.5° = 2160 Schreibvorgänge für volle Range
  
Bei 800h Messebetrieb:
  ~19.2 Milliarden motor steps
  ~400.000 FRAM writes (bei 0.5° Intervall)
  
FRAM Lebensdauer:
  10^14 / 400.000 = 250 Millionen Messezyklen möglich
```

### ISR Performance
- **FRAM-Schreiben erfolgt NICHT im ISR**
- ISR setzt nur `positionChanged` Flag
- Main Loop schreibt asynchron
- I2C dauert ~2-5ms, ISR muss <10µs sein

## Memory Layout

```
FRAM MB85RC256V (32KB @ 0x50)
├── Slot A (Primary)
│   ├── 0x0000-0x0001: Magic (0xFAFA)
│   ├── 0x0002-0x0005: Position (signed long)
│   └── 0x0006-0x0007: CRC16
│
├── Slot B (Backup)
│   ├── 0x0008-0x0009: Magic (0xFAFA)
│   ├── 0x000A-0x000D: Position (signed long)
│   └── 0x000E-0x000F: CRC16
│
└── 0x0010-0x7FFF: Reserved (future use)
```

## Fehlerbehandlung

### FRAM nicht gefunden
```
ERROR: FRAM not found at 0x50!
WARNING: Position tracking will not be persistent.
```
System läuft weiter, aber ohne persistente Speicherung.

### Beide Slots korrupt
```
FRAM: No valid data found, assuming home position (0)
```
System initialisiert beide Slots mit Position 0.

### Ein Slot korrupt
```
FRAM: Slot A INVALID
FRAM: Using Slot B: +1234
```
System verwendet den gültigen Slot.

## Debugging

### FRAM Status prüfen
```
> framinfo

=== FRAM Status ===
Initialized: YES
I2C Address: 0x50
Last Saved Position: +1234 steps
                    = +13.91°
==================
```

### Position prüfen
```
> getpos1

Motor1 Position: +1234 steps = +13.91°
  Remaining CW:  1066.1°
  Remaining CCW: 1093.9°
```

### Position bei Limit
```
> poslimit

Motor1 Position: +28000 steps = +315.00°
  Remaining CW:  765.0°
  Remaining CCW: 1395.0°
  ⚠️  WARNING: Near cable limit!
```

## Best Practices

1. **Initial Setup:** Immer mit `setzero` bei mechanischer Markierung kalibrieren
2. **Regelmäßige Re-Kalibrierung:** Alle paar Betriebstage zur Markierung zurückfahren
3. **Vor Wartung:** `framinfo` aufrufen, Position notieren
4. **Nach langer Pause:** Position mit `getpos1` prüfen
5. **Bei Unsicherheit:** `gotohome1` → `setzero` bei mechanischer Markierung

## Zukünftige Erweiterungen

- Web-Interface Integration
- Position-Historie (nutzt restliche 32KB FRAM)
- Automatische Drift-Korrektur
- Betriebsstunden-Tracking
- Wartungsintervalle
