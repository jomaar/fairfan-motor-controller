#ifndef OSCILLATION_MOTOR_H
#define OSCILLATION_MOTOR_H

#include "StepperMotor.h"
#include "Config.h"
#include <Bounce2.h>

enum class HomingState {
    IDLE,
    MOVE_LEFT,
    MOVE_RIGHT,
    OFFSET,
    COMPLETE
};

class OscillationMotor : public StepperMotor {
private:
    // Limit switches
    Bounce leftSwitch;
    Bounce rightSwitch;
    
    // Homing state machine
    HomingState homingState;
    unsigned long homeRangeSteps;
    unsigned long offsetSteps;
    
    // Speed profile variables
    float currentSpeedFactor;
    unsigned long accelEndStep;
    unsigned long decelStartStep;
    bool speedProfileEnabled;
    const unsigned long accelZoneSteps;   // Pre-calculated accel zone (based on 360°)
    const unsigned long decelZoneSteps;   // Pre-calculated decel zone (based on 360°)
    
    // Position tracking
    long currentPosition;
    bool isHomed;
    
    // Calculate acceleration/deceleration factors
    float getAccelFactor(unsigned long currentStep) const {
        if (!speedProfileEnabled || accelEndStep == 0) return 1.0f;
        float progress = (float)currentStep / (float)accelEndStep;
        float factor = pow(progress, Config::Motor2::POWER_CURVE);
        return max(factor, Config::Motor2::MIN_SPEED_FACTOR);
    }
    
    float getDecelFactor(unsigned long currentStep) const {
        if (!speedProfileEnabled || currentStep <= decelStartStep) return 1.0f;
        unsigned long remainingSteps = totalSteps - currentStep;
        unsigned long decelSteps = totalSteps - decelStartStep;
        if (decelSteps == 0) return 1.0f;
        float progress = (float)remainingSteps / (float)decelSteps;
        float factor = pow(progress, Config::Motor2::POWER_CURVE);
        return max(factor, Config::Motor2::MIN_SPEED_FACTOR);
    }
    
public:
    OscillationMotor() 
        : StepperMotor(Config::Motor2::STEP_PIN, Config::Motor2::DIR_PIN,
                       Config::Motor2::STEPS_PER_REV, Config::Motor2::MICROSTEPS,
                       Config::Motor2::GEAR_RATIO, Config::Motor2::TARGET_RPM),
          leftSwitch(), rightSwitch(),
          homingState(HomingState::IDLE), homeRangeSteps(0), offsetSteps(0),
          currentSpeedFactor(1.0f), accelEndStep(0), decelStartStep(0),
          speedProfileEnabled(false),
          accelZoneSteps((unsigned long)(Config::Motor2::GEAR_RATIO * Config::Motor2::STEPS_PER_REV * Config::Motor2::MICROSTEPS * Config::Motor2::ACCEL_ZONE)),
          decelZoneSteps((unsigned long)(Config::Motor2::GEAR_RATIO * Config::Motor2::STEPS_PER_REV * Config::Motor2::MICROSTEPS * Config::Motor2::DECEL_ZONE)),
          currentPosition(0), isHomed(false) {}
    
    void init() override {
        StepperMotor::init();
        
        // Setup limit switches (normally closed) with pull-up resistors
        pinMode(Config::Motor2::LEFT_SWITCH_PIN, INPUT_PULLUP);
        pinMode(Config::Motor2::RIGHT_SWITCH_PIN, INPUT_PULLUP);
        
        // Setup Bounce objects for debouncing
        leftSwitch.attach(Config::Motor2::LEFT_SWITCH_PIN);
        leftSwitch.interval(Config::Timing::DEBOUNCE_MS);
        rightSwitch.attach(Config::Motor2::RIGHT_SWITCH_PIN);
        rightSwitch.interval(Config::Timing::DEBOUNCE_MS);
        
        // Calculate offset in steps (10 degrees)
        offsetSteps = (unsigned long)((Config::Motor2::OFFSET_DEGREES / 360.0f) 
                                      * gearRatio * stepsPerRev * microsteps);
    }
    
    void updateSwitches() {
        leftSwitch.update();
        rightSwitch.update();
    }
    
