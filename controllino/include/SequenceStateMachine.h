#ifndef SEQUENCE_STATE_MACHINE_H
#define SEQUENCE_STATE_MACHINE_H

#include "MainMotor.h"
#include "OscillationMotor.h"

class SequenceStateMachine {
private:
    enum class State {
        IDLE,
        MOVING_LEFT,
        MOVING_RIGHT,
        STOPPING  // Soft stop - let motors finish current movement
    };
    
    MainMotor& motor1;
    OscillationMotor& motor2;
    
    State currentState;
    bool motor1SameAsMotor2;
    float motor1Degrees;  // Degrees for Motor1 in sequence (set by start())
    
public:
    SequenceStateMachine(MainMotor& m1, OscillationMotor& m2)
        : motor1(m1), motor2(m2), 
          currentState(State::IDLE), 
          motor1SameAsMotor2(Config::Sequence::MOTOR1_SAME_DIR_AS_MOTOR2),
          motor1Degrees(Config::Motor1::SEQUENCE_DEGREES) {}
    
    void setSameDirection(bool same) {
        motor1SameAsMotor2 = same;
    }
    
    bool getSameDirection() const {
        return motor1SameAsMotor2;
    }
    
    void start(float customDegrees = 0.0f) {
        if (!motor2.isHomingComplete()) {
            Serial.println(F("Error: Motor 2 not homed. Run 'home' command first!"));
            return;
        }
        
        // Use custom degrees if provided, otherwise use config default
        motor1Degrees = (customDegrees > 0) ? customDegrees : Config::Motor1::SEQUENCE_DEGREES;
        
        // After homing, motor2 is at right end of usable range
        // Motor 2 ALWAYS starts moving LEFT (CCW) after homing
        // Motor 1 starts same or opposite direction based on config MOTOR1_SAME_DIR_AS_MOTOR2
        
        Serial.print(F("Sequence started: Motor2=LEFT, Motor1="));
        Serial.print(motor1SameAsMotor2 ? F("LEFT (same)") : F("RIGHT (opposite)"));
        Serial.print(F(", "));
        Serial.print(motor1Degrees);
        Serial.println(F("°"));
        
        // Start Motor 2 moving LEFT
        motor2.startOscillation(false);  // false = LEFT
        currentState = State::MOVING_LEFT;
        
        // Start Motor 1 - same or opposite direction
        motor1.setDirection(motor1SameAsMotor2 ? Config::CW_RIGHT : Config::CCW_LEFT);
        delay(Config::Timing::DIR_CHANGE_DELAY_MS);
        delayMicroseconds(Config::Timing::DIR_SETUP_US);
        motor1.startMovement(motor1Degrees);
    }
    
    void stop() {
        motor1.disable();
        motor2.disable();
        currentState = State::IDLE;
        Serial.println(F("Sequence stopped"));
    }
    
    void softStop() {
        if (currentState == State::IDLE) {
            Serial.println(F("Sequence not running"));
            return;
        }
        currentState = State::STOPPING;
        Serial.println(F("Soft stop: Motors will finish current movement..."));
    }
    
    void update() {
        if (currentState == State::IDLE) return;
        
        // Handle soft stop - wait for both motors to complete, then go idle
        if (currentState == State::STOPPING) {
            if (motor1.isMovementComplete() && motor2.isMovementComplete()) {
                currentState = State::IDLE;
                motor2.invalidateHoming();  // Motor2 position unknown - require re-homing
                Serial.println(F("Soft stop complete - Run 'home' before restarting seq1"));
            }
            return;
        }
        
        // Wait for BOTH motors to complete before reversing direction
        switch (currentState) {
            case State::MOVING_LEFT:
                if (motor1.isMovementComplete() && motor2.isMovementComplete()) {
                    Serial.println(F("Both motors completed, reversing direction"));
                    
                    // Motor 2 reverses: LEFT -> RIGHT
                    motor2.startOscillation(true);
                    
                    // Motor 1 reverses: same or opposite to Motor 2
                    motor1.setDirection(motor1SameAsMotor2 ? Config::CCW_LEFT : Config::CW_RIGHT);
                    delay(Config::Timing::DIR_CHANGE_DELAY_MS);
                    delayMicroseconds(Config::Timing::DIR_SETUP_US);
                    motor1.startMovement(motor1Degrees);
                    
                    currentState = State::MOVING_RIGHT;
                }
                break;
                
            case State::MOVING_RIGHT:
                if (motor1.isMovementComplete() && motor2.isMovementComplete()) {
                    Serial.println(F("Both motors completed, reversing direction"));
                    
                    // Motor 2 reverses: RIGHT -> LEFT
                    motor2.startOscillation(false);
                    
                    // Motor 1 reverses: same or opposite to Motor 2
                    motor1.setDirection(motor1SameAsMotor2 ? Config::CW_RIGHT : Config::CCW_LEFT);
                    delay(Config::Timing::DIR_CHANGE_DELAY_MS);
                    delayMicroseconds(Config::Timing::DIR_SETUP_US);
                    motor1.startMovement(motor1Degrees);
                    
                    currentState = State::MOVING_LEFT;
                }
                break;
                
            case State::IDLE:
            case State::STOPPING:
                // Nothing to do - handled above
                break;
        }
    }
    
    bool isActive() const {
        return currentState != State::IDLE;
    }
};

#endif // SEQUENCE_STATE_MACHINE_H
