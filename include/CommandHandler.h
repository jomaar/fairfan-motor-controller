#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include <Arduino.h>
#include "MainMotor.h"
#include "OscillationMotor.h"
#include "SequenceStateMachine.h"

class CommandHandler {
private:
    MainMotor& motor1;
    OscillationMotor& motor2;
    SequenceStateMachine& sequence;
    
    String inputString;
    bool stringComplete;
    float motor1CustomDegrees;  // Custom degree value for Motor 1
    
    void processCommand() {
        inputString.trim();
        inputString.toLowerCase();
        
        // Motor 1 commands
        if (inputString == "go1") {
            motor1.setDirection(Config::CW_RIGHT);
            delay(Config::Timing::DIR_CHANGE_DELAY_MS);
            delayMicroseconds(Config::Timing::DIR_SETUP_US);
            
            // Use custom degrees if set, otherwise use default TEST_DEGREES
            float degreesToMove = (motor1CustomDegrees > 0) ? motor1CustomDegrees : Config::Motor1::TEST_DEGREES;
            motor1.startMovement(degreesToMove);
            
            Serial.print(F("Motor 1: Started "));
            Serial.print(degreesToMove);
            Serial.println(F("°"));
        }
        else if (inputString == "stop1") {
            motor1.disable();
            Serial.println(F("Motor 1: Stopped"));
        }
        
        // Motor 2 commands
        else if (inputString == "home" || inputString == "home2") {
            motor2.startHoming();
        }
        else if (inputString == "stop2") {
            motor2.disable();
            Serial.println(F("Motor 2: Stopped"));
        }
        
        // Emergency stop all
        else if (inputString == "stopall") {
            motor1.disable();
            motor2.disable();
            sequence.stop();
            Serial.println(F("EMERGENCY STOP: All motors stopped"));
        }
        
        // Sequence commands
        else if (inputString == "seq1") {
            // Pass custom degrees if set, otherwise sequence uses default
            sequence.start(motor1CustomDegrees);
        }
        else if (inputString == "stopseq") {
            sequence.stop();
        }
        else if (inputString == "softstop") {
            sequence.softStop();
        }
        
        // Direction mode commands
        else if (inputString == "sync" || inputString == "same") {
            sequence.setSameDirection(true);
            Serial.println(F("Mode: SAME direction (Motor1 follows Motor2)"));
        }
        else if (inputString == "opposite" || inputString == "alt") {
            sequence.setSameDirection(false);
            Serial.println(F("Mode: OPPOSITE direction (Motor1 reverse of Motor2)"));
        }
        
        // Status command
        else if (inputString == "mode" || inputString == "status") {
            Serial.print(F("Mode: "));
            Serial.println(sequence.getSameDirection() ? F("SAME direction") : F("OPPOSITE direction"));
        }
        
        // Get current Motor 1 degree setting (must be checked BEFORE startsWith)
        else if (inputString == "deg" || inputString == "degrees") {
            Serial.print(F("Motor 1 current setting: "));
            Serial.print(motor1CustomDegrees);
            Serial.println(F("°"));
        }
        
        // Set Motor 1 custom degrees (e.g., "deg360", "deg720", "deg90")
        else if (inputString.startsWith("deg")) {
            String degValue = inputString.substring(3);  // Extract number after "deg"
            degValue.trim();  // Remove any whitespace
            float degrees = degValue.toFloat();
            
            // Debug: Show what was parsed
            Serial.print(F("Parsed: '"));
            Serial.print(degValue);
            Serial.print(F("' = "));
            Serial.println(degrees);
            
            if (degValue.length() > 0 && degrees >= 0 && degrees <= 7200) {  // Allow 0-7200°
                motor1CustomDegrees = degrees;
                Serial.print(F("Motor 1 degrees set to: "));
                Serial.print(motor1CustomDegrees);
                Serial.println(F("°"));
            } else {
                Serial.println(F("Error: Degrees must be between 0 and 7200"));
            }
        }
        
        // Help command
        else if (inputString == "help") {
            printHelp();
        }
        
        else {
            Serial.print(F("Unknown command: "));
            Serial.println(inputString);
            Serial.println(F("Type 'help' for command list"));
        }
        
        inputString = "";
        stringComplete = false;
    }
    
    void printHelp() {
        Serial.println(F("\n=== Available Commands ==="));
        Serial.println(F("Motor 1:"));
        Serial.println(F("  go1       - Start Motor 1 (uses custom degrees or 180° default)"));
        Serial.println(F("  stop1     - Stop Motor 1"));
        Serial.println(F("  deg<n>    - Set Motor 1 degrees (e.g., deg360, deg720, deg90)"));
        Serial.println(F("  deg       - Show current Motor 1 degree setting"));
        Serial.println(F("\nMotor 2:"));
        Serial.println(F("  home      - Home Motor 2 (find limit switches)"));
        Serial.println(F("  stop2     - Stop Motor 2"));
        Serial.println(F("\nSequence:"));
        Serial.println(F("  seq1     - Start oscillation sequence"));
        Serial.println(F("  stopseq  - Stop sequence immediately"));
        Serial.println(F("  softstop - Stop after current movement (requires re-homing)"));
        Serial.println(F("\nEmergency:"));
        Serial.println(F("  stopall  - STOP ALL (motors + sequence)"));
        Serial.println(F("\nConfiguration:"));
        Serial.println(F("  sync     - Motor1 follows Motor2 (same direction)"));
        Serial.println(F("  opposite - Motor1 opposite to Motor2"));
        Serial.println(F("  mode     - Show current direction mode"));
        Serial.println(F("\nOther:"));
        Serial.println(F("  help     - Show this help message"));
        Serial.println(F("==========================\n"));
    }
    
public:
    CommandHandler(MainMotor& m1, OscillationMotor& m2, SequenceStateMachine& seq)
        : motor1(m1), motor2(m2), sequence(seq), 
          inputString(""), stringComplete(false), motor1CustomDegrees(0.0f) {
        inputString.reserve(50);
    }
    
    void init() {
        Serial.begin(Config::Serial::BAUD_RATE);
        Serial.println(F("\n=== FairFan Motor Controller ==="));
        Serial.println(F("Type 'help' for command list\n"));
    }
    
    void update() {
        // Read serial input
        while (Serial.available()) {
            char inChar = (char)Serial.read();
            
            if (inChar == '\n' || inChar == '\r') {
                if (inputString.length() > 0) {
                    stringComplete = true;
                }
            } else {
                inputString += inChar;
            }
        }
        
        // Process complete command
        if (stringComplete) {
            processCommand();
        }
    }
};

#endif // COMMAND_HANDLER_H