    // Limit switch state (NC: pressed = LOW)
    bool isLeftSwitchPressed() const {
        return leftSwitch.read() == LOW;
    }
    
    bool isRightSwitchPressed() const {
        return rightSwitch.read() == LOW;
    }
    
    // Homing state machine
    void startHoming() {
        // Force stop any ongoing movement first
        stopMovement();
        
        homingState = HomingState::MOVE_LEFT;  // Start by moving to LEFT switch first
        homeRangeSteps = 0;
        isHomed = false;
        speedProfileEnabled = false; // Disable during homing
        Serial.println(F("Homing Motor 2: Starting"));
    }
    
    void updateHoming() {
        switch (homingState) {
            case HomingState::IDLE:
                break;
                
            case HomingState::MOVE_LEFT:
                if (!enabled) {
                    // Check if already at LEFT switch - if so, move away first
                    if (isLeftSwitchPressed()) {
                        Serial.println(F("Homing Motor 2: Already at LEFT, moving away..."));
                        setDirection(Config::CCW_LEFT); // Move RIGHT first
                        delay(Config::Timing::DIR_CHANGE_DELAY_MS);
                        delayMicroseconds(Config::Timing::DIR_SETUP_US);
                        resetStepCount();
                        totalSteps = offsetSteps;  // Move away by offset distance
                        enabled = true;
                        delay(100);  // Wait a bit for movement to start
                        // Wait for movement to complete
                        while (enabled) {
                            // Motor will auto-disable when done
                            delay(10);
                        }
                        Serial.println(F("Homing Motor 2: Freed from LEFT switch"));
                    }
                    
                    Serial.println(F("Homing Motor 2: Moving to LEFT switch..."));
                    setDirection(Config::CW_RIGHT); // Inverted: CW signal for LEFT movement
                    delay(Config::Timing::DIR_CHANGE_DELAY_MS);
                    delayMicroseconds(Config::Timing::DIR_SETUP_US);
                    resetStepCount();
                    totalSteps = 0xFFFFFFFF; // Run until limit switch hit
                    enabled = true;
                }
                
                if (isLeftSwitchPressed()) {
                    enabled = false;
                    currentPosition = 0;
                    Serial.println(F("Homing Motor 2: Left limit reached"));
                    delay(500);
                    homingState = HomingState::MOVE_RIGHT;  // Next: move to RIGHT
                }
                break;
                
            case HomingState::MOVE_RIGHT:
                if (!enabled) {
                    Serial.println(F("Homing Motor 2: Moving to RIGHT switch..."));
                    setDirection(Config::CCW_LEFT); // Inverted: CCW signal for RIGHT movement
                    delay(Config::Timing::DIR_CHANGE_DELAY_MS);
                    delayMicroseconds(Config::Timing::DIR_SETUP_US);
                    resetStepCount();
                    totalSteps = 0xFFFFFFFF; // Run until limit switch hit
                    homeRangeSteps = 0;
                    enabled = true;
                }
                
                if (isRightSwitchPressed()) {
                    enabled = false;
                    homeRangeSteps = stepCount;  // Save the range
                    Serial.print(F("Homing Motor 2: Right limit reached, Range = "));
                    Serial.print(homeRangeSteps);
                    Serial.println(F(" steps"));
                    delay(500);
                    resetStepCount(); // Reset for offset movement
                    homingState = HomingState::OFFSET;  // Next: offset back to LEFT
                }
                break;
                
            case HomingState::OFFSET:
                if (enabled) {
                    // Offset movement is in progress, wait for completion
                    // ISR will auto-disable when stepCount >= totalSteps
                } else if (stepCount >= offsetSteps) {
                    // Offset movement completed
                    currentPosition = homeRangeSteps - offsetSteps;  // Position is 10° from right
                    Serial.print(F("Homing Motor 2: Offset complete, position = "));
                    Serial.println(currentPosition);
                    homingState = HomingState::COMPLETE;
                } else {
                    // Start offset movement (first time entering this state)
                    setDirection(Config::CW_RIGHT); // Inverted: CW signal for LEFT movement
                    delay(Config::Timing::DIR_CHANGE_DELAY_MS);
                    delayMicroseconds(Config::Timing::DIR_SETUP_US);
                    totalSteps = offsetSteps;
                    enabled = true;
                    Serial.print(F("Homing Motor 2: Moving offset "));
                    Serial.print(offsetSteps);
                    Serial.println(F(" steps to LEFT"));
                }
                break;
                
            case HomingState::COMPLETE:
                isHomed = true;
                speedProfileEnabled = true; // Enable after homing
                enabled = false; // Make sure motor is stopped
                resetStepCount();
                totalSteps = 0;
                Serial.println(F("Homing Motor 2: Complete!"));
                homingState = HomingState::IDLE;
                break;
        }
    }
    
