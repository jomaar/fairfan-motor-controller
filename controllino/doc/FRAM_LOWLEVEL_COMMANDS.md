# FRAM Low-Level Read/Write Commands

## Übersicht

Diese Befehle ermöglichen direkten Zugriff auf den FRAM-Speicher für Debugging, Testing und erweiterte Anwendungen.

**ACHTUNG:** Diese Befehle können die Position-Tracking-Daten überschreiben! Nur für Experten/Entwickler.

---

## Befehle

### 1. Write Byte to FRAM

**Syntax:**
```
wFRAM<address>-<value>
```

**Beispiele:**
```
wFRAM0-255          # Schreibe 255 (0xFF) an Adresse 0
wFRAM100-42         # Schreibe 42 (0x2A) an Adresse 100
wFRAM0x10-0xFF      # Schreibe 0xFF an Adresse 0x10 (16)
wFRAM0x0100-128     # Schreibe 128 an Adresse 0x0100 (256)
```

**Parameter:**
- `<address>`: 0-32767 (0x0000-0x7FFF) - FRAM-Adresse
  - Dezimal: `wFRAM100-42`
  - Hexadezimal: `wFRAM0x10-42`
- `<value>`: 0-255 (0x00-0xFF) - Byte-Wert
  - Dezimal: `wFRAM100-42`
  - Hexadezimal: `wFRAM100-0xFF`

**Ausgabe:**
```
> wFRAM100-42

FRAM[0x0064] = 0x2A (42)
```

---

### 2. Read Byte from FRAM

**Syntax:**
```
rFRAM<address>
```

**Beispiele:**
```
rFRAM0              # Lese Byte an Adresse 0
rFRAM100            # Lese Byte an Adresse 100
rFRAM0x10           # Lese Byte an Adresse 0x10 (16)
rFRAM0x0100         # Lese Byte an Adresse 0x0100 (256)
```

**Parameter:**
- `<address>`: 0-32767 (0x0000-0x7FFF) - FRAM-Adresse
  - Dezimal: `rFRAM100`
  - Hexadezimal: `rFRAM0x10`

**Ausgabe:**
```
> rFRAM100

FRAM[0x0064] = 0x2A (42)
```

---

### 3. Read Range from FRAM

**Syntax:**
```
rFRAM<start>-<end>
```

**Beispiele:**
```
rFRAM0-15           # Lese 16 Bytes (0x00-0x0F)
rFRAM100-115        # Lese 16 Bytes (100-115)
rFRAM0x00-0x0F      # Lese 16 Bytes in Hex
rFRAM0x0000-0x001F  # Lese 32 Bytes (Position-Tracking Daten)
```

**Parameter:**
- `<start>`: Start-Adresse (0-32767)
- `<end>`: End-Adresse (0-32767)
- **Limit:** Max. 256 Bytes pro Abfrage

**Ausgabe:**
```
> rFRAM0-15

FRAM[0x0000 - 0x000F]:
0x0000: FA FA 00 00 00 00 12 34 FA FA 00 00 00 00 56 78
```

Formatierung: 16 Bytes pro Zeile im Hex-Dump Format

---

## FRAM Memory Map

### Position Tracking Daten (Auto-verwaltet)

```
Slot A (Primary):
  0x0000-0x0001: Magic Number (0xFAFA)
  0x0002-0x0005: Position (signed long, 4 bytes)
  0x0006-0x0007: CRC16 Checksum (2 bytes)

Slot B (Backup):
  0x0008-0x0009: Magic Number (0xFAFA)
  0x000A-0x000D: Position (signed long, 4 bytes)
  0x000E-0x000F: CRC16 Checksum (2 bytes)

User Area:
  0x0010-0x7FFF: Frei verfügbar (32752 bytes)
```

### Wichtige Adressen

