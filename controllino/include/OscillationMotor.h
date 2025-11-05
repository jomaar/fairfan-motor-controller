#ifndef OSCILLATION_MOTOR_H
#define OSCILLATION_MOTOR_H

#include "StepperMotor.h"
#include "Config.h"
#include <Bounce2.h>

enum class HomingState {
    IDLE,
    MOVE_LEFT,
    OFFSET_LEFT,
    MOVE_RIGHT,
    OFFSET_RIGHT,
    COMPLETE
};

class OscillationMotor : public StepperMotor {
private:
    // Limit switches
    Bounce leftSwitch;
    Bounce rightSwitch;
    
    // Homing state machine
    HomingState homingState;
    unsigned long homeRangeSteps;      // Usable oscillation range (between offsets)
    unsigned long totalRangeSteps;     // Total range (switch to switch)
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
          homingState(HomingState::IDLE), homeRangeSteps(0), totalRangeSteps(0), offsetSteps(0),
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
        
        homingState = HomingState::MOVE_RIGHT;  // Start by moving to RIGHT switch first
        homeRangeSteps = 0;
        isHomed = false;
        speedProfileEnabled = false; // Disable during homing
        DEBUG_PRINTLN(F("Homing Motor 2: Starting"));
    }
    
    void updateHoming() {
        switch (homingState) {
            case HomingState::IDLE:
                break;
                
            case HomingState::MOVE_RIGHT:
                if (!enabled) {
                    // Check if already at RIGHT switch - if so, move away first
                    if (isRightSwitchPressed()) {
                        DEBUG_PRINTLN(F("Homing Motor 2: Already at RIGHT, moving away..."));
                        setDirection(Config::CW_RIGHT); // Move LEFT first
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
                        DEBUG_PRINTLN(F("Homing Motor 2: Freed from RIGHT switch"));
                    }
                    
                    DEBUG_PRINTLN(F("Homing Motor 2: Moving to RIGHT switch..."));
                    setDirection(Config::CCW_LEFT); // Inverted: CCW signal for RIGHT movement
                    delay(Config::Timing::DIR_CHANGE_DELAY_MS);
                    delayMicroseconds(Config::Timing::DIR_SETUP_US);
                    resetStepCount();
                    totalSteps = 0xFFFFFFFF; // Run until limit switch hit
                    enabled = true;
                }
                
                if (isRightSwitchPressed()) {
                    enabled = false;
                    currentPosition = 0;  // RIGHT is position 0 for now
                    DEBUG_PRINTLN(F("Homing Motor 2: Right limit reached"));
                    delay(500);
                    resetStepCount(); // Reset for offset movement
                    homingState = HomingState::OFFSET_RIGHT;  // Next: offset from RIGHT
                }
                break;
                
            case HomingState::OFFSET_RIGHT:
                if (!enabled) {
                    // Check if we just finished the offset movement
                    if (stepCount >= offsetSteps) {
                        currentPosition = offsetSteps;  // Now at offset from RIGHT
                        DEBUG_PRINT(F("Homing Motor 2: Offset from RIGHT complete, position = "));
                        DEBUG_PRINTLN(currentPosition);
                        homingState = HomingState::MOVE_LEFT;  // Next: move to LEFT
                    } else {
                        // Start the offset movement
                        DEBUG_PRINT(F("Homing Motor 2: Moving offset "));
                        DEBUG_PRINT(offsetSteps);
                        DEBUG_PRINTLN(F(" steps to LEFT"));
                        setDirection(Config::CW_RIGHT); // LEFT movement
                        delay(Config::Timing::DIR_CHANGE_DELAY_MS);
                        delayMicroseconds(Config::Timing::DIR_SETUP_US);
                        resetStepCount();
                        totalSteps = offsetSteps;
                        enabled = true;
                    }
                }
                break;
                
            case HomingState::MOVE_LEFT:
                if (!enabled) {
                    DEBUG_PRINTLN(F("Homing Motor 2: Moving to LEFT switch..."));
                    setDirection(Config::CW_RIGHT); // Inverted: CW signal for LEFT movement
                    delay(Config::Timing::DIR_CHANGE_DELAY_MS);
                    delayMicroseconds(Config::Timing::DIR_SETUP_US);
                    resetStepCount();
                    totalSteps = 0xFFFFFFFF; // Run until limit switch hit
                    homeRangeSteps = 0;
                    enabled = true;
                }
                
                if (isLeftSwitchPressed()) {
                    enabled = false;
                    // Total range from RIGHT switch to LEFT switch
                    // = offsetSteps (RIGHT offset) + stepCount (steps from RIGHT offset to LEFT switch)
                    totalRangeSteps = stepCount + offsetSteps;
                    DEBUG_PRINT(F("Homing Motor 2: Left limit reached, total range = "));
                    DEBUG_PRINT(totalRangeSteps);
                    DEBUG_PRINTLN(F(" steps"));
                    delay(500);
                    resetStepCount(); // Reset for offset movement
                    homingState = HomingState::OFFSET_LEFT;  // Next: offset back from LEFT
                }
                break;
                
            case HomingState::OFFSET_LEFT:
                if (!enabled) {
                    // Check if we just finished the offset movement
                    if (stepCount >= offsetSteps) {
                        // Calculate final position and usable range
                        currentPosition = totalRangeSteps - offsetSteps;  // Now at LEFT offset position
                        homeRangeSteps = totalRangeSteps - (2 * offsetSteps);  // Usable range between offsets
                        
                        DEBUG_PRINT(F("Homing Motor 2: Offset from LEFT complete, position = "));
                        DEBUG_PRINTLN(currentPosition);
                        DEBUG_PRINT(F("Homing Motor 2: Usable oscillation range = "));
                        DEBUG_PRINT(homeRangeSteps);
                        DEBUG_PRINTLN(F(" steps"));
                        
                        // Mark as homed BEFORE changing state
                        isHomed = true;
                        speedProfileEnabled = false;  // Keep disabled until oscillation starts
                        LOG_PRINTLN(F("Homing Motor 2: Complete!"));
                        
                        homingState = HomingState::COMPLETE;
                    } else {
                        // Start the offset movement
                        DEBUG_PRINT(F("Homing Motor 2: Moving offset "));
                        DEBUG_PRINT(offsetSteps);
                        DEBUG_PRINTLN(F(" steps to RIGHT"));
                        setDirection(Config::CCW_LEFT); // RIGHT movement
                        delay(Config::Timing::DIR_CHANGE_DELAY_MS);
                        delayMicroseconds(Config::Timing::DIR_SETUP_US);
                        resetStepCount();
                        totalSteps = offsetSteps;
                        enabled = true;
                    }
                }
                break;
                
            case HomingState::COMPLETE:
                // Transition to IDLE (everything already set in OFFSET_RIGHT)
                enabled = false; // Make sure motor is stopped
                resetStepCount();
                totalSteps = 0;
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
        
        DEBUG_PRINT(F("Motor2 startOscillation: direction="));
        DEBUG_PRINT(directionRight ? "RIGHT" : "LEFT");
        DEBUG_PRINT(F(", currentPos="));
        DEBUG_PRINT(currentPosition);
        DEBUG_PRINT(F(", homeRange="));
        DEBUG_PRINTLN(homeRangeSteps);
        
        // Direction is inverted for this motor: RIGHT=CCW signal, LEFT=CW signal
        setDirection(directionRight ? Config::CCW_LEFT : Config::CW_RIGHT);
        delay(Config::Timing::DIR_CHANGE_DELAY_MS);
        delayMicroseconds(Config::Timing::DIR_SETUP_US);
        
        // Move EXACTLY homeRangeSteps in the specified direction
        // This ensures NO drift - every step is counted precisely
        totalSteps = homeRangeSteps;
        
        // Use pre-calculated accel/decel zones (relative to 360°)
        accelEndStep = accelZoneSteps;
        decelStartStep = (totalSteps > decelZoneSteps) ? (totalSteps - decelZoneSteps) : 0;
        
        resetStepCount();
        currentSpeedFactor = Config::Motor2::MIN_SPEED_FACTOR;
        speedProfileEnabled = true;
        enabled = true;
        
        // After movement completes, update currentPosition
        // After RIGHT->LEFT homing:
        // - currentPosition after homing = totalRangeSteps - offsetSteps (high, near LEFT)
        // - Position 0 = near RIGHT offset
        // LEFT movement: moves towards higher position (away from 0, towards totalRangeSteps)
        // RIGHT movement: moves towards lower position (towards 0)
        if (!directionRight) {
            // Moving LEFT towards totalRangeSteps - offsetSteps (high position)
            currentPosition = totalRangeSteps - offsetSteps;
        } else {
            // Moving RIGHT towards offsetSteps (low position)
            currentPosition = offsetSteps;
        }
        
        DEBUG_PRINT(F("Motor2: Starting oscillation, will end at position "));
        DEBUG_PRINTLN(currentPosition);
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
        speedProfileEnabled = false;
        DEBUG_PRINTLN(F("Motor2: Oscillation stopped"));
    }
    
    bool isOscillating() const {
        return speedProfileEnabled;
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
