/**
 * FairFan Motor Controller
 * 
 * Dual stepper motor control system with:
 * - Motor 1: Main motor with speed profiling (180° oscillation)
 * - Motor 2: Fine motor with homing and limit switches
 * - Synchronized or opposite direction modes
 * - Serial command interface
 * 
 * Refactored with OOP principles: SPOT, SSOT, DRY
 */

#include <Arduino.h>
#include <Controllino.h>
#include <TimerOne.h>
#include <TimerThree.h>
#include <Wire.h>

#include "Config.h"
#include "MainMotor.h"
#include "OscillationMotor.h"
#include "SequenceStateMachine.h"
#include "SoftstopStateMachine.h"
#include "PositionManager.h"
#include "CommandHandler.h"

// === Global Motor Instances ===
MainMotor motor1;
OscillationMotor motor2;
SequenceStateMachine sequence(motor1, motor2);
SoftstopStateMachine softstop(motor1, motor2);
PositionManager positionManager;
CommandHandler commandHandler(motor1, motor2, sequence, softstop, positionManager);

// === Softstop Button State ===
bool softstopButtonPressed = false;
unsigned long softstopButtonPressTime = 0;

// === Autostart State ===
enum class AutoStartState {
    IDLE,
    GOTO_HOME1,
    WAIT_MOTOR1_HOME,
    START_HOMING,
    WAIT_HOMING,
    START_SEQUENCE,
    COMPLETE
};
AutoStartState autoStartState = AutoStartState::IDLE;

// === ISR Wrappers ===
// Note: ISRs must be global functions, not class methods
// They delegate to the motor instances

void stepMotor1() {
    motor1.step();
}

void stepMotor2() {
    motor2.step();
}

// === Setup ===
void setup() {
    // Initialize serial communication
    commandHandler.init();
    
    // Initialize motors FIRST
    motor1.init();
    motor2.init();
    
    // Setup Timers BEFORE any motor movement
    Timer1.initialize(motor1.getTimerPeriod());
    Timer1.attachInterrupt(stepMotor1);
    
    Timer3.initialize(motor2.getTimerPeriod());
    Timer3.attachInterrupt(stepMotor2);
    
    Serial.println(F("System initialized"));
    Serial.print(F("Motor 1: Timer period = "));
    Serial.print(motor1.getTimerPeriod());
    Serial.println(F(" µs"));
    Serial.print(F("Motor 2: Timer period = "));
    Serial.print(motor2.getTimerPeriod());
    Serial.println(F(" µs"));
    
    // Initialize FRAM
    Serial.println(F("Initializing FRAM..."));
    if (!positionManager.init()) {
        Serial.println(F("WARNING: FRAM initialization failed!"));
        Serial.println(F("Position tracking will not be persistent."));
    }
    
    // Load saved position from FRAM
    long savedPosition = 0;
    bool positionLoaded = positionManager.loadPosition(&savedPosition);
    
    // Validate loaded position (plausibility check)
    long maxValidSteps = (long)(Config::Motor1::MAX_DEGREES / 360.0f * 32000);  // 1080° = 96000 steps
    bool positionValid = (abs(savedPosition) <= maxValidSteps);
    
    if (positionLoaded && savedPosition != 0 && positionValid) {
        Serial.print(F("⚠️  POWER LOSS DETECTED! Saved position: "));
        Serial.print(savedPosition);
        Serial.print(F(" steps ("));
        Serial.print(motor1.stepsToDegrees(savedPosition), 2);
        Serial.println(F("°)"));
        
        // Restore position to motor
        motor1.setPosition(savedPosition);
        
        // Auto-recovery if enabled
        if (Config::FRAM::ENABLE_AUTO_RECOVERY) {
            float degreesToMove = abs(motor1.getPositionDegrees());
            
            // Sanity check: Don't recover if too far (possible corruption)
            if (degreesToMove > Config::Motor1::MAX_DEGREES) {
                Serial.println(F("ERROR: Saved position exceeds limits! FRAM may be corrupt."));
                Serial.println(F("Resetting to home position (0). Please calibrate with 'setzero'."));
                motor1.setPosition(0);
                positionManager.savePosition(0);
            } else {
                Serial.println(F("AUTO-RECOVERY will start in main loop..."));
                Serial.print(F("Recovery needed: "));
                Serial.print(degreesToMove, 2);
                Serial.println(F("°"));
                Serial.println(F("Use 'gotohome1' command or wait for auto-recovery"));
            }
        }
    } else if (positionLoaded && !positionValid) {
        Serial.print(F("⚠️  FRAM DATA CORRUPT! Invalid position: "));
        Serial.print(savedPosition);
        Serial.println(F(" steps"));
        Serial.println(F("Resetting to home (0). Please calibrate with 'setzero' command."));
        motor1.setPosition(0);
        positionManager.clear();
        positionManager.savePosition(0);
    } else {
        Serial.println(F("Motor1 starting at home position (0)"));
        motor1.setPosition(0);
    }
    
    // Initialize softstop button (24V input via optocoupler)
    pinMode(Config::Buttons::SOFTSTOP_PIN, INPUT);
    Serial.println(F("Softstop button on DI2 (24V input, active HIGH when pressed)"));
    
    Serial.println(F("Setup complete, entering main loop..."));
    
    // Note: Autostart sequence is now triggered by button press (if BUTTON_AUTOSTART enabled)
    if (!Config::Sequence::BUTTON_AUTOSTART && Config::Homing::AUTO_START_ON_BOOT) {
        // Old behavior: autostart immediately on boot (if both flags set)
        Serial.println(F("Auto-boot enabled. Autostart will begin in loop."));
    } else if (Config::Sequence::BUTTON_AUTOSTART) {
        Serial.println(F("🔘 BUTTON MODE: Press button to start autostart (gotohome1 -> home -> seq1)"));
    } else {
        Serial.println(F("Automatic homing disabled. Use 'home' command to start homing."));
    }
}

