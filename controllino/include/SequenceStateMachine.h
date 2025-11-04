#ifndef SEQUENCE_STATE_MACHINE_H
#define SEQUENCE_STATE_MACHINE_H

#include "MainMotor.h"
#include "OscillationMotor.h"
#include "Config.h"

enum class SequenceState {
    IDLE,
    MOTOR1_TO_MAX_CW,
    MOTOR1_TO_ZERO_CCW
};

class SequenceStateMachine {
private:
    MainMotor& motor1;
    OscillationMotor& motor2;
    SequenceState currentState;
    
    bool motor2TriggeredHigh;
    bool motor2TriggeredLow;
    
    float motor1StartTriggerHigh;
    float motor1StartTriggerLow;
    
    // Calculate Motor2 speed adjustment to sync with Motor1 overlap timing
    // Uses trapezoidal speed profile integration for accurate timing
    float calculateMotor2SpeedFactor() {
        // ═══════════════════════════════════════════════════════════════
        // MOTOR1 TIME CALCULATION WITH ACCEL/DECEL INTEGRATION
        // ═══════════════════════════════════════════════════════════════
        
        // 1. Motor1 travels 2 × OVERLAP_DEGREES during one Motor2 half-oscillation
        float motor1OverlapDistance = 2.0f * Config::Sequence::MOTOR2_OVERLAP_DEGREES;
        // Example: 2 × 20° = 40°
        
        // 2. Motor2's oscillation distance (measured during homing)
        unsigned long motor2TotalSteps = motor2.getOscillationSteps();  // Use actual oscillation distance with safety margins
        float motor2OscillationDegrees = motor2.stepsToDegrees(motor2TotalSteps);
        
        // 3. Define Motor1 Accel/Decel zones (relative to 360° reference)
        float accelZoneDegrees = 360.0f * Config::Motor1::ACCEL_ZONE;  // 360° × 0.05 = 18°
        float decelZoneDegrees = 360.0f * Config::Motor1::DECEL_ZONE;  // 360° × 0.05 = 18°
        
        // 4. Calculate constant-speed zone
        // If overlap < accel+decel, the movement is all ramping (no constant zone)
        float constantZoneDegrees = 0.0f;
        if (motor1OverlapDistance > (accelZoneDegrees + decelZoneDegrees)) {
            constantZoneDegrees = motor1OverlapDistance - accelZoneDegrees - decelZoneDegrees;
        }
        // Example: 40° > 36° → constantZone = 40° - 18° - 18° = 4°
        
        // 5. Average speed in ramping zones
        // Approximation: triangular profile from MIN (0.1x) to MAX (1.0x)
        // Average = (0.1 + 1.0) / 2 = 0.55
        float avgRampSpeed = (Config::Motor1::MIN_SPEED_FACTOR + 1.0f) / 2.0f;
        
        // 6. Time integration for each zone
        // Time = Distance / Speed
        // Time = (degrees/360°) / (RPM/60s × speedFactor)
        
        float timeAccel = (accelZoneDegrees / 360.0f) / 
                          ((Config::Motor1::TARGET_RPM / 60.0f) * avgRampSpeed);
        // Example: (18°/360°) / (3.0 RPM/60s × 0.55) = 0.05 / 0.0275 = 1.82s
        
        float timeDecel = (decelZoneDegrees / 360.0f) / 
                          ((Config::Motor1::TARGET_RPM / 60.0f) * avgRampSpeed);
        // Example: (18°/360°) / (3.0 RPM/60s × 0.55) = 1.82s
        
        float timeConstant = (constantZoneDegrees / 360.0f) / 
                             (Config::Motor1::TARGET_RPM / 60.0f);
        // Example: (4°/360°) / (3.0 RPM/60s) = 0.0111 / 0.05 = 0.22s
        
        // 7. Total time for Motor1 overlap movement
        float motor1TimeSeconds = timeAccel + timeConstant + timeDecel;
        // Example: 1.82s + 0.22s + 1.82s = 3.86 seconds
        
        // ═══════════════════════════════════════════════════════════════
        // MOTOR2 RPM CALCULATION WITH ACCEL/DECEL
        // ═══════════════════════════════════════════════════════════════
        
        // 8. Motor2 must travel HALF its oscillation in the same time
        float motor2HalfDistance = motor2OscillationDegrees / 2.0f;
        // Example with 25° total: 25° / 2 = 12.5°
        
        // 9. Motor2 Accel/Decel zones (also relative to 360° reference)
        float motor2AccelZone = 360.0f * Config::Motor2::ACCEL_ZONE;  // 360° × 0.10 = 36°
        float motor2DecelZone = 360.0f * Config::Motor2::DECEL_ZONE;  // 360° × 0.10 = 36°
        float motor2ConstantZone = max(0.0f, motor2HalfDistance - motor2AccelZone - motor2DecelZone);
        // Example: 12.5° < 72° → motor2ConstantZone = 0° (only ramping!)
        
        float motor2AvgRampSpeed = (Config::Motor2::MIN_SPEED_FACTOR + 1.0f) / 2.0f;
        
        // 10. Solve for required Motor2 RPM
        // Given: motor1TimeSeconds (e.g., 3.86s)
        // Find: Motor2 RPM such that Time_Motor2 = motor1TimeSeconds
        //
        // Time_Motor2 = (AccelZone/360° / AvgSpeed + ConstantZone/360°) / (RPM/60s)
        //
        // Solving for RPM:
        // RPM = (AccelZone/360° / AvgSpeed + ConstantZone/360°) / (Time/60s)
        
        float motor2RequiredRPM = 
            ((motor2AccelZone + motor2DecelZone) / (360.0f * motor2AvgRampSpeed) + 
             motor2ConstantZone / 360.0f) / (motor1TimeSeconds / 60.0f);
        
        // Example with 12.5° Motor2:
        // = ((36° + 36°) / (360° × 0.55) + 0° / 360°) / (3.86s / 60s)
        // = (72° / 198°) / 0.0643
        // = 0.364 / 0.0643
        // = 0.566 RPM
        
        // 11. Calculate speed factor (multiplier relative to base speed)
        float speedFactor = motor2RequiredRPM / Config::Motor2::TARGET_RPM;
        // Example: 0.566 / 0.5 = 1.13x
        
        // 12. Safety check: Clamp to reasonable limits (max 20x faster)
        if (speedFactor > 20.0f) {
            Serial.println(F("WARNING: Motor2 speed factor too high, clamping to 20x"));
            speedFactor = 20.0f;
        }
        
        return speedFactor;
    }
    
public:
    SequenceStateMachine(MainMotor& m1, OscillationMotor& m2)
        : motor1(m1), motor2(m2), currentState(SequenceState::IDLE),
          motor2TriggeredHigh(false), motor2TriggeredLow(false),
          motor1StartTriggerHigh(0.0f), motor1StartTriggerLow(0.0f) {}
    
