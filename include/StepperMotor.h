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
    unsigned long totalSteps;
    float stepFreq;
    
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
          stepCount(0), stepLevel(false), enabled(false), totalSteps(0) {
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
            if (!stepLevel) stepCount++;
        } else {
            enabled = false;
            stepLevel = false;
        }
    }
    
    // Control methods
    void setDirection(bool dirHigh) {
        digitalWrite(dirPin, dirHigh ? HIGH : LOW);
    }
    
    void enable() {
        enabled = true;
    }
    
    void disable() {
        enabled = false;
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
    
    // Calculate timer period in microseconds
    unsigned long getTimerPeriod() const {
        return (unsigned long)(500000.0f / stepFreq);
    }
};

#endif // STEPPER_MOTOR_H