| Adresse | Beschreibung | Format |
|---------|--------------|--------|
| 0x0000 | Slot A Magic | 0xFAFA |
| 0x0002 | Slot A Position (MSB) | Byte 3 von 4 |
| 0x0003 | Slot A Position | Byte 2 von 4 |
| 0x0004 | Slot A Position | Byte 1 von 4 |
| 0x0005 | Slot A Position (LSB) | Byte 0 von 4 |
| 0x0006 | Slot A CRC16 (High) | CRC MSB |
| 0x0007 | Slot A CRC16 (Low) | CRC LSB |
| 0x0008 | Slot B Magic | 0xFAFA |
| 0x000A-0x000D | Slot B Position | Signed long |
| 0x000E-0x000F | Slot B CRC16 | CRC16 |

---

## Anwendungsfälle

### 1. Position-Tracking Daten inspizieren

**Slot A lesen:**
```
> rFRAM0-7

FRAM[0x0000 - 0x0007]:
0x0000: FA FA 00 00 12 34 A5 B2
```

Interpretation:
- `FA FA` = Magic Number ✓
- `00 00 12 34` = Position = 0x00001234 = 4660 steps
- `A5 B2` = CRC16 Checksum

**Slot B lesen:**
```
> rFRAM8-15

FRAM[0x0008 - 0x000F]:
0x0008: FA FA 00 00 12 34 A5 B2
```

---

### 2. Position manuell setzen (für Tests)

**Warnung:** Überschreibt Position-Tracking!

```
# Position auf 1000 steps setzen (0x000003E8)
> wFRAM2-0x00        # MSB
> wFRAM3-0x00
> wFRAM4-0x03
> wFRAM5-0xE8        # LSB

# CRC neu berechnen lassen (System neu starten)
# Oder: framclear + setzero verwenden
```

---

### 3. User-Daten im freien Bereich speichern

**Ab Adresse 0x0010 (16) ist der Speicher frei:**

```
# Beispiel: Betriebsstunden speichern
> wFRAM0x10-0x12     # Hours LSB
> wFRAM0x11-0x34     # Hours
> wFRAM0x12-0x56     # Hours
> wFRAM0x13-0x78     # Hours MSB

# Später auslesen:
> rFRAM0x10-0x13

FRAM[0x0010 - 0x0013]:
0x0010: 12 34 56 78
```

---

### 4. FRAM komplett auslesen

**Erste 32 Bytes (Position + User-Start):**
```
> rFRAM0-31

FRAM[0x0000 - 0x001F]:
0x0000: FA FA 00 00 12 34 A5 B2 FA FA 00 00 12 34 A5 B2
0x0010: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
```

**Hinweis:** Max. 256 Bytes pro Befehl!

---

### 5. FRAM-Bereich löschen

**User-Bereich löschen (ohne Position-Tracking):**
```
# Von 0x0010 bis 0x010F (256 Bytes)
> wFRAM0x10-0        # Start bei 0x10
> wFRAM0x11-0
> wFRAM0x12-0
... (manuell für jeden Byte)
```

**Oder:** `framclear` löscht kompletten FRAM

---

### 6. Hex-Dump für Analyse

**Position-Tracking komplett:**
```
> rFRAM0-15

FRAM[0x0000 - 0x000F]:
0x0000: FA FA 00 00 12 34 A5 B2 FA FA 00 00 12 34 A5 B2
        ^^^^^ Magic   ^^^^^^^^^ Position ^^^^^ CRC16
              ^^^^^ Magic      ^^^^^^^^^ Position ^^^^^ CRC16
              Slot A                    Slot B
```

---

## Error-Handling

### Adresse außerhalb des Bereichs

```
> rFRAM0x8000

ERROR: Address out of range (max 0x7FFF): 0x8000
```

### Ungültiges Format

```
> wFRAM100

ERROR: Invalid format. Use: wFRAM<addr>-<value>
Examples: wFRAM100-255, wFRAM0x10-0xFF
```

### Zu großer Bereich

```
> rFRAM0-1000

ERROR: Range too large (max 256 bytes)
```

### Start > End

```
> rFRAM100-50

ERROR: Start address must be <= end address
```

---

## Sicherheitshinweise

⚠️ **WICHTIG:**

1. **Position-Tracking nicht überschreiben!**
   - Adressen 0x0000-0x000F sind reserviert
   - Überschreiben führt zu Position-Verlust
   - Bei Bedarf: `framclear` → `setzero`