    void start() {
        if (!motor2.isHomingComplete()) {
            Serial.println(F("Error: Motor 2 not homed. Run 'home' command first!"));
            return;
        }
        
        motor1StartTriggerHigh = Config::Sequence::MOTOR1_MAX_DEGREES - Config::Sequence::MOTOR2_OVERLAP_DEGREES;
        motor1StartTriggerLow = Config::Sequence::MOTOR2_OVERLAP_DEGREES;
        
        // Calculate and set Motor2 speed synchronization
        float speedFactor = calculateMotor2SpeedFactor();
        motor2.setSpeedMultiplier(speedFactor);
        
        // Debug output
        unsigned long motor2Steps = motor2.getOscillationSteps();  // Use actual oscillation distance
        float motor2Degrees = motor2.stepsToDegrees(motor2Steps);
        float motor2HalfDegrees = motor2Degrees / 2.0f;
        float motor2ResultRPM = Config::Motor2::TARGET_RPM * speedFactor;
        
        Serial.println(F("=== Synchronized Sequence Started ==="));
        Serial.print(F("Motor1: 0\xF8 <--> "));
        Serial.print(Config::Sequence::MOTOR1_MAX_DEGREES, 0);
        Serial.print(F("\xF8 @ "));
        Serial.print(Config::Motor1::TARGET_RPM, 1);
        Serial.println(F(" RPM"));
        
        Serial.print(F("Motor2 range: "));
        Serial.print(motor2Degrees, 1);
        Serial.print(F("\xF8 (half = "));
        Serial.print(motor2HalfDegrees, 1);
        Serial.println(F("\xF8)"));
        
        Serial.print(F("Motor2 overlap: "));
        Serial.print(Config::Sequence::MOTOR2_OVERLAP_DEGREES, 0);
        Serial.println(F("\xF8 (at each end)"));
        
        Serial.print(F("Motor2 base speed: "));
        Serial.print(Config::Motor2::TARGET_RPM, 1);
        Serial.print(F(" RPM -> "));
        Serial.print(motor2ResultRPM, 2);
        Serial.print(F(" RPM ("));
        Serial.print(speedFactor, 2);
        Serial.println(F("x)"));
        
        motor1.setDirection(Config::CW_RIGHT);
        delay(Config::Timing::DIR_CHANGE_DELAY_MS);
        delayMicroseconds(Config::Timing::DIR_SETUP_US);
        motor1.startMovement(Config::Sequence::MOTOR1_MAX_DEGREES, false);
        
        currentState = SequenceState::MOTOR1_TO_MAX_CW;
        motor2TriggeredHigh = false;
        motor2TriggeredLow = false;
        
        Serial.println(F("Phase 1: Motor1 -> 720\xF8 CW"));
    }
    