    bool isHomingComplete() const {
        return isHomed;
    }
    
    HomingState getHomingState() const {
        return homingState;
    }
    
    // Invalidate homing (motor position unknown - must re-home before seq1)
    void invalidateHoming() {
        isHomed = false;
    }
    
    // Oscillation control
    void startOscillation(bool directionRight) {
        if (!isHomed) return;
        
        // Direction is inverted for this motor: RIGHT=CCW signal, LEFT=CW signal
        setDirection(directionRight ? Config::CCW_LEFT : Config::CW_RIGHT);
        delay(Config::Timing::DIR_CHANGE_DELAY_MS);
        delayMicroseconds(Config::Timing::DIR_SETUP_US);
        
        // FULL continuous movement: Motor2 moves the ENTIRE home range
        // This allows continuous movement through Motor1's direction change
        // Motor2 runs from one side to the other without stopping
        totalSteps = homeRangeSteps;
        
        // Use pre-calculated accel/decel zones (relative to 360°)
        // This keeps acceleration consistent regardless of movement distance
        accelEndStep = accelZoneSteps;
        decelStartStep = (totalSteps > decelZoneSteps) ? (totalSteps - decelZoneSteps) : 0;
        
        resetStepCount();
        currentSpeedFactor = Config::Motor2::MIN_SPEED_FACTOR;
        speedProfileEnabled = true;  // Enable speed ramping
        enabled = true;
    }
    
    // Half oscillation for synchronized sequence
    void startHalfOscillation(bool directionRight) {
        if (!isHomed) return;
        
        // Direction is inverted for this motor: RIGHT=CCW signal, LEFT=CW signal
        setDirection(directionRight ? Config::CCW_LEFT : Config::CW_RIGHT);
        delay(Config::Timing::DIR_CHANGE_DELAY_MS);
        delayMicroseconds(Config::Timing::DIR_SETUP_US);
        
        // HALF oscillation range
        totalSteps = (homeRangeSteps - (2 * offsetSteps) - 50) / 2;
        
        // Use pre-calculated accel/decel zones (relative to 360°)
        accelEndStep = accelZoneSteps;
        decelStartStep = (totalSteps > decelZoneSteps) ? (totalSteps - decelZoneSteps) : 0;
        
        resetStepCount();
        currentSpeedFactor = Config::Motor2::MIN_SPEED_FACTOR;
        enabled = true;
    }
    
    // Update speed profile (call from main loop)
    float updateSpeedProfile() {
        if (!speedProfileEnabled || !enabled) return 1.0f;
        
        unsigned long currentStep = stepCount;
        float accelFactor = getAccelFactor(currentStep);
        float decelFactor = getDecelFactor(currentStep);
        currentSpeedFactor = min(accelFactor, decelFactor);
        
        return currentSpeedFactor;
    }
    
    float getSpeedFactor() const {
        return currentSpeedFactor;
    }
    
    bool isMovementComplete() const {
        return !enabled && stepCount >= totalSteps;
    }
    
    void stopOscillation() {
        stopMovement();  // Alias for semantic clarity
    }
    
    unsigned long getTotalSteps() const {
        return totalSteps;
    }
    
    // Calculate the actual oscillation distance (with safety margins applied)
    // This is FULL continuous movement from one side to the other
    unsigned long getOscillationSteps() const {
        if (!isHomed) return 0;
        // Full distance: the complete measured home range
        return homeRangeSteps;
    }
    
    float stepsToDegrees(unsigned long steps) const {
        return ((float)steps / (float)(gearRatio * stepsPerRev * microsteps)) * 360.0f;
    }
    
    long getPosition() const {
        return currentPosition;
    }
};

#endif // OSCILLATION_MOTOR_H
