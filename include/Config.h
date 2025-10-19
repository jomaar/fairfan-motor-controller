#ifndef CONFIG_H
#define CONFIG_H

#include <Arduino.h>
#include <Controllino.h>

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
        constexpr float TARGET_RPM = 15.0f;          // Target speed at output shaft (after gear reduction)
        constexpr float TEST_DEGREES = 180.0f;       // Movement angle for 'go1' test command (half rotation)
        constexpr float SEQUENCE_DEGREES = 720.0f;   // Movement angle for seq1 oscillation (2 full rotations)
        // Speed Profile (calculated relative to 360° rotation for consistent acceleration)
        constexpr float ACCEL_ZONE = 0.05f;          // Acceleration zone (5% of 360° = 18°)
        constexpr float DECEL_ZONE = 0.05f;          // Deceleration zone (5% of 360° = 18°)
        constexpr float POWER_CURVE = 0.8f;          // Power curve exponent for acceleration/deceleration profile (0.8 = gentle curve)
        constexpr float MIN_SPEED_FACTOR = 0.1f;     // Minimum speed as fraction of target speed (0.1 = 10% minimum to prevent stalling)
    }
    
    // Motor 2 Parameters (Oscillation Motor)
    namespace Motor2 {
        constexpr uint8_t STEP_PIN = 7;                          // Digital pin for step pulses
        constexpr uint8_t DIR_PIN = 8;                           // Digital pin for direction control
        constexpr uint8_t ENABLE_PIN = 9;                        // Enable/disable motor driver (LOW=enabled)
        constexpr uint8_t LEFT_SWITCH_PIN = CONTROLLINO_DI0;     // Limit switch at left position (NC = normally closed)
        constexpr uint8_t RIGHT_SWITCH_PIN = CONTROLLINO_DI1;    // Limit switch at right position (NC = normally closed)
        constexpr uint16_t STEPS_PER_REV = 200;                  // Full steps per revolution (1.8° step angle motor)
        constexpr uint8_t MICROSTEPS = 8;                        // Microstepping driver setting (1/8 step)
        constexpr uint8_t GEAR_RATIO = 50;                       // Gear reduction ratio (50:1)
        constexpr float TARGET_RPM = 5.0f;                       // Target speed at output shaft (after gear reduction)
        constexpr float OFFSET_DEGREES = 10.0f;                  // Offset from right limit switch after homing (safety margin)
        // Speed Profile (calculated relative to 360° rotation for consistent acceleration)
        constexpr float ACCEL_ZONE = 0.10f;          // Acceleration zone (10% of 360° = 36°)
        constexpr float DECEL_ZONE = 0.10f;          // Deceleration zone (10% of 360° = 36°)
        constexpr float POWER_CURVE = 0.8f;          // Power curve exponent for acceleration/deceleration profile (0.8 = gentle curve)
        constexpr float MIN_SPEED_FACTOR = 0.1f;     // Minimum speed as fraction of target speed (0.1 = 10% minimum to prevent stalling)
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
        constexpr bool MOTOR1_SAME_DIR_AS_MOTOR2 = true;     // If true, Motor1 moves same direction as Motor2; if false, opposite direction
    }
}

#endif // CONFIG_H
