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
          decelZoneSteps((unsigned long)(Config::Motor1::GEAR_RATIO * Config::Motor1::STEPS_PER_REV * Config::Motor1::MICROSTEPS * Config::Motor1::DECEL_ZONE)) {}
    
    // Calculate total steps for given degrees
    unsigned long calculateSteps(float degrees) const {
        return (unsigned long)((degrees / 360.0f) * gearRatio * stepsPerRev * microsteps);
    }
    
    // Start movement with speed profiling
    void startMovement(float degrees) {
        // Safety check: limit maximum rotation
        if (degrees > Config::Motor1::MAX_DEGREES) {
            Serial.print(F("Error: Motor1 rotation limited to "));
            Serial.print(Config::Motor1::MAX_DEGREES);
            Serial.println(F("° (3 rotations max)"));
            degrees = Config::Motor1::MAX_DEGREES;
        }
        
        totalSteps = calculateSteps(degrees);
        
        // Use pre-calculated accel/decel zones (relative to 360°)
        // This keeps acceleration consistent regardless of movement distance
        accelEndStep = accelZoneSteps;
        decelStartStep = (totalSteps > decelZoneSteps) ? (totalSteps - decelZoneSteps) : 0;
        
        stepCount = 0;
        currentSpeedFactor = Config::Motor1::MIN_SPEED_FACTOR; // Start at minimum speed
        enabled = true;
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
};

#endif // MAIN_MOTOR_H