2. **CRC16 berücksichtigen**
   - Manuelle Änderungen invalidieren Checksumme
   - System erkennt korrupte Daten
   - Nach manuellen Änderungen: Neustart

3. **Kein Write während Bewegung**
   - System schreibt automatisch während Bewegung
   - Konflikt möglich!
   - Empfehlung: Motor stoppen vor manuellen Writes

4. **FRAM-Lebensdauer**
   - 10^14 Schreibzyklen
   - Praktisch unbegrenzt für normale Nutzung
   - Aber: Unnötige Writes vermeiden

---

## Debugging-Beispiele

### Problem: Position stimmt nicht

**1. Position auslesen:**
```
> framinfo

Last Saved Position: +1234 steps = +13.91°
```

**2. FRAM-Daten prüfen:**
```
> rFRAM0-15

FRAM[0x0000 - 0x000F]:
0x0000: FA FA 00 00 04 D2 XX XX FA FA 00 00 04 D2 XX XX
                      ^^^^^ = 0x04D2 = 1234 ✓
```

**3. Magic Numbers prüfen:**
```
> rFRAM0-1

FRAM[0x0000] = 0xFA (250)
FRAM[0x0001] = 0xFA (250)

✓ Slot A Magic OK
```

---

### Problem: FRAM korrupt

**1. Beide Slots prüfen:**
```
> rFRAM0-15

FRAM[0x0000 - 0x000F]:
0x0000: 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00 00
```

**2. FRAM zurücksetzen:**
```
> framclear

FRAM cleared, position reset to 0
```

**3. Position neu kalibrieren:**
```
> setzero

Motor1 position set to ZERO (home)
FRAM updated - this is now the reference point
```

---

## Erweiterte Nutzung

### Custom Data Storage

**Beispiel: Wartungsintervall speichern**

```cpp
// Adresse 0x20-0x23: Betriebsstunden (4 bytes)
// Adresse 0x24-0x27: Wartungszähler (4 bytes)

// Speichern (via Serial):
> wFRAM0x20-0x00    # Hours (MSB)
> wFRAM0x21-0x01
> wFRAM0x22-0x5E
> wFRAM0x23-0x80    # Hours (LSB) = 90000

// Auslesen:
> rFRAM0x20-0x27

FRAM[0x0020 - 0x0027]:
0x0020: 00 01 5E 80 00 00 00 00
```

---

## Kommando-Referenz (Kurzform)

| Befehl | Beschreibung | Beispiel |
|--------|--------------|----------|
| `wFRAM<a>-<v>` | Write byte | `wFRAM100-42` |
| `rFRAM<a>` | Read byte | `rFRAM100` |
| `rFRAM<s>-<e>` | Read range | `rFRAM0-15` |

**Adresse-Formate:**
- Dezimal: `100`, `1234`
- Hexadezimal: `0x10`, `0x0100`

**Wert-Formate:**
- Dezimal: `42`, `255`
- Hexadezimal: `0x2A`, `0xFF`

---

## Tipps & Tricks

1. **Hex-Adressen verwenden:**
   ```
   rFRAM0x0000-0x000F  (leichter lesbar als rFRAM0-15)
   ```

2. **Daten in 16-Byte-Blöcken lesen:**
   ```
   rFRAM0x00-0x0F     # Block 1
   rFRAM0x10-0x1F     # Block 2
   rFRAM0x20-0x2F     # Block 3
   ```

3. **Position-Tracking nicht stören:**
   ```
   # Sicher: Ab 0x10 (16)
   wFRAM0x10-0xFF
   
   # UNSICHER: Überschreibt Position!
   wFRAM0x00-0xFF  ❌
   ```

4. **Hex-Dumps speichern:**
   ```
   Serial Monitor → Copy/Paste → Textdatei
   Später vergleichen/analysieren
   ```

---

**Stand:** November 2025  
**Version:** FairFan Motor Controller v1.0  
**FRAM:** MB85RC256V (32KB @ I2C 0x50)
