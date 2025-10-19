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

#include "Config.h"
#include "MainMotor.h"
#include "OscillationMotor.h"
#include "SequenceStateMachine.h"
#include "CommandHandler.h"

// === Global Motor Instances ===
MainMotor motor1;
OscillationMotor motor2;
SequenceStateMachine sequence(motor1, motor2);
CommandHandler commandHandler(motor1, motor2, sequence);

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
    
    // Initialize motors
    motor1.init();
    motor2.init();
    
    // Setup Timer 1 for Motor 1
    Timer1.initialize(motor1.getTimerPeriod());
    Timer1.attachInterrupt(stepMotor1);
    
    // Setup Timer 3 for Motor 2
    Timer3.initialize(motor2.getTimerPeriod());
    Timer3.attachInterrupt(stepMotor2);
    
    Serial.println(F("System initialized"));
    Serial.print(F("Motor 1: Timer period = "));
    Serial.print(motor1.getTimerPeriod());
    Serial.println(F(" µs"));
    Serial.print(F("Motor 2: Timer period = "));
    Serial.print(motor2.getTimerPeriod());
    Serial.println(F(" µs"));
    Serial.println(F("Setup complete, entering main loop..."));
    
    // Start automatic homing of Motor 2
    Serial.println(F("Starting automatic homing..."));
    motor2.startHoming();
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
    
    // Auto-start sequence after homing completes (if configured)
    if (Config::Sequence::AUTO_START_AFTER_HOMING && !autoStartExecuted) {
        if (motor2.getHomingState() == HomingState::IDLE && motor2.isHomingComplete()) {
            Serial.println(F("Auto-starting seq1 after homing..."));
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
    
    if (motor2.isEnabled()) {
        float speedFactor2 = motor2.updateSpeedProfile();
        unsigned long newPeriod2 = (unsigned long)(motor2.getTimerPeriod() / speedFactor2);
        Timer3.setPeriod(newPeriod2);
    }
    
    // Small delay to prevent excessive loop rate
    delay(10);
}
