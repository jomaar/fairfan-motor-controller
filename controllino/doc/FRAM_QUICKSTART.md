# FRAM Position Tracking - Quick Start Guide

## Hardware Setup

1. **FRAM Modul:** MB85RC256V (Adafruit #1895 oder kompatibel)
2. **Anschluss an Controllino MAXI Automation:**
   ```
   FRAM VCC  → 5V
   FRAM GND  → GND
   FRAM SDA  → SDA (Pin 20)
   FRAM SCL  → SCL (Pin 21)
   ```
3. **I2C-Adresse:** 0x50 (Standard für MB85RC256V)

## Software Upload

```bash
cd controllino
platformio run --target upload --environment controllino_maxi_automation
```

## Erste Inbetriebnahme

### 1. System starten und FRAM prüfen

```
Serial Monitor öffnen (115200 baud)

Ausgabe beim Boot:
  Initializing FRAM...
  FRAM MB85RC256V initialized
  Motor1 starting at home position (0)
  System initialized
```

### 2. FRAM-Status prüfen

```
> framinfo

=== FRAM Status ===
Initialized: YES
I2C Address: 0x50
Last Saved Position: 0 steps
                    = 0.00°
==================
```

### 3. Initial-Kalibrierung

**Wichtig:** Motor1 muss bei mechanischer Markierung am Gehäuse stehen!

```
1. Motor1 manuell zur Markierung bewegen (falls nötig)
2. Befehl eingeben:
   > setzero
   
   Motor1 position set to ZERO (home)
   FRAM updated - this is now the reference point
```

## Testbetrieb

### Test 1: Bewegung mit Position-Tracking

```
> m1cw90

Motor 1: Started 90.0° CW
Position saved: +8000 steps (+90.00°)
```

### Test 2: Position abfragen

```
> getpos1

Motor1 Position: +8000 steps = +90.00°
  Remaining CW:  990.0°
  Remaining CCW: 1170.0°
```

### Test 3: Zurück zu Home

```
> gotohome1

Returning to home: 90.00° CCW
Position saved: 0 steps (0.00°)
```

### Test 4: Stromausfall simulieren

```
1. Motor bewegen:
   > m1cw180
   Position saved: +16000 steps (+180.00°)

2. Controllino ausschalten (Stromzufuhr trennen)

3. Controllino einschalten

4. Boot-Meldung prüfen:
   ⚠️  POWER LOSS DETECTED! Saved position: +16000 steps (+180.00°)
   AUTO-RECOVERY: Returning Motor1 to home position...
   Recovery: 180.00° CCW
   ✓ Motor1 recovery complete - at home position
```

## Normale Verwendung

### Motor bewegen (Position wird automatisch gespeichert)

```
> m1cw360      # 360° im Uhrzeigersinn
> m1ccw180     # 180° gegen Uhrzeigersinn
> go1          # Standard-Test (180°)
```

### Position-Verwaltung

```
> getpos1      # Aktuelle Position anzeigen
> poslimit     # Limits und verbleibende Rotation
> gotohome1    # Zurück zur Nullposition
> setzero      # Aktuelle Position als Null setzen
```

### FRAM-Verwaltung

```
> framinfo     # FRAM-Status anzeigen
> framclear    # FRAM zurücksetzen (ACHTUNG!)
```

## Sicherheits-Limits

### Soft-Limit: 900° (2.5 Umdrehungen)

```
⚠️ Warnung wird ausgegeben
✓ Bewegung wird NICHT blockiert
→ Operateur kann entscheiden
```

### Hard-Limit: 1080° (3 Umdrehungen)

```
❌ Bewegung wird BLOCKIERT
→ Verhindert Kabelschaden
→ Nur Rückfahrt erlaubt: gotohome1
```

Beispiel:
```
> m1cw1100

ERROR: Movement blocked! Would exceed 1080° limit
Current: 200.0°, Requested: 1300.0°
Use 'gotohome1' to return to zero position
```

## Auto-Recovery (Stromausfall)

Das System erkennt automatisch Stromausfälle:

1. **Position wird erkannt:**
   ```
   ⚠️  POWER LOSS DETECTED! Saved position: +720 steps (+81.00°)
   ```

2. **Automatische Rückfahrt:**
   ```
   AUTO-RECOVERY: Returning Motor1 to home position...
   Recovery: 81.00° CCW
   ```

3. **Bestätigung:**
   ```
   ✓ Motor1 recovery complete - at home position
   ```

4. **Normalbetrieb:**
   ```
   Starting automatic homing of Motor2...
   [Motor2 Homing läuft...]
   ```

## Regelmäßige Re-Kalibrierung

**Empfohlen:** Alle paar Betriebstage zur Korrektur von akkumuliertem Drift

1. Motor1 zur mechanischen Markierung fahren
2. `setzero` ausführen
3. Drift wird ausgeglichen

## Konfiguration ändern

In `include/Config.h`:

```cpp
namespace FRAM {
    // Auto-Recovery ein/aus
    constexpr bool ENABLE_AUTO_RECOVERY = true;
    
    // Recovery vor oder nach Motor2-Homing
    constexpr bool RECOVERY_BEFORE_HOMING = true;
}

namespace Motor1 {
    // Limits ändern (VORSICHT!)
    constexpr float MAX_DEGREES = 1080.0f;      // Hard-Limit
    constexpr float SOFT_LIMIT_DEGREES = 900.0f; // Soft-Limit
    
    // FRAM Update-Intervall
    constexpr long FRAM_UPDATE_INTERVAL_STEPS = 44;  // ~0.5°
}
```

## Troubleshooting

### FRAM nicht gefunden

```
ERROR: FRAM not found at 0x50!
WARNING: Position tracking will not be persistent.
```

**Lösung:**
1. I2C-Verkabelung prüfen (SDA/SCL vertauscht?)
2. FRAM-Adresse prüfen (Jumper auf Modul?)
3. 5V-Versorgung prüfen

### Position nach Recovery falsch

```
Motor steht nicht an mechanischer Markierung nach Recovery
```

**Lösung:**
1. Manuell zur Markierung fahren
2. `setzero` ausführen
3. Erneut testen

### Bewegung blockiert

```
ERROR: Movement blocked! Would exceed 1080° limit
```

**Lösung:**
```
> gotohome1    # Zurück zu Position 0
> setzero      # Bei mechanischer Markierung neu kalibrieren
```

## Kommando-Referenz

| Befehl | Beschreibung |
|--------|--------------|
| `setzero` | Position 0 setzen (bei Markierung!) |
| `getpos1` | Position anzeigen |
| `gotohome1` | Zu Position 0 fahren |
| `poslimit` | Limits und verbleibende Rotation |
| `framinfo` | FRAM-Status |
| `framclear` | FRAM zurücksetzen |
| `m1cw<n>` | n° im Uhrzeigersinn |
| `m1ccw<n>` | n° gegen Uhrzeigersinn |
| `go1` | Test-Bewegung (180°) |
| `stop1` | Motor1 stoppen |
| `help` | Alle Befehle anzeigen |

## Wichtige Hinweise

⚠️ **Immer bei mechanischer Markierung `setzero` nach:**
- Erster Inbetriebnahme
- Manual-Bewegung des Motors (bei ausgeschaltetem System)
- Verdacht auf Position-Drift
- Wartungsarbeiten

✅ **FRAM schreibt automatisch:**
- Alle ~0.5° während Bewegung
- Bei Bewegungsende
- Kein manueller Eingriff nötig!

🔄 **Auto-Recovery schützt Kabel:**
- Passiert automatisch nach Stromausfall
- Fährt immer zu Position 0 zurück
- Erst dann Motor2-Homing

📊 **Position-Genauigkeit:**
- Auflösung: ±0.5° (44 steps)
- Ausreichend für Kabel-Toleranz
- Drift-Korrektur durch Re-Kalibrierung