    void stop() {
        if (currentState == SequenceState::IDLE) {
            Serial.println(F("Sequence not running"));
            return;
        }
        
        Serial.println(F("Stopping sequence..."));
        motor1.stopMovement();
        motor2.stopOscillation();
        currentState = SequenceState::IDLE;
        motor2TriggeredHigh = false;
        motor2TriggeredLow = false;
        Serial.println(F("Sequence stopped"));
    }
    
    bool isActive() const {
        return currentState != SequenceState::IDLE;
    }
    
    void update() {
        if (currentState == SequenceState::IDLE) return;
        
        float motor1Position = motor1.getPositionDegrees();
        
        switch (currentState) {
            case SequenceState::MOTOR1_TO_MAX_CW:
                // Motor2 starts at 680° and runs CONTINUOUSLY through Motor1's direction change
                if (!motor2TriggeredHigh && motor1Position >= motor1StartTriggerHigh) {
                    Serial.print(F("Motor2 START continuous movement at Motor1="));
                    Serial.print(motor1Position, 1);
                    Serial.println(F("\xF8"));
                    motor2.startOscillation(false);  // Full continuous movement LEFT
                    motor2TriggeredHigh = true;
                }
                
                if (motor1.isMovementComplete()) {
                    Serial.println(F("Phase 2: Motor1 -> 0\xF8 CCW"));
                    Serial.println(F("Motor2 continues running (no restart)"));
                    
                    motor1.setDirection(Config::CCW_LEFT);
                    delay(Config::Timing::DIR_CHANGE_DELAY_MS);
                    delayMicroseconds(Config::Timing::DIR_SETUP_US);
                    motor1.startMovement(Config::Sequence::MOTOR1_MAX_DEGREES, false);
                    
                    currentState = SequenceState::MOTOR1_TO_ZERO_CCW;
                    motor2TriggeredLow = false;
                }
                break;
                
            case SequenceState::MOTOR1_TO_ZERO_CCW:
                // Motor2 continues running - no new start needed!
                // Just check if it has completed its oscillation
                if (!motor2TriggeredLow && motor1Position <= motor1StartTriggerLow) {
                    Serial.print(F("Motor1 passed 40\xF8, Motor2 should be finishing at Motor1="));
                    Serial.print(motor1Position, 1);
                    Serial.println(F("\xF8"));
                    motor2TriggeredLow = true;
                }
                
                if (motor1.isMovementComplete()) {
                    Serial.println(F("Phase 1: Motor1 -> 720\xF8 CW"));
                    
                    motor1.setDirection(Config::CW_RIGHT);
                    delay(Config::Timing::DIR_CHANGE_DELAY_MS);
                    delayMicroseconds(Config::Timing::DIR_SETUP_US);
                    motor1.startMovement(Config::Sequence::MOTOR1_MAX_DEGREES, false);
                    
                    currentState = SequenceState::MOTOR1_TO_MAX_CW;
                    motor2TriggeredHigh = false;
                }
                break;
                
            case SequenceState::IDLE:
                break;
        }
    }
};

#endif
