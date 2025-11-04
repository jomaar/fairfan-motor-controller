#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <Controllino.h>
#include "SpeedProfiles.h"

// ============================================================
// ACTIVE SPEED PROFILE SELECTION
// ============================================================
// Ändere diese Zahl (1, 2, oder 3) um zwischen Profilen zu wechseln
// Profile sind in SpeedProfiles.h definiert
#define ACTIVE_PROFILE 2

namespace Config {
    // Motor Direction Constants (more readable than LOW/HIGH)
    constexpr bool CCW_LEFT = false;    // Counter-clockwise / Left direction (LOW)
    constexpr bool CW_RIGHT = true;     // Clockwise / Right direction (HIGH)
    
    // Motor 1 Parameters (Main Motor)
    namespace Motor1 {
        constexpr uint8_t STEP_PIN = 6;              // Digital pin for step pulses (one pulse = one microstep)
        constexpr uint8_t DIR_PIN = 4;               // Digital pin for direction control (HIGH/LOW)
        constexpr uint16_t STEPS_PER_REV = 200;      // Full steps per revolution (1.8° step angle motor)
        constexpr uint8_t MICROSTEPS = 8;            // Microstepping driver setting (1/8 step)
        constexpr uint8_t GEAR_RATIO = 20;           // Gear reduction ratio (20:1)
        constexpr float TEST_DEGREES = 180.0f;       // Movement angle for 'go1' test command (half rotation)
        constexpr float MAX_DEGREES = 360.0f;        // Maximum allowed rotation (1 full rotation = safety limit)
        constexpr float SOFT_LIMIT_DEGREES = 900.0f; // Soft warning limit (2.5 rotations)
        
        // Speed Profile - loaded from SpeedProfiles.h based on ACTIVE_PROFILE
        #if ACTIVE_PROFILE == 1
            constexpr float TARGET_RPM = M1_P1_RPM;
            constexpr float ACCEL_ZONE = M1_P1_ACCEL;
            constexpr float DECEL_ZONE = M1_P1_DECEL;
            constexpr float POWER_CURVE = M1_P1_CURVE;
            constexpr float MIN_SPEED_FACTOR = M1_P1_MINSPEED;
        #elif ACTIVE_PROFILE == 2
            constexpr float TARGET_RPM = M1_P2_RPM;
            constexpr float ACCEL_ZONE = M1_P2_ACCEL;
            constexpr float DECEL_ZONE = M1_P2_DECEL;
            constexpr float POWER_CURVE = M1_P2_CURVE;
            constexpr float MIN_SPEED_FACTOR = M1_P2_MINSPEED;
        #elif ACTIVE_PROFILE == 3
            constexpr float TARGET_RPM = M1_P3_RPM;
            constexpr float ACCEL_ZONE = M1_P3_ACCEL;
            constexpr float DECEL_ZONE = M1_P3_DECEL;
            constexpr float POWER_CURVE = M1_P3_CURVE;
            constexpr float MIN_SPEED_FACTOR = M1_P3_MINSPEED;
        #else
            #error "ACTIVE_PROFILE must be 1, 2, or 3"
        #endif
        
        // Position Tracking & FRAM
        // Motor1: 20 gear × 200 steps × 8 microsteps = 32000 steps/360°
        // 0.5° = 32000 / 360 × 0.5 ≈ 44 steps
        constexpr long FRAM_UPDATE_INTERVAL_STEPS = 44;  // Update FRAM every ~0.5° (cable has tolerance)
        constexpr float FRAM_UPDATE_INTERVAL_DEGREES = 0.5f;
    }
    
    // Motor 2 Parameters (Oscillation Motor)
    namespace Motor2 {
        constexpr uint8_t STEP_PIN = 7;                          // Digital pin for step pulses
        constexpr uint8_t DIR_PIN = 8;                           // Digital pin for direction control
        constexpr uint8_t ENABLE_PIN = 9;                        // Enable/disable motor driver (LOW=enabled)
        constexpr uint8_t LEFT_SWITCH_PIN = CONTROLLINO_DI1;     // Limit switch at left position (NC = normally closed)
        constexpr uint8_t RIGHT_SWITCH_PIN = CONTROLLINO_DI0;    // Limit switch at right position (NC = normally closed)
        constexpr uint16_t STEPS_PER_REV = 200;                  // Full steps per revolution (1.8° step angle motor)
        constexpr uint8_t MICROSTEPS = 8;                        // Microstepping driver setting (1/8 step)
        constexpr uint8_t GEAR_RATIO = 50;                       // Gear reduction ratio (50:1)
        constexpr float OFFSET_DEGREES = 0.5f;                   // Offset from right limit switch after homing (safety margin)
        
