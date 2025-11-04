#ifndef SPEED_PROFILES_H
#define SPEED_PROFILES_H

/**
 * Speed Profile Presets
 * 
 * Definiere hier 3 verschiedene Profile zum Testen.
 * Wechsel zwischen Profilen durch Ändern von ACTIVE_PROFILE in Config.h
 * und neu kompilieren.
 * 
 * Workflow:
 * 1. Ändere Werte für Profile 1, 2, oder 3 hier
 * 2. Setze ACTIVE_PROFILE = 1, 2 oder 3 in Config.h
 * 3. Kompiliere und teste
 * 4. Speichere gute Profile hier als Preset
 */

// ============================================================
// PROFILE 1 - Balanced (aktueller Standard)
// ============================================================
#define M1_P1_RPM 5.0f
#define M1_P1_ACCEL 0.05f
#define M1_P1_DECEL 0.05f
#define M1_P1_CURVE 1.5f
#define M1_P1_MINSPEED 0.1f

#define M2_P1_RPM 1.2f
#define M2_P1_ACCEL 0.05f
#define M2_P1_DECEL 0.05f
#define M2_P1_CURVE 0.9f
#define M2_P1_MINSPEED 0.1f

#define M2_P1_TRIGGER_OFFSET 30.0f

// ============================================================
// PROFILE 2 - Fast (zum Testen)
// ============================================================
#define M1_P2_RPM 8.0f
#define M1_P2_ACCEL 0.2f
#define M1_P2_DECEL 0.2f
#define M1_P2_CURVE 0.5f
#define M1_P2_MINSPEED 0.1f

#define M2_P2_RPM 2.0f
#define M2_P2_ACCEL 0.4f
#define M2_P2_DECEL 0.2f
#define M2_P2_CURVE 0.5f
#define M2_P2_MINSPEED 0.12f

#define M2_P2_TRIGGER_OFFSET 70.0f

// ============================================================
// PROFILE 3 - Smooth (zum Testen)
// ============================================================
#define M1_P3_RPM 4.0f
#define M1_P3_ACCEL 0.08f
#define M1_P3_DECEL 0.08f
#define M1_P3_CURVE 1.2f
#define M1_P3_MINSPEED 0.1f

#define M2_P3_RPM 0.8f
#define M2_P3_ACCEL 0.08f
#define M2_P3_DECEL 0.08f
#define M2_P3_CURVE 0.8f
#define M2_P3_MINSPEED 0.1f

#define M2_P3_TRIGGER_OFFSET 45.0f

#endif // SPEED_PROFILES_H
