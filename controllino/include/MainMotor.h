#ifndef MAIN_MOTOR_H
#define MAIN_MOTOR_H

#include "StepperMotor.h"
#include "Config.h"

class MainMotor : public StepperMotor {
private:
    // Speed profile variables
    float currentSpeedFactor;
    unsigned long accelEndStep;
    unsigned long decelStartStep;
    const unsigned long accelZoneSteps;   // Pre-calculated accel zone (based on 360°)
    const unsigned long decelZoneSteps;   // Pre-calculated decel zone (based on 360°)
    
    // Position tracking for FRAM
    long lastSavedPosition;          // Last position written to FRAM
    unsigned long stepsSinceLastSave; // Steps since last FRAM write
    
    // Calculate acceleration/deceleration factors
    float getAccelFactor(unsigned long currentStep) const {
        if (accelEndStep == 0) return 1.0f;
        float progress = (float)currentStep / (float)accelEndStep;
        float factor = pow(progress, Config::Motor1::POWER_CURVE);
        return max(factor, Config::Motor1::MIN_SPEED_FACTOR);
    }
    
    float getDecelFactor(unsigned long currentStep) const {
        if (currentStep <= decelStartStep) return 1.0f;
        unsigned long remainingSteps = totalSteps - currentStep;
        unsigned long decelSteps = totalSteps - decelStartStep;
        if (decelSteps == 0) return 1.0f;
        float progress = (float)remainingSteps / (float)decelSteps;
        float factor = pow(progress, Config::Motor1::POWER_CURVE);
        return max(factor, Config::Motor1::MIN_SPEED_FACTOR);
    }
    
public:
    MainMotor() 
        : StepperMotor(Config::Motor1::STEP_PIN, Config::Motor1::DIR_PIN,
                       Config::Motor1::STEPS_PER_REV, Config::Motor1::MICROSTEPS,
                       Config::Motor1::GEAR_RATIO, Config::Motor1::TARGET_RPM),
          currentSpeedFactor(1.0f), accelEndStep(0), decelStartStep(0),
          accelZoneSteps((unsigned long)(Config::Motor1::GEAR_RATIO * Config::Motor1::STEPS_PER_REV * Config::Motor1::MICROSTEPS * Config::Motor1::ACCEL_ZONE)),
          decelZoneSteps((unsigned long)(Config::Motor1::GEAR_RATIO * Config::Motor1::STEPS_PER_REV * Config::Motor1::MICROSTEPS * Config::Motor1::DECEL_ZONE)),
          lastSavedPosition(0), stepsSinceLastSave(0) {}
    
    // Calculate total steps for given degrees
    unsigned long calculateSteps(float degrees) const {
        return (unsigned long)((degrees / 360.0f) * gearRatio * stepsPerRev * microsteps);
    }
    
    // Convert steps to degrees
    float stepsToDegrees(long steps) const {
        return ((float)steps / (float)(gearRatio * stepsPerRev * microsteps)) * 360.0f;
    }
    
    // Check if movement would exceed limits
    bool checkPositionLimits(float degrees, bool direction) {
        long stepsToMove = (long)calculateSteps(abs(degrees));
        long predictedPosition = currentPosition + (direction ? stepsToMove : -stepsToMove);
        float predictedDegrees = stepsToDegrees(predictedPosition);
        
        // Check against soft limit (warning)
        if (abs(predictedDegrees) > Config::Motor1::SOFT_LIMIT_DEGREES) {
            Serial.print(F("WARNING: Approaching cable limit! Current: "));
            Serial.print(stepsToDegrees(currentPosition), 1);
            Serial.print(F("°, After move: "));
            Serial.print(predictedDegrees, 1);
            Serial.println(F("°"));
        }
        
        // Check against hard limit (block)
        if (abs(predictedDegrees) > Config::Motor1::MAX_DEGREES) {
            Serial.print(F("ERROR: Movement blocked! Would exceed "));
            Serial.print(Config::Motor1::MAX_DEGREES);
            Serial.print(F("° limit. Current: "));
            Serial.print(stepsToDegrees(currentPosition), 1);
            Serial.print(F("°, Requested: "));
            Serial.print(predictedDegrees, 1);
            Serial.println(F("°"));
            Serial.println(F("Use 'gotohome1' to return to zero position"));
            return false;
        }
        
        return true;
    }
    
    // Start movement with speed profiling
    bool startMovement(float degrees, bool checkLimits = true) {
        // Safety check: limit maximum rotation
        if (degrees > Config::Motor1::MAX_DEGREES) {
            Serial.print(F("Error: Motor1 rotation limited to "));
            Serial.print(Config::Motor1::MAX_DEGREES);
            Serial.println(F("° (3 rotations max)"));
            degrees = Config::Motor1::MAX_DEGREES;
        }
        
        // Check position limits (unless override for homing)
        if (checkLimits && !checkPositionLimits(degrees, currentDirection)) {
            return false;  // Movement blocked
        }
        
        totalSteps = calculateSteps(degrees);
        
        // Use pre-calculated accel/decel zones (relative to 360°)
        // This keeps acceleration consistent regardless of movement distance
        accelEndStep = accelZoneSteps;
        decelStartStep = (totalSteps > decelZoneSteps) ? (totalSteps - decelZoneSteps) : 0;
        
        stepCount = 0;
        currentSpeedFactor = Config::Motor1::MIN_SPEED_FACTOR; // Start at minimum speed
        enabled = true;
        return true;
    }
    
    // Update speed profile (call from main loop)
    float updateSpeedProfile() {
        if (!enabled) return 1.0f;
        
        unsigned long currentStep = stepCount;
        float accelFactor = getAccelFactor(currentStep);
        float decelFactor = getDecelFactor(currentStep);
        currentSpeedFactor = min(accelFactor, decelFactor);
        
        return currentSpeedFactor;
    }
    
    // Get current speed factor
    float getSpeedFactor() const {
        return currentSpeedFactor;
    }
    
    // Check if movement complete
    bool isMovementComplete() const {
        return !enabled && stepCount >= totalSteps;
    }
    
    // === Position Management ===
    
    // Check if position should be saved (every ~0.5°)
    bool shouldSavePosition() const {
        long delta = abs(currentPosition - lastSavedPosition);
        return delta >= Config::Motor1::FRAM_UPDATE_INTERVAL_STEPS;
    }
    
    // Mark position as saved
    void markPositionSaved() {
        lastSavedPosition = currentPosition;
        clearPositionChanged();
    }
    
    // Get position in degrees
    float getPositionDegrees() const {
        return stepsToDegrees(currentPosition);
    }
    
    // Get remaining rotation capacity
    void printPositionInfo() const {
        float currentDeg = stepsToDegrees(currentPosition);
        float remainingCW = Config::Motor1::MAX_DEGREES - currentDeg;
        float remainingCCW = Config::Motor1::MAX_DEGREES + currentDeg;
        
        Serial.print(F("Motor1 Position: "));
        Serial.print(currentPosition);
        Serial.print(F(" steps = "));
        Serial.print(currentDeg, 2);
        Serial.println(F("°"));
        
        Serial.print(F("  Remaining CW:  "));
        Serial.print(remainingCW, 1);
        Serial.println(F("°"));
        
        Serial.print(F("  Remaining CCW: "));
        Serial.print(remainingCCW, 1);
        Serial.println(F("°"));
        
        if (abs(currentDeg) > Config::Motor1::SOFT_LIMIT_DEGREES) {
            Serial.println(F("  ⚠️  WARNING: Near cable limit!"));
        }
    }
};

#endif // MAIN_MOTOR_H
