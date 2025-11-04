#ifndef STEPPER_MOTOR_H
#define STEPPER_MOTOR_H

#include <Arduino.h>

class StepperMotor {
protected:
    // Pin configuration
    const uint8_t stepPin;
    const uint8_t dirPin;
    
    // Motor parameters (const to save RAM)
    const uint16_t stepsPerRev;
    const uint8_t microsteps;
    const uint8_t gearRatio;
    const float targetRPM;
    
    // State variables
    volatile unsigned long stepCount;
    volatile bool stepLevel;
    volatile bool enabled;
    volatile bool currentDirection;  // Current movement direction (CW/CCW)
    unsigned long totalSteps;
    float stepFreq;
    float speedMultiplier;  // Global speed adjustment (default 1.0)
    
    // Position tracking (for persistent storage)
    volatile long currentPosition;      // Signed position: + = CW, - = CCW
    volatile bool positionChanged;      // Flag for async FRAM update
    
    // Calculate step frequency
    void calculateStepFreq() {
        float motorRPM = targetRPM * gearRatio;
        stepFreq = (motorRPM / 60.0f) * stepsPerRev * microsteps;
    }
    
public:
    StepperMotor(uint8_t step, uint8_t dir, 
                 uint16_t spr, uint8_t ms, uint8_t gr, float rpm)
        : stepPin(step), dirPin(dir), 
          stepsPerRev(spr), microsteps(ms), gearRatio(gr), targetRPM(rpm),
          stepCount(0), stepLevel(false), enabled(false), currentDirection(true),
          totalSteps(0), speedMultiplier(1.0f), currentPosition(0), positionChanged(false) {
        calculateStepFreq();
    }
    
    virtual ~StepperMotor() {}
    
    // Initialize pins
    virtual void init() {
        pinMode(stepPin, OUTPUT);
        pinMode(dirPin, OUTPUT);
        digitalWrite(dirPin, HIGH);
    }
    
    // ISR callback - must be fast!
    virtual void step() {
        if (enabled && stepCount < totalSteps) {
            stepLevel = !stepLevel;
            digitalWrite(stepPin, stepLevel);
            if (!stepLevel) {
                stepCount++;
                // Update position: CW = +1, CCW = -1
                currentPosition += currentDirection ? 1 : -1;
                positionChanged = true;  // Flag for main loop
            }
        } else {
            enabled = false;
            stepLevel = false;
        }
    }
    
    // Control methods
    void setDirection(bool dirHigh) {
        digitalWrite(dirPin, dirHigh ? HIGH : LOW);
        currentDirection = dirHigh;  // Track for position calculation
    }
    
    void enable() {
        enabled = true;
    }
    
    void disable() {
        enabled = false;
    }
    
    void stopMovement() {
        disable();  // Alias for clarity
    }
    
    void resetStepCount() {
        stepCount = 0;
    }
    
    void setTotalSteps(unsigned long steps) {
        totalSteps = steps;
    }
    
    // Getters (inline for performance)
    inline bool isEnabled() const { return enabled; }
    inline unsigned long getStepCount() const { return stepCount; }
    inline unsigned long getTotalSteps() const { return totalSteps; }
    inline float getStepFreq() const { return stepFreq; }
    
    // Position tracking
    inline long getPosition() const { return currentPosition; }
    inline bool hasPositionChanged() const { return positionChanged; }
    inline void clearPositionChanged() { positionChanged = false; }
    inline void setPosition(long pos) { 
        currentPosition = pos; 
        positionChanged = true;
    }
    
    // Calculate timer period in microseconds
    unsigned long getTimerPeriod() const {
        return (unsigned long)(500000.0f / (stepFreq * speedMultiplier));
    }
    
    void setSpeedMultiplier(float multiplier) {
        speedMultiplier = max(0.1f, min(10.0f, multiplier));  // Clamp 0.1x - 10x
    }
    
    float getSpeedMultiplier() const {
        return speedMultiplier;
    }
};

#endif // STEPPER_MOTOR_H
