#ifndef COMMAND_HANDLER_H
#define COMMAND_HANDLER_H

#include <Arduino.h>
#include "MainMotor.h"
#include "OscillationMotor.h"
#include "SequenceStateMachine.h"
#include "PositionManager.h"

class CommandHandler {
private:
    MainMotor& motor1;
    OscillationMotor& motor2;
    SequenceStateMachine& sequence;
    PositionManager& positionManager;
    
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
            if (motor1.startMovement(degreesToMove)) {
                Serial.print(F("Motor 1: Started "));
                Serial.print(degreesToMove);
                Serial.println(F("°"));
            }
        }
        else if (inputString == "stop1") {
            motor1.disable();
            Serial.println(F("Motor 1: Stopped"));
        }
        
        // Motor 1 directional commands (e.g., "m1ccw120", "m1cw90")
        else if (inputString.startsWith("m1ccw") || inputString.startsWith("m1cw")) {
            bool isCW = inputString.startsWith("m1cw");
            String degValue = inputString.substring(isCW ? 4 : 5);  // Extract number after "m1cw" or "m1ccw"
            degValue.trim();
            float degrees = degValue.toFloat();
            
            if (degValue.length() > 0 && degrees > 0 && degrees <= Config::Motor1::MAX_DEGREES) {
                motor1.setDirection(isCW ? Config::CW_RIGHT : Config::CCW_LEFT);
                delay(Config::Timing::DIR_CHANGE_DELAY_MS);
                delayMicroseconds(Config::Timing::DIR_SETUP_US);
                
                if (motor1.startMovement(degrees)) {
                    Serial.print(F("Motor 1: Started "));
                    Serial.print(degrees);
                    Serial.print(F("° "));
                    Serial.println(isCW ? F("CW") : F("CCW"));
                }
            } else {
                Serial.print(F("Error: Invalid degrees. Use 0.1-"));
                Serial.print(Config::Motor1::MAX_DEGREES);
                Serial.println(F("° (e.g., m1cw120, m1ccw90)"));
            }
        }
        
        // === New FRAM/Position Commands ===
        
        // Set current position as zero (home)
        else if (inputString == "setzero" || inputString == "zero") {
            motor1.setPosition(0);
            positionManager.savePosition(0);
            Serial.println(F("Motor1 position set to ZERO (home)"));
            Serial.println(F("FRAM updated - this is now the reference point"));
        }
        
        // Get current position
        else if (inputString == "getpos1" || inputString == "pos1") {
            motor1.printPositionInfo();
        }
        
        // Show position limits
        else if (inputString == "poslimit" || inputString == "limits") {
            motor1.printPositionInfo();
        }
        
        // Go to home position (0)
        else if (inputString == "gotohome1" || inputString == "home1") {
            long currentPos = motor1.getPosition();
            
            if (currentPos == 0) {
                Serial.println(F("Motor1 already at home position (0)"));
                // Don't return here - let the command finish properly
            } else {
                // Calculate degrees to move
                float degreesToMove = abs(motor1.getPositionDegrees());
                bool directionCW = (currentPos < 0);  // If negative, go CW to reach 0
                
                Serial.print(F("Returning to home: "));
                Serial.print(degreesToMove, 2);
                Serial.print(F("° "));
                Serial.println(directionCW ? F("CW") : F("CCW"));
                
                motor1.setDirection(directionCW ? Config::CW_RIGHT : Config::CCW_LEFT);
                delay(Config::Timing::DIR_CHANGE_DELAY_MS);
                delayMicroseconds(Config::Timing::DIR_SETUP_US);
                
                // Don't check limits when going home
                motor1.startMovement(degreesToMove, false);
            }
        }
        
        // FRAM info
        else if (inputString == "framinfo" || inputString == "fram") {
            positionManager.printInfo();
        }
        
        // Clear FRAM (factory reset)
        else if (inputString == "framclear" || inputString == "framreset") {
            positionManager.clear();
            motor1.setPosition(0);
            Serial.println(F("FRAM cleared, position reset to 0"));
            Serial.println(F("Please run 'setzero' at mechanical home position"));
        }
        
        // Quick FRAM fix for corrupt data
        else if (inputString == "framfix") {
            Serial.println(F("Fixing FRAM corruption..."));
            positionManager.clear();
            motor1.setPosition(0);
            positionManager.savePosition(0);
            positionManager.savePosition(0);  // Write both slots
            Serial.println(F("✓ FRAM repaired and initialized"));
            Serial.println(F("Position set to 0 - please calibrate with 'setzero' at home"));
        }
        
        // === Low-Level FRAM Commands ===
        
        // Write to FRAM: wFRAM<address>-<value>
        // Examples: wFRAM0-255, wFRAM100-42, wFRAM0x10-0xFF
        else if (inputString.startsWith("wfram")) {
            String params = inputString.substring(5);  // Remove "wfram"
            params.trim();
            
            int dashPos = params.indexOf('-');
            if (dashPos > 0) {
                String addrStr = params.substring(0, dashPos);
                String valueStr = params.substring(dashPos + 1);
                addrStr.trim();
                valueStr.trim();
                
                // Parse address (support decimal and hex)
                uint16_t address = 0;
                if (addrStr.startsWith("0x")) {
                    address = (uint16_t)strtol(addrStr.c_str(), NULL, 16);
                } else {
                    address = (uint16_t)addrStr.toInt();
                }
                
                // Parse value (support decimal and hex)
                uint8_t value = 0;
                if (valueStr.startsWith("0x")) {
                    value = (uint8_t)strtol(valueStr.c_str(), NULL, 16);
                } else {
                    value = (uint8_t)valueStr.toInt();
                }
                
                // Check address range (MB85RC256V = 32KB = 0x0000-0x7FFF)
                if (address > 0x7FFF) {
                    Serial.print(F("ERROR: Address out of range (max 0x7FFF): 0x"));
                    Serial.println(address, HEX);
                } else {
                    positionManager.writeByteRaw(address, value);
                    Serial.print(F("FRAM[0x"));
                    if (address < 0x1000) Serial.print(F("0"));
                    if (address < 0x100) Serial.print(F("0"));
                    if (address < 0x10) Serial.print(F("0"));
                    Serial.print(address, HEX);
                    Serial.print(F("] = 0x"));
                    if (value < 0x10) Serial.print(F("0"));
                    Serial.print(value, HEX);
                    Serial.print(F(" ("));
                    Serial.print(value);
                    Serial.println(F(")"));
                }
            } else {
                Serial.println(F("ERROR: Invalid format. Use: wFRAM<addr>-<value>"));
                Serial.println(F("Examples: wFRAM0-255, wFRAM100-42, wFRAM0x10-0xFF"));
            }
        }
        
        // Read from FRAM: rFRAM<address> or rFRAM<start>-<end>
        // Examples: rFRAM0, rFRAM100, rFRAM0x10, rFRAM0-15 (read range)
        else if (inputString.startsWith("rfram")) {
            String params = inputString.substring(5);  // Remove "rfram"
            params.trim();
            
            int dashPos = params.indexOf('-');
            
            if (dashPos > 0) {
                // Range read: rFRAM<start>-<end>
                String startStr = params.substring(0, dashPos);
                String endStr = params.substring(dashPos + 1);
                startStr.trim();
                endStr.trim();
                
                uint16_t startAddr = 0;
                uint16_t endAddr = 0;
                
                if (startStr.startsWith("0x")) {
                    startAddr = (uint16_t)strtol(startStr.c_str(), NULL, 16);
                } else {
                    startAddr = (uint16_t)startStr.toInt();
                }
                
                if (endStr.startsWith("0x")) {
                    endAddr = (uint16_t)strtol(endStr.c_str(), NULL, 16);
                } else {
                    endAddr = (uint16_t)endStr.toInt();
                }
                
                if (startAddr > 0x7FFF || endAddr > 0x7FFF) {
                    Serial.println(F("ERROR: Address out of range (max 0x7FFF)"));
                } else if (startAddr > endAddr) {
                    Serial.println(F("ERROR: Start address must be <= end address"));
                } else if ((endAddr - startAddr) > 255) {
                    Serial.println(F("ERROR: Range too large (max 256 bytes)"));
                } else {
                    Serial.print(F("FRAM[0x"));
                    if (startAddr < 0x1000) Serial.print(F("0"));
                    if (startAddr < 0x100) Serial.print(F("0"));
                    if (startAddr < 0x10) Serial.print(F("0"));
                    Serial.print(startAddr, HEX);
                    Serial.print(F(" - 0x"));
                    if (endAddr < 0x1000) Serial.print(F("0"));
                    if (endAddr < 0x100) Serial.print(F("0"));
                    if (endAddr < 0x10) Serial.print(F("0"));
                    Serial.print(endAddr, HEX);
                    Serial.println(F("]:"));
                    
                    for (uint16_t addr = startAddr; addr <= endAddr; addr++) {
                        if ((addr - startAddr) % 16 == 0) {
                            Serial.print(F("0x"));
                            if (addr < 0x1000) Serial.print(F("0"));
                            if (addr < 0x100) Serial.print(F("0"));
                            if (addr < 0x10) Serial.print(F("0"));
                            Serial.print(addr, HEX);
                            Serial.print(F(": "));
                        }
                        
                        uint8_t value = positionManager.readByteRaw(addr);
                        if (value < 0x10) Serial.print(F("0"));
                        Serial.print(value, HEX);
                        Serial.print(F(" "));
                        
                        if ((addr - startAddr + 1) % 16 == 0 || addr == endAddr) {
                            Serial.println();
                        }
                    }
                }
            } else {
                // Single byte read: rFRAM<address>
                uint16_t address = 0;
                if (params.startsWith("0x")) {
                    address = (uint16_t)strtol(params.c_str(), NULL, 16);
                } else {
                    address = (uint16_t)params.toInt();
                }
                
                if (address > 0x7FFF) {
                    Serial.print(F("ERROR: Address out of range (max 0x7FFF): 0x"));
                    Serial.println(address, HEX);
                } else {
                    uint8_t value = positionManager.readByteRaw(address);
                    Serial.print(F("FRAM[0x"));
                    if (address < 0x1000) Serial.print(F("0"));
                    if (address < 0x100) Serial.print(F("0"));
                    if (address < 0x10) Serial.print(F("0"));
                    Serial.print(address, HEX);
                    Serial.print(F("] = 0x"));
                    if (value < 0x10) Serial.print(F("0"));
                    Serial.print(value, HEX);
                    Serial.print(F(" ("));
                    Serial.print(value);
                    Serial.println(F(")"));
                }
            }
        }
        
        // Motor 2 commands
        else if (inputString == "home" || inputString == "home2") {
            motor2.startHoming();
        }
        else if (inputString == "stop2") {
            motor2.disable();
            Serial.println(F("Motor 2: Stopped"));
        }
        
        // Soft stop: Motor1 to home + Motor2 homing (parallel)
        else if (inputString == "softstop") {
            // Stop sequence first
            sequence.stop();
            Serial.println(F("SOFT STOP: Returning to home positions..."));
            
            // Motor1: Go to home (0°)
            long currentPos = motor1.getPosition();
            if (currentPos != 0) {
                float degreesToMove = abs(motor1.getPositionDegrees());
                bool directionCW = (currentPos < 0);
                
                Serial.print(F("  Motor1: Returning to 0° ("));
                Serial.print(degreesToMove, 1);
                Serial.print(F("° "));
                Serial.print(directionCW ? F("CW") : F("CCW"));
                Serial.println(F(")"));
                
                motor1.setDirection(directionCW ? Config::CW_RIGHT : Config::CCW_LEFT);
                delay(Config::Timing::DIR_CHANGE_DELAY_MS);
                delayMicroseconds(Config::Timing::DIR_SETUP_US);
                motor1.startMovement(degreesToMove, false);
            } else {
                Serial.println(F("  Motor1: Already at home (0°)"));
            }
            
            // Motor2: Start homing (runs parallel to Motor1)
            Serial.println(F("  Motor2: Starting homing sequence..."));
            motor2.startHoming();
        }
        
        // Emergency stop all (hard stop)
        else if (inputString == "stopall") {
            motor1.disable();
            motor2.disable();
            sequence.stop();
            Serial.println(F("EMERGENCY STOP: All motors stopped"));
        }
        
        // Sequence commands
        else if (inputString == "seq1") {
            sequence.start();  // Uses Config::Sequence settings
        }
        else if (inputString == "stopseq") {
            sequence.stop();
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
            
            if (degValue.length() > 0 && degrees >= 0 && degrees <= Config::Motor1::MAX_DEGREES) {
                motor1CustomDegrees = degrees;
                Serial.print(F("Motor 1 degrees set to: "));
                Serial.print(motor1CustomDegrees);
                Serial.println(F("°"));
            } else {
                Serial.print(F("Error: Degrees must be between 0 and "));
                Serial.print(Config::Motor1::MAX_DEGREES);
                Serial.println(F("° (3 rotations max)"));
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
        Serial.println(F("  m1cw<n>   - Rotate Motor 1 clockwise (e.g., m1cw120 = 120° CW)"));
        Serial.println(F("  m1ccw<n>  - Rotate Motor 1 counter-clockwise (e.g., m1ccw90 = 90° CCW)"));
        Serial.println(F("  deg<n>    - Set Motor 1 degrees (0-1080°, e.g., deg360, deg720)"));
        Serial.println(F("  deg       - Show current Motor 1 degree setting"));
        Serial.println(F("\nMotor 1 Position (FRAM):"));
        Serial.println(F("  setzero   - Set current position as home (zero point)"));
        Serial.println(F("  getpos1   - Show current Motor1 position"));
        Serial.println(F("  gotohome1 - Return Motor1 to home position (0)"));
        Serial.println(F("  poslimit  - Show position limits and remaining rotation"));
        Serial.println(F("  framinfo  - Show FRAM status and saved position"));
        Serial.println(F("  framclear - Clear FRAM (factory reset)"));
        Serial.println(F("  framfix   - Fix corrupt FRAM data and reset to 0"));
        Serial.println(F("\nFRAM Low-Level (Debug):"));
        Serial.println(F("  wFRAM<addr>-<val>  - Write byte (e.g., wFRAM100-255, wFRAM0x10-0xFF)"));
        Serial.println(F("  rFRAM<addr>        - Read byte (e.g., rFRAM100, rFRAM0x10)"));
        Serial.println(F("  rFRAM<start>-<end> - Read range (e.g., rFRAM0-15, rFRAM0x00-0x0F)"));
        Serial.println(F("\nMotor 2:"));
        Serial.println(F("  home      - Home Motor 2 (find limit switches)"));
        Serial.println(F("  stop2     - Stop Motor 2"));
        Serial.println(F("\nSequence (Synchronized Oscillation):"));
        Serial.println(F("  seq1     - Start sequence (Motor1: 0°⟷720°, Motor2: overlap at ends)"));
        Serial.println(F("  stopseq  - Stop sequence immediately"));
        Serial.println(F("\nStop Commands:"));
        Serial.println(F("  softstop - Soft stop: Motor1→home (0°) + Motor2→home (parallel)"));
        Serial.println(F("  stopall  - Emergency stop: ALL motors immediately"));
        Serial.println(F("\nOther:"));
        Serial.println(F("  help     - Show this help message"));
        Serial.println(F("==========================\n"));
    }
    
public:
    CommandHandler(MainMotor& m1, OscillationMotor& m2, SequenceStateMachine& seq, PositionManager& pm)
        : motor1(m1), motor2(m2), sequence(seq), positionManager(pm),
          inputString(""), stringComplete(false), motor1CustomDegrees(0.0f) {
        inputString.reserve(50);
    }
    
    void init() {
        Serial.begin(Config::Serial::BAUD_RATE);
        Serial.println(F("\n=== FairFan Motor Controller ==="));
        Serial.println(F("Type 'help' for command list\n"));
    }
    
    void update() {
        // Read serial input from Serial0 (USB or ESP32)
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