        // Speed Profile - loaded from SpeedProfiles.h based on ACTIVE_PROFILE
        #if ACTIVE_PROFILE == 1
            constexpr float TARGET_RPM = M2_P1_RPM;
            constexpr float ACCEL_ZONE = M2_P1_ACCEL;
            constexpr float DECEL_ZONE = M2_P1_DECEL;
            constexpr float POWER_CURVE = M2_P1_CURVE;
            constexpr float MIN_SPEED_FACTOR = M2_P1_MINSPEED;
        #elif ACTIVE_PROFILE == 2
            constexpr float TARGET_RPM = M2_P2_RPM;
            constexpr float ACCEL_ZONE = M2_P2_ACCEL;
            constexpr float DECEL_ZONE = M2_P2_DECEL;
            constexpr float POWER_CURVE = M2_P2_CURVE;
            constexpr float MIN_SPEED_FACTOR = M2_P2_MINSPEED;
        #elif ACTIVE_PROFILE == 3
            constexpr float TARGET_RPM = M2_P3_RPM;
            constexpr float ACCEL_ZONE = M2_P3_ACCEL;
            constexpr float DECEL_ZONE = M2_P3_DECEL;
            constexpr float POWER_CURVE = M2_P3_CURVE;
            constexpr float MIN_SPEED_FACTOR = M2_P3_MINSPEED;
        #endif
    }
    
    // Timing
    namespace Timing {
        constexpr unsigned long DIR_CHANGE_DELAY_MS = 50;   // Delay after direction change before movement (motor settling time)
        constexpr unsigned long DIR_SETUP_US = 5;           // Direction signal setup time in microseconds (driver requirement)
        constexpr unsigned long DEBOUNCE_MS = 5;            // Limit switch debounce time in milliseconds
        constexpr unsigned long STATE_PAUSE_MS = 500;       // Pause between homing state transitions (allows motor to settle)
        constexpr unsigned long HOMING_PAUSE_MS = 100;      // Pause during homing operations (not currently used)
    }
    
    // Serial Communication
    namespace Serial {
        constexpr unsigned long BAUD_RATE = 115200;  // Serial port baud rate (bits per second)
    }
    
    // Sequence Behavior
    namespace Sequence {
        constexpr bool AUTO_START_AFTER_HOMING = true;      // If true, seq1 starts automatically after Motor2 homing completes
        
        // Motor2 trigger positions (symmetric around Motor1 oscillation range)
        // Loaded from SpeedProfiles.h based on ACTIVE_PROFILE
        #if ACTIVE_PROFILE == 1
            constexpr float MOTOR2_TRIGGER_OFFSET = M2_P1_TRIGGER_OFFSET;
        #elif ACTIVE_PROFILE == 2
            constexpr float MOTOR2_TRIGGER_OFFSET = M2_P2_TRIGGER_OFFSET;
        #elif ACTIVE_PROFILE == 3
            constexpr float MOTOR2_TRIGGER_OFFSET = M2_P3_TRIGGER_OFFSET;
        #endif
        
        constexpr float MOTOR2_TRIGGER_HIGH = Motor1::MAX_DEGREES - MOTOR2_TRIGGER_OFFSET;
        constexpr float MOTOR2_TRIGGER_LOW = MOTOR2_TRIGGER_OFFSET;
    }
    
    // Homing Behavior
    namespace Homing {
        constexpr bool AUTO_START_ON_BOOT = true;            // If true, homing starts automatically on boot; if false, only on command
    }
    
    // FRAM Configuration
    namespace FRAM {
        constexpr uint8_t I2C_ADDRESS = 0x50;                 // MB85RC256V FRAM I2C address
        constexpr bool ENABLE_AUTO_RECOVERY = true;           // Auto-return to home position after power loss
        constexpr bool RECOVERY_BEFORE_HOMING = true;         // If true, Motor1 recovery happens before Motor2 homing
    }
}

#endif // CONFIG_H