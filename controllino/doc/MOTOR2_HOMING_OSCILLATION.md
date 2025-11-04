# Motor2 Homing und Oszillation - Implementierung

## Übersicht
Motor2 (Oszillationsmotor) führt einen präzisen Homing-Vorgang mit beidseitigen Offsets durch und oszilliert anschließend driftfrei zwischen den definierten Positionen.

## Homing-Sequenz

### Ablauf (RIGHT → LEFT)
1. **MOVE_RIGHT**: Fahrt zum rechten Endlagenschalter
2. **OFFSET_RIGHT**: Offset von 0.5° (111 Steps) nach links
3. **MOVE_LEFT**: Fahrt zum linken Endlagenschalter (Messung der Gesamtdistanz)
4. **OFFSET_LEFT**: Offset von 0.5° (111 Steps) nach rechts
5. **COMPLETE**: Homing abgeschlossen, Motor ist bereit für Oszillation

### Positions-Semantik
Nach dem RIGHT→LEFT Homing:
- **Position 0**: Nähe rechter Endlagenschalter (theoretisch)
- **Position ~111**: Rechter Offset (tatsächliche Start-/Endposition)
- **Position ~7426**: Linker Offset (andere Wendepunkt)
- **Hohe Position**: Links (Endlagenschalter bei totalRangeSteps)

### Beidseitige Offsets
- **OFFSET_DEGREES**: 0.5° (Config.h)
- **Umrechnung**: offsetSteps = 0.5° / 360° × 50 × 200 × 8 ≈ 111 Steps
- **Zweck**: Verhindert mechanische Belastung der Endlagenschalter
- **Nutzbereich**: homeRangeSteps = totalRangeSteps - (2 × offsetSteps)

## Oszillation

### Drift-freie Bewegung
- Motor oszilliert **exakt** `homeRangeSteps` in jede Richtung
- Jeder Schritt wird gezählt: `stepCount` vs. `homeRangeSteps`
- Bei Abweichung: `WARNING: Step count mismatch! DRIFT detected!`

### Position Tracking
```cpp
// LEFT-Bewegung: Von rechtem Offset zu linkem Offset
currentPosition = totalRangeSteps - offsetSteps;  // ~7426 (hoch)

// RIGHT-Bewegung: Von linkem Offset zu rechtem Offset  
currentPosition = offsetSteps;  // ~111 (niedrig)
```

### Trigger-Positionen
Motor2 wird durch Motor1 Position getriggert (symmetric around 720°):
- **MOTOR2_TRIGGER_OFFSET**: 60° (Config.h)
- **MOTOR2_TRIGGER_HIGH**: 660° (Motor1) → startet RIGHT-Bewegung
- **MOTOR2_TRIGGER_LOW**: 60° (Motor1) → startet LEFT-Bewegung

## Geschwindigkeitsprofil

### Parameter
- **TARGET_RPM**: 0.8 RPM (nach Getriebe)
- **ACCEL_ZONE**: 5% von 360° = 18°
- **DECEL_ZONE**: 5% von 360° = 18°
- **POWER_CURVE**: 0.9 (fast linear)
- **MIN_SPEED_FACTOR**: 0.1 (10% Mindestgeschwindigkeit)

### Trapezprofil
1. **Beschleunigung**: 18° mit Power-Curve 0.9
2. **Konstant**: Restliche Distanz bei 100% Geschwindigkeit
3. **Verzögerung**: 18° mit Power-Curve 0.9

## Autostart-Sequenz

### Vollständige Sequenz
1. **gotohome1**: Motor1 fährt zu Position 0° (falls nicht dort)
2. **home**: Motor2 Homing-Sequenz (RIGHT → LEFT)
3. **seq1**: Automatischer Start der Oszillations-Sequenz

### State Machine
```
IDLE → GOTO_HOME1 → WAIT_MOTOR1_HOME → START_HOMING → 
WAIT_HOMING → START_SEQUENCE → COMPLETE
```

### Konfiguration
```cpp
Config::Homing::AUTO_START_ON_BOOT = true;
Config::Sequence::AUTO_START_AFTER_HOMING = true;
```

## Debugging

### Serial Output
```
Homing Motor 2: Starting
Homing Motor 2: Moving to RIGHT switch...
Homing Motor 2: Right limit reached
Homing Motor 2: Offset from RIGHT complete, position = 111
Homing Motor 2: Moving to LEFT switch...
Homing Motor 2: Left limit reached, total range = 7537 steps
Homing Motor 2: Offset from LEFT complete, position = 7426
Homing Motor 2: Usable oscillation range = 7315 steps
Homing Motor 2: Complete!
```

### Oszillations-Verifikation
```
Motor2 startOscillation: direction=RIGHT, currentPos=7426, homeRange=7315
Motor2: Starting oscillation, will end at position 111
Motor2 oscillation COMPLETE: 7315 steps (expected 7315)
```

Wenn Drift auftritt:
```
WARNING: Step count mismatch! DRIFT detected!
```

## Technische Details

### Schrittberechnungen
- **Gear Ratio**: 50:1
- **Steps per Rev**: 200
- **Microsteps**: 8
- **Umrechnung**: 1° = 50 × 200 × 8 / 360 ≈ 222.2 Steps

### Messungen
- **Typischer Bereich**: ~33.6° (~7500 Steps)
- **Mit Offsets**: ~7315 Steps nutzbarer Bereich
- **Offset je Seite**: ~111 Steps (0.5°)

## Änderungshistorie

### 2025-11-04
- Homing-Richtung umgekehrt: LEFT→RIGHT zu RIGHT→LEFT
- Beidseitige Offsets implementiert (vorher nur einseitig)
- Positions-Tracking für neue Homing-Richtung korrigiert
- Trigger-Richtungen an neue Position-Semantik angepasst
- Drift-Erkennung implementiert
- Autostart-Sequenz vervollständigt (gotohome1 → home → seq1)
- Geschwindigkeitsparameter optimiert (4.0 RPM Motor1, 0.8 RPM Motor2)
- Trigger-Positionen symmetrisch konfiguriert (60° Offset)
