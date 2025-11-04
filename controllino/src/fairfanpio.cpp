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
#include "PositionManager.h"
#include "CommandHandler.h"

// === Global Motor Instances ===
MainMotor motor1;
OscillationMotor motor2;
SequenceStateMachine sequence(motor1, motor2);
PositionManager positionManager;
CommandHandler commandHandler(motor1, motor2, sequence, positionManager);

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
    
    Serial.println(F("Setup complete, entering main loop..."));
    
    // Start automatic homing of Motor 2 (if configured)
    if (Config::Homing::AUTO_START_ON_BOOT) {
        Serial.println(F("Starting automatic homing of Motor2..."));
        motor2.startHoming();
    } else {
        Serial.println(F("Automatic homing disabled. Use 'home' command to start homing."));
    }
}

// === Main Loop ===
void loop() {
    static bool firstLoop = true;
    static bool autoStartExecuted = false;
    
    if (firstLoop) {
        Serial.println(F("Loop started!"));
        firstLoop = false;
    }
    
    // Update debounced limit switches
    motor2.updateSwitches();
    
    // Process serial commands
    commandHandler.update();
    
    // Update homing state machine if active
    if (motor2.getHomingState() != HomingState::IDLE) {
        motor2.updateHoming();
    }
    
    // Auto-start sequence after homing completes (ONLY if both auto-boot and auto-sequence are enabled)
    if (Config::Homing::AUTO_START_ON_BOOT && Config::Sequence::AUTO_START_AFTER_HOMING && !autoStartExecuted) {
        if (motor2.getHomingState() == HomingState::IDLE && motor2.isHomingComplete()) {
            Serial.println(F("Auto-starting seq1 after automatic homing..."));
            sequence.start();
            autoStartExecuted = true;
        }
    }
    
    // Update sequence state machine if active
    if (sequence.isActive()) {
        sequence.update();
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
    
    if (motor2.isEnabled()) {
        float speedFactor2 = motor2.updateSpeedProfile();
        unsigned long newPeriod2 = (unsigned long)(motor2.getTimerPeriod() / speedFactor2);
        Timer3.setPeriod(newPeriod2);
    }
    
    // Small delay to prevent excessive loop rate
    delay(10);
}
