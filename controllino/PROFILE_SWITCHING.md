# Speed Profile System - Kurzanleitung

## So funktioniert's

### 1. Profile bearbeiten
Öffne `include/SpeedProfiles.h` und ändere die Werte:

```cpp
// PROFILE 2 - Fast
#define M1_P2_RPM 6.0f          // ← Ändere RPM
#define M1_P2_ACCEL 0.04f       // ← Ändere Beschleunigung
#define M1_P2_DECEL 0.04f
#define M1_P2_CURVE 1.8f
#define M1_P2_MINSPEED 0.12f
```

### 2. Profil wählen
Öffne `include/Config.h` und ändere die Zeile:

```cpp
#define ACTIVE_PROFILE 2  // ← Ändere zu 1, 2 oder 3
```

### 3. Kompilieren & Hochladen
```bash
pio run -t upload
```

### 4. Testen
- Beobachte das Verhalten
- Wenn gut → speichere Werte in SpeedProfiles.h
- Wenn nicht gut → passe Werte an und wiederhole

## Verfügbare Profile

| Profile | Motor1 RPM | Motor2 RPM | Beschreibung |
|---------|------------|------------|--------------|
| **1** | 5.0 | 1.2 | Balanced (Standard) |
| **2** | 6.0 | 1.5 | Fast (zum Testen) |
| **3** | 4.0 | 0.8 | Smooth (zum Testen) |

## Parameter Bedeutung

- **RPM**: Geschwindigkeit (höher = schneller)
- **ACCEL/DECEL**: Beschleunigungszone (0.05 = 5% von 360°)
- **CURVE**: Beschleunigungskurve (< 1.0 sanft, > 1.0 aggressiv)
- **MINSPEED**: Mindestgeschwindigkeit (0.1 = 10%)

## Beispiel Workflow

1. Teste Profile 1 (Standard)
2. Ändere Profile 2 auf andere RPM-Werte
3. Setze `ACTIVE_PROFILE 2` in Config.h
4. Upload und teste
5. Wenn perfekt: Behalte Werte in Profile 2
6. Teste Profile 3 mit anderen Werten
7. Am Ende hast du 3 getestete, bewährte Profile gespeichert