// === Main Loop ===
void loop() {
    static bool firstLoop = true;
    
    if (firstLoop) {
        Serial.println(F("Loop started!"));
        firstLoop = false;
        
        // Initialize autostart sequence only if NOT using button mode
        if (!Config::Sequence::BUTTON_AUTOSTART && Config::Homing::AUTO_START_ON_BOOT && Config::Sequence::AUTO_START_AFTER_HOMING) {
            long currentPos = motor1.getPosition();
            if (currentPos != 0) {
                Serial.println(F("=== AUTOSTART: Step 1 - Moving Motor1 to home (0°) ==="));
                float degreesToMove = abs(motor1.getPositionDegrees());
                bool directionCW = (currentPos < 0);
                
                motor1.setDirection(directionCW ? Config::CW_RIGHT : Config::CCW_LEFT);
                delay(Config::Timing::DIR_CHANGE_DELAY_MS);
                delayMicroseconds(Config::Timing::DIR_SETUP_US);
                motor1.startMovement(degreesToMove, false);
                
                autoStartState = AutoStartState::WAIT_MOTOR1_HOME;
            } else {
                Serial.println(F("=== AUTOSTART: Motor1 already at home, starting homing ==="));
                autoStartState = AutoStartState::START_HOMING;
            }
        }
    }
    
    // Update debounced limit switches
    motor2.updateSwitches();
    
    // Process serial commands
    commandHandler.update();
    
    // Autostart state machine
    switch (autoStartState) {
        case AutoStartState::WAIT_MOTOR1_HOME:
            if (motor1.isMovementComplete()) {
                Serial.println(F("=== AUTOSTART: Step 2 - Motor1 at home, starting Motor2 homing ==="));
                autoStartState = AutoStartState::START_HOMING;
            }
            break;
            
        case AutoStartState::START_HOMING:
            motor2.startHoming();
            autoStartState = AutoStartState::WAIT_HOMING;
            break;
            
        case AutoStartState::WAIT_HOMING:
            if (motor2.getHomingState() == HomingState::IDLE && motor2.isHomingComplete()) {
                Serial.println(F("=== AUTOSTART: Step 3 - Homing complete, starting seq1 ==="));
                sequence.start();
                autoStartState = AutoStartState::COMPLETE;
            }
            break;
            
        case AutoStartState::IDLE:
        case AutoStartState::GOTO_HOME1:
        case AutoStartState::START_SEQUENCE:
        case AutoStartState::COMPLETE:
            // Nothing to do
            break;
    }
    
    // Update homing state machine if active (including COMPLETE state for transition to IDLE)
    if (motor2.getHomingState() != HomingState::IDLE) {
        motor2.updateHoming();
    }
    
    // Update sequence state machine if active
    if (sequence.isActive()) {
        sequence.update();
    }
    
    // Update softstop state machine if active
    if (softstop.isActive()) {
        softstop.update();
    } else if (autoStartState == AutoStartState::COMPLETE) {
        // Reset autostart state after softstop completes or sequence ends
        // This allows the button to trigger autostart again
        autoStartState = AutoStartState::IDLE;
    }
    
    // === Softstop Button Handling (with debouncing) ===
    // 24V input via optocoupler: LOW = not pressed, HIGH = pressed (24V applied)
    bool buttonState = digitalRead(Config::Buttons::SOFTSTOP_PIN);
    
    if (buttonState == HIGH && !softstopButtonPressed) {
        // Button just pressed (24V applied to DI2)
        softstopButtonPressTime = millis();
        softstopButtonPressed = true;
    } else if (buttonState == LOW && softstopButtonPressed) {
        // Button released - check if it was pressed long enough (debounce)
        unsigned long pressDuration = millis() - softstopButtonPressTime;
        
        if (pressDuration >= Config::Buttons::DEBOUNCE_MS) {
            // Valid button press detected
            
            // Check system state and trigger appropriate action
            if (Config::Sequence::BUTTON_AUTOSTART) {
                // Button mode enabled
                
                if (sequence.isActive()) {
                    // Sequence running → trigger softstop
                    if (!softstop.isActive()) {
                        Serial.println(F("🔴 BUTTON: Softstop triggered"));
                        sequence.stopWithoutMotors();
                        softstop.start();
                    }
                } else if (autoStartState == AutoStartState::IDLE && 
                           motor2.getHomingState() == HomingState::IDLE && 
                           !motor1.isEnabled()) {
                    // System idle → trigger autostart
                    Serial.println(F("🟢 BUTTON: Starting autostart sequence..."));
                    long currentPos = motor1.getPosition();
                    
                    if (currentPos != 0) {
                        Serial.println(F("=== AUTOSTART: Step 1 - Moving Motor1 to home (0°) ==="));
                        float degreesToMove = abs(motor1.getPositionDegrees());
                        bool directionCW = (currentPos < 0);
                        
                        motor1.setDirection(directionCW ? Config::CW_RIGHT : Config::CCW_LEFT);
                        delay(Config::Timing::DIR_CHANGE_DELAY_MS);
                        delayMicroseconds(Config::Timing::DIR_SETUP_US);
                        motor1.startMovement(degreesToMove, false);
                        autoStartState = AutoStartState::WAIT_MOTOR1_HOME;
                    } else {
                        Serial.println(F("=== AUTOSTART: Motor1 already at home, starting Motor2 homing ==="));
                        autoStartState = AutoStartState::START_HOMING;
                    }
                }
            } else {
                // Button mode disabled, only softstop
                if (sequence.isActive() && !softstop.isActive()) {
                    Serial.println(F("⚠️  SOFTSTOP BUTTON PRESSED!"));
                    sequence.stopWithoutMotors();
                    softstop.start();
                }
            }
        }
        
        softstopButtonPressed = false;
    }
    
    // Update speed profiles
    if (motor1.isEnabled()) {
        float speedFactor1 = motor1.updateSpeedProfile();
        unsigned long newPeriod1 = (unsigned long)(motor1.getTimerPeriod() / speedFactor1);
        Timer1.setPeriod(newPeriod1);
    }
    
    // Save Motor1 position to FRAM periodically (every ~0.5°)
    if (motor1.shouldSavePosition()) {
        positionManager.savePosition(motor1.getPosition());
        motor1.markPositionSaved();
    }
    
    // Save position when movement completes
    static bool motor1WasMoving = false;
    if (motor1WasMoving && motor1.isMovementComplete()) {
        positionManager.savePosition(motor1.getPosition());
        motor1.markPositionSaved();
        Serial.print(F("Position saved: "));
        Serial.print(motor1.getPosition());
        Serial.print(F(" steps ("));
        Serial.print(motor1.getPositionDegrees(), 2);
        Serial.println(F("°)"));
    }
    motor1WasMoving = motor1.isEnabled();
    
    // Track Motor2 oscillation completion to verify NO drift
    static bool motor2WasMoving = false;
    if (motor2WasMoving && !motor2.isEnabled() && motor2.isOscillating()) {
        Serial.print(F("Motor2 oscillation COMPLETE: "));
        Serial.print(motor2.getStepCount());
        Serial.print(F(" steps (expected "));
        Serial.print(motor2.getOscillationSteps());
        Serial.println(F(")"));
        if (motor2.getStepCount() != motor2.getOscillationSteps()) {
            Serial.println(F("WARNING: Step count mismatch! DRIFT detected!"));
        }
    }
    motor2WasMoving = motor2.isEnabled();
    
    if (motor2.isEnabled()) {
        float speedFactor2 = motor2.updateSpeedProfile();
        unsigned long newPeriod2 = (unsigned long)(motor2.getTimerPeriod() / speedFactor2);
        Timer3.setPeriod(newPeriod2);
    }
    
    // Small delay to prevent excessive loop rate
    delay(10);
}
