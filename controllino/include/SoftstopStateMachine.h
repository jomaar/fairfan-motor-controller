#ifndef SOFTSTOP_STATE_MACHINE_H
#define SOFTSTOP_STATE_MACHINE_H

#include "MainMotor.h"
#include "OscillationMotor.h"
#include "Config.h"

enum class SoftstopState {
    IDLE,
    WAITING_FOR_MOTORS_TO_STOP,
    MOTOR1_RETURNING_HOME,
    MOTOR2_HOMING
};

class SoftstopStateMachine {
private:
    MainMotor& motor1;
    OscillationMotor& motor2;
    SoftstopState currentState;
    bool motor1NeedsReversal;  // True if Motor1 needs to reverse after stopping
    bool motor1Handled;        // True if Motor1 reversal already started
    bool motor2Handled;        // True if Motor2 homing already started
    
public:
    SoftstopStateMachine(MainMotor& m1, OscillationMotor& m2)
        : motor1(m1), motor2(m2), currentState(SoftstopState::IDLE), 
          motor1NeedsReversal(false), motor1Handled(false), motor2Handled(false) {}
    
    void start() {
        float currentDeg = motor1.getPositionDegrees();
        
        Serial.println(F("SOFT STOP: Motors will decel to reversal points..."));
        Serial.print(F("  Motor1 at "));
        Serial.print(currentDeg, 1);
        Serial.print(F("° - will decel to reversal point (0° or "));
        Serial.print(Config::Motor1::MAX_DEGREES, 0);
        Serial.println(F("°)"));
        
        // Check if Motor1 is heading to MAX_DEGREES (upper half) or to 0° (lower half)
        if (currentDeg > Config::Motor1::MAX_DEGREES / 2.0f) {
            // Motor1 was moving CW toward MAX_DEGREES
            // After reaching MAX_DEGREES with decel, it needs to reverse to 0°
            Serial.println(F("  -> After reaching reversal point, will return to 0°"));
            motor1NeedsReversal = true;
        } else {
            // Motor1 was moving CCW toward 0°
            // After reaching 0° with decel, it's already home
            Serial.println(F("  -> After reaching reversal point (0°), already home"));
            motor1NeedsReversal = false;
        }
        
        Serial.println(F("  Motor2: Will complete current oscillation with decel"));
        
        motor1Handled = false;
        motor2Handled = false;
        currentState = SoftstopState::WAITING_FOR_MOTORS_TO_STOP;
    }
    
    void stop() {
        if (currentState != SoftstopState::IDLE) {
            Serial.println(F("Softstop cancelled"));
            currentState = SoftstopState::IDLE;
        }
    }
    
    bool isActive() const {
        return currentState != SoftstopState::IDLE;
    }
    
    void update() {
        switch (currentState) {
            case SoftstopState::IDLE:
                // Nothing to do
                break;
                
            case SoftstopState::WAITING_FOR_MOTORS_TO_STOP:
                // Handle Motor1 stopping (independent of Motor2)
                if (!motor1Handled && !motor1.isEnabled()) {
                    motor1Handled = true;
                    
                    float currentDeg = motor1.getPositionDegrees();
                    
                    Serial.println(F("SOFT STOP: Motor1 reached reversal point"));
                    Serial.print(F("  Motor1: Stopped at "));
                    Serial.print(currentDeg, 1);
                    Serial.println(F("°"));
                    
                    // Check if Motor1 needs to move to 0°
                    // Allow small tolerance (e.g., within 1°) to consider as "at home"
                    if (abs(currentDeg) < 1.0f) {
                        // Motor1 is already at/near home position
                        Serial.println(F("  Motor1: Already at home (0°)"));
                    } else {
                        // Motor1 needs to return to 0°, regardless of previous direction
                        Serial.print(F("  Motor1: Returning from "));
                        Serial.print(currentDeg, 1);
                        Serial.println(F("° to 0° with accel/decel"));
                        
                        motor1.setDirection(Config::CCW_LEFT);
                        delay(Config::Timing::DIR_CHANGE_DELAY_MS);
                        delayMicroseconds(Config::Timing::DIR_SETUP_US);
                        motor1.startMovement(currentDeg, false);  // false = skip limit check (return to home)
                        
                        // Motor1 is now moving, will be checked in MOTOR1_RETURNING_HOME state
                    }
                }
                
                // Handle Motor2 stopping (independent of Motor1) - just mark as handled
                if (!motor2Handled && !motor2.isEnabled()) {
                    motor2Handled = true;
                    Serial.println(F("SOFT STOP: Motor2 reached reversal point (waiting for Motor1)"));
                }
                
                // Transition once both motors have stopped their initial movement
                if (motor1Handled && motor2Handled) {
                    float currentDeg = motor1.getPositionDegrees();
                    
                    // Check if Motor1 is already at home (within 1° tolerance)
                    if (abs(currentDeg) < 1.0f) {
                        // Motor1 already home, skip directly to Motor2 homing
                        Serial.println(F("SOFT STOP: Motor1 at home, starting Motor2 homing..."));
                        motor2.startHoming();
                        currentState = SoftstopState::MOTOR2_HOMING;
                    } else {
                        // Motor1 is returning home, wait for it
                        currentState = SoftstopState::MOTOR1_RETURNING_HOME;
                        Serial.println(F("SOFT STOP: Waiting for Motor1 to reach 0°..."));
                    }
                }
                break;
                
            case SoftstopState::MOTOR1_RETURNING_HOME:
                // Wait for Motor1 to reach home position
                if (!motor1.isEnabled()) {
                    Serial.println(F("SOFT STOP: Motor1 at home (0°)"));
                    Serial.println(F("SOFT STOP: Starting Motor2 homing..."));
                    motor2.startHoming();
                    currentState = SoftstopState::MOTOR2_HOMING;
                }
                break;
                
            case SoftstopState::MOTOR2_HOMING:
                // Wait for Motor2 homing to complete
                if (motor2.getHomingState() == HomingState::IDLE) {
                    Serial.println(F("SOFT STOP: Complete! Both motors at home"));
                    currentState = SoftstopState::IDLE;
                }
                break;
        }
    }
};

#endif // SOFTSTOP_STATE_MACHINE_H
