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
    float motor2TriggerHigh;
    float motor2TriggerLow;
    
public:
    SequenceStateMachine(MainMotor& m1, OscillationMotor& m2)
        : motor1(m1), motor2(m2), currentState(SequenceState::IDLE),
          motor2TriggeredHigh(false), motor2TriggeredLow(false),
          motor2TriggerHigh(0.0f), motor2TriggerLow(0.0f) {}
    
    void start() {
        if (!motor2.isHomingComplete()) {
            Serial.println(F("Error: Motor 2 not homed. Run 'home' command first!"));
            return;
        }
        
        motor2TriggerHigh = Config::Sequence::MOTOR2_TRIGGER_HIGH;
        motor2TriggerLow = Config::Sequence::MOTOR2_TRIGGER_LOW;
        
        // Debug output
        unsigned long motor2Steps = motor2.getOscillationSteps();
        float motor2Degrees = motor2.stepsToDegrees(motor2Steps);
        
        Serial.println(F("=== Independent Sequence Started ==="));
        Serial.print(F("Motor1: 0\xF8 <--> "));
        Serial.print(Config::Motor1::MAX_DEGREES, 0);
        Serial.print(F("\xF8 @ "));
        Serial.print(Config::Motor1::TARGET_RPM, 1);
        Serial.println(F(" RPM"));
        
        Serial.print(F("Motor2: "));
        Serial.print(motor2Degrees, 1);
        Serial.print(F("\xF8 @ "));
        Serial.print(Config::Motor2::TARGET_RPM, 1);
        Serial.println(F(" RPM"));
        
        Serial.print(F("Motor2 trigger HIGH: "));
        Serial.print(motor2TriggerHigh, 0);
        Serial.println(F("\xF8 of Motor1"));
        
        Serial.print(F("Motor2 trigger LOW: "));
        Serial.print(motor2TriggerLow, 0);
        Serial.println(F("\xF8 of Motor1"));
        
        motor1.setDirection(Config::CW_RIGHT);
        delay(Config::Timing::DIR_CHANGE_DELAY_MS);
        delayMicroseconds(Config::Timing::DIR_SETUP_US);
        motor1.startMovement(Config::Motor1::MAX_DEGREES, false);
        
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
    
    // Stop sequence state machine without stopping motors (for softstop)
    void stopWithoutMotors() {
        if (currentState == SequenceState::IDLE) {
            return;
        }
        
        Serial.println(F("Sequence: Stopping state machine (motors continue)"));
        currentState = SequenceState::IDLE;
        motor2TriggeredHigh = false;
        motor2TriggeredLow = false;
    }
    
    bool isActive() const {
        return currentState != SequenceState::IDLE;
    }
    
    void update() {
        if (currentState == SequenceState::IDLE) return;
        
        float motor1Position = motor1.getPositionDegrees();
        
        switch (currentState) {
            case SequenceState::MOTOR1_TO_MAX_CW:
                // Trigger Motor2 at HIGH position (near MAX_DEGREES) - only if Motor2 is NOT moving
                if (!motor2TriggeredHigh && motor1Position >= motor2TriggerHigh) {
                    if (!motor2.isEnabled()) {
                        Serial.print(F("Motor2 TRIGGER HIGH at Motor1="));
                        Serial.print(motor1Position, 1);
                        Serial.print(F("\xF8, starting RIGHT oscillation ("));
                        Serial.print(motor2.getOscillationSteps());
                        Serial.println(F(" steps)"));
                        motor2.startOscillation(true);  // RIGHT: from LEFT offset to RIGHT offset
                        motor2TriggeredHigh = true;
                    }
                }
                
                if (motor1.isMovementComplete()) {
                    Serial.println(F("Phase 2: Motor1 -> 0\xF8 CCW"));
                    
                    motor1.setDirection(Config::CCW_LEFT);
                    delay(Config::Timing::DIR_CHANGE_DELAY_MS);
                    delayMicroseconds(Config::Timing::DIR_SETUP_US);
                    motor1.startMovement(Config::Motor1::MAX_DEGREES, false);
                    
                    currentState = SequenceState::MOTOR1_TO_ZERO_CCW;
                }
                break;
                
            case SequenceState::MOTOR1_TO_ZERO_CCW:
                // Trigger Motor2 at LOW position (near 0°) - only if Motor2 is NOT moving
                if (!motor2TriggeredLow && motor1Position <= motor2TriggerLow) {
                    if (!motor2.isEnabled()) {
                        Serial.print(F("Motor2 TRIGGER LOW at Motor1="));
                        Serial.print(motor1Position, 1);
                        Serial.print(F("\xF8, starting LEFT oscillation ("));
                        Serial.print(motor2.getOscillationSteps());
                        Serial.println(F(" steps)"));
                        motor2.startOscillation(false);  // LEFT: from RIGHT offset to LEFT offset
                        motor2TriggeredLow = true;
                    }
                }
                
                if (motor1.isMovementComplete()) {
                    Serial.println(F("Phase 1: Motor1 -> 720\xF8 CW"));
                    
                    motor1.setDirection(Config::CW_RIGHT);
                    delay(Config::Timing::DIR_CHANGE_DELAY_MS);
                    delayMicroseconds(Config::Timing::DIR_SETUP_US);
                    motor1.startMovement(Config::Motor1::MAX_DEGREES, false);
                    
                    currentState = SequenceState::MOTOR1_TO_MAX_CW;
                    motor2TriggeredHigh = false;  // Reset for next cycle
                    motor2TriggeredLow = false;   // Reset for next cycle
                }
                break;
                
            case SequenceState::IDLE:
                break;
        }
    }
};

#endif
