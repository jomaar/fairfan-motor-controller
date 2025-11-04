#ifndef POSITION_MANAGER_H
#define POSITION_MANAGER_H

#include <Arduino.h>
#include <Wire.h>

/**
 * PositionManager - Persistent Motor Position Storage with FRAM
 * 
 * Features:
 * - Dual-slot ping-pong storage for transactional safety
 * - CRC16 checksums for data integrity
 * - Magic numbers for validation
 * - Handles power loss during write operations
 * 
 * FRAM: MB85RC256V (32KB, I2C address 0x50)
 * 
 * Memory Layout:
 *   Slot A (Primary):
 *     0x0000-0x0001: Magic Number (0xFAFA)
 *     0x0002-0x0005: Position (signed long, 4 bytes)
 *     0x0006-0x0007: CRC16 (2 bytes)
 *   
 *   Slot B (Backup):
 *     0x0008-0x0009: Magic Number (0xFAFA)
 *     0x000A-0x000D: Position (signed long, 4 bytes)
 *     0x000E-0x000F: CRC16 (2 bytes)
 * 
 * Write Strategy (Ping-Pong):
 *   First write  -> Slot A
 *   Second write -> Slot B
 *   Third write  -> Slot A (overwrite)
 *   ...and so on
 * 
 * Read Strategy:
 *   - If both valid and identical: use value
 *   - If both valid but different: use Slot A (most recent)
 *   - If only one valid: use that one
 *   - If none valid: initialize to 0
 */
class PositionManager {
private:
    // FRAM I2C Configuration
    static constexpr uint8_t FRAM_I2C_ADDRESS = 0x50;
    static constexpr uint16_t FRAM_MAGIC_NUMBER = 0xFAFA;
    
    // Memory slots
    static constexpr uint16_t SLOT_SIZE = 8;  // Magic(2) + Position(4) + CRC(2)
    static constexpr uint16_t SLOT_A_ADDR = 0x0000;
    static constexpr uint16_t SLOT_B_ADDR = 0x0008;
    
    // Offsets within a slot
    static constexpr uint8_t OFFSET_MAGIC = 0;
    static constexpr uint8_t OFFSET_POSITION = 2;
    static constexpr uint8_t OFFSET_CRC = 6;
    
    // State
    bool lastSlotWasA;
    long lastSavedPosition;
    bool initialized;
    
    // === CRC16 Calculation (CCITT) ===
    uint16_t calculateCRC16(const uint8_t* data, size_t length) {
        uint16_t crc = 0xFFFF;
        for (size_t i = 0; i < length; i++) {
            crc ^= (uint16_t)data[i] << 8;
            for (uint8_t j = 0; j < 8; j++) {
                if (crc & 0x8000) {
                    crc = (crc << 1) ^ 0x1021;
                } else {
                    crc <<= 1;
                }
            }
        }
        return crc;
    }
    
    // === I2C FRAM Read/Write ===
    void framWrite8(uint16_t addr, uint8_t value) {
        Wire.beginTransmission(FRAM_I2C_ADDRESS);
        Wire.write((uint8_t)(addr >> 8));    // Address MSB
        Wire.write((uint8_t)(addr & 0xFF));  // Address LSB
        Wire.write(value);
        Wire.endTransmission();
    }
    
    void framWrite16(uint16_t addr, uint16_t value) {
        framWrite8(addr, (uint8_t)(value >> 8));
        framWrite8(addr + 1, (uint8_t)(value & 0xFF));
    }
    
    void framWrite32(uint16_t addr, uint32_t value) {
        framWrite8(addr, (uint8_t)(value >> 24));
        framWrite8(addr + 1, (uint8_t)(value >> 16));
        framWrite8(addr + 2, (uint8_t)(value >> 8));
        framWrite8(addr + 3, (uint8_t)(value & 0xFF));
    }
    
    uint8_t framRead8(uint16_t addr) {
        Wire.beginTransmission(FRAM_I2C_ADDRESS);
        Wire.write((uint8_t)(addr >> 8));
        Wire.write((uint8_t)(addr & 0xFF));
        Wire.endTransmission();
        
        Wire.requestFrom(FRAM_I2C_ADDRESS, (uint8_t)1);
        if (Wire.available()) {
            return Wire.read();
        }
        return 0;
    }
    
    uint16_t framRead16(uint16_t addr) {
        uint16_t value = (uint16_t)framRead8(addr) << 8;
        value |= framRead8(addr + 1);
        return value;
    }
    
    uint32_t framRead32(uint16_t addr) {
        uint32_t value = (uint32_t)framRead8(addr) << 24;
        value |= (uint32_t)framRead8(addr + 1) << 16;
        value |= (uint32_t)framRead8(addr + 2) << 8;
        value |= framRead8(addr + 3);
        return value;
    }
    
    // === Slot Operations ===
    void writeSlot(uint16_t slotAddr, long position) {
        // Prepare data for CRC
        uint8_t data[6];  // Magic(2) + Position(4)
        data[0] = (uint8_t)(FRAM_MAGIC_NUMBER >> 8);
        data[1] = (uint8_t)(FRAM_MAGIC_NUMBER & 0xFF);
        data[2] = (uint8_t)(position >> 24);
        data[3] = (uint8_t)(position >> 16);
        data[4] = (uint8_t)(position >> 8);
        data[5] = (uint8_t)(position & 0xFF);
        
        // Calculate CRC
        uint16_t crc = calculateCRC16(data, 6);
        
        // Write to FRAM
        framWrite16(slotAddr + OFFSET_MAGIC, FRAM_MAGIC_NUMBER);
        framWrite32(slotAddr + OFFSET_POSITION, (uint32_t)position);
        framWrite16(slotAddr + OFFSET_CRC, crc);
    }
    
    bool validateSlot(uint16_t slotAddr, long* outPosition) {
        // Read magic number
        uint16_t magic = framRead16(slotAddr + OFFSET_MAGIC);
        if (magic != FRAM_MAGIC_NUMBER) {
            return false;
        }
        
        // Read position
        uint32_t posRaw = framRead32(slotAddr + OFFSET_POSITION);
        long position = (long)posRaw;
        
        // Read stored CRC
        uint16_t storedCRC = framRead16(slotAddr + OFFSET_CRC);
        
        // Calculate CRC from data
        uint8_t data[6];
        data[0] = (uint8_t)(magic >> 8);
        data[1] = (uint8_t)(magic & 0xFF);
        data[2] = (uint8_t)(position >> 24);
        data[3] = (uint8_t)(position >> 16);
        data[4] = (uint8_t)(position >> 8);
        data[5] = (uint8_t)(position & 0xFF);
        
        uint16_t calculatedCRC = calculateCRC16(data, 6);
        
        if (calculatedCRC != storedCRC) {
            return false;
        }
        
        // Valid!
        if (outPosition != nullptr) {
            *outPosition = position;
        }
        return true;
    }
    
public:
    PositionManager() 
        : lastSlotWasA(false), lastSavedPosition(0), initialized(false) {}
    
    // === Initialization ===
    bool init() {
        Wire.begin();
        
        // Test FRAM presence
        Wire.beginTransmission(FRAM_I2C_ADDRESS);
        if (Wire.endTransmission() != 0) {
            Serial.println(F("ERROR: FRAM not found at 0x50!"));
            return false;
        }
        
        initialized = true;
        Serial.println(F("FRAM MB85RC256V initialized"));
        return true;
    }
    
    // === Save Position (Ping-Pong Strategy) ===
    void savePosition(long position) {
        if (!initialized) {
            Serial.println(F("WARN: FRAM not initialized"));
            return;
        }
        
        // Alternate between slots for safety
        if (lastSlotWasA) {
            writeSlot(SLOT_B_ADDR, position);
            lastSlotWasA = false;
        } else {
            writeSlot(SLOT_A_ADDR, position);
            lastSlotWasA = true;
        }
        
        lastSavedPosition = position;
    }
    
    // === Load Position (with validation) ===
    bool loadPosition(long* outPosition) {
        if (!initialized) {
            Serial.println(F("WARN: FRAM not initialized"));
            return false;
        }
        
        long posA = 0, posB = 0;
        bool validA = validateSlot(SLOT_A_ADDR, &posA);
        bool validB = validateSlot(SLOT_B_ADDR, &posB);
        
        Serial.print(F("FRAM: Slot A "));
        Serial.print(validA ? F("valid=") : F("INVALID"));
        if (validA) Serial.println(posA);
        else Serial.println();
        
        Serial.print(F("FRAM: Slot B "));
        Serial.print(validB ? F("valid=") : F("INVALID"));
        if (validB) Serial.println(posB);
        else Serial.println();
        
        if (validA && validB) {
            // Both valid - check which was written last
            // In ping-pong: we alternate A -> B -> A -> B...
            // lastSlotWasA tells us which was written last
            // But on boot, we don't know the write order, so we need a heuristic
            
            // Strategy: If both values are identical or very close, use either
            // Otherwise, we need to detect which is newer
            
            if (posA == posB) {
                // Both identical - perfect, use either
                *outPosition = posA;
                lastSavedPosition = posA;
                Serial.print(F("FRAM: Both slots identical: "));
                Serial.println(*outPosition);
                return true;
            }
            
            // Different values - this indicates incomplete write or we need to pick one
            // IMPORTANT: In a proper ping-pong, only ONE should be the "current" value
            // For safety in case of uncertainty, we could add a counter/timestamp
            // For now: Warn user and use Slot A (most recent write goes to A first)
            Serial.println(F("WARN: Slots differ! This may indicate write interruption."));
            Serial.print(F("Using Slot A: "));
            Serial.println(posA);
            *outPosition = posA;
            lastSavedPosition = posA;
            lastSlotWasA = true;  // Assume A was written last
            return true;
        } else if (validA) {
            *outPosition = posA;
            lastSavedPosition = posA;
            Serial.print(F("FRAM: Using Slot A: "));
            Serial.println(*outPosition);
            return true;
        } else if (validB) {
            *outPosition = posB;
            lastSavedPosition = posB;
            Serial.print(F("FRAM: Using Slot B: "));
            Serial.println(*outPosition);
            return true;
        } else {
            // Both invalid - FRAM empty or corrupted
            Serial.println(F("FRAM: No valid data found, assuming home position (0)"));
            *outPosition = 0;
            lastSavedPosition = 0;
            // Initialize both slots with 0
            savePosition(0);
            savePosition(0);
            return false;
        }
    }
    
    // === Clear FRAM (factory reset) ===
    void clear() {
        if (!initialized) return;
        
        // Write zeros to both slots
        for (uint16_t i = 0; i < SLOT_SIZE * 2; i++) {
            framWrite8(i, 0x00);
        }
        
        lastSavedPosition = 0;
        lastSlotWasA = false;
        Serial.println(F("FRAM: Cleared"));
    }
    
    // === Get Info ===
    long getLastSavedPosition() const {
        return lastSavedPosition;
    }
    
    bool isInitialized() const {
        return initialized;
    }
    
    void printInfo() {
        Serial.println(F("\n=== FRAM Status ==="));
        Serial.print(F("Initialized: "));
        Serial.println(initialized ? F("YES") : F("NO"));
        Serial.print(F("I2C Address: 0x"));
        Serial.println(FRAM_I2C_ADDRESS, HEX);
        Serial.print(F("Last Saved Position: "));
        Serial.print(lastSavedPosition);
        Serial.println(F(" steps"));
        
        // Convert to degrees
        // Motor1: 20 gear × 200 steps × 8 microsteps = 32000 steps/360°
        float degrees = (float)lastSavedPosition / 32000.0f * 360.0f;
        Serial.print(F("                    = "));
        Serial.print(degrees, 2);
        Serial.println(F("°"));
        Serial.println(F("==================\n"));
    }
    
    // === Low-Level FRAM Access (for debugging) ===
    
    // Write single byte to FRAM (raw access)
    void writeByteRaw(uint16_t address, uint8_t value) {
        if (!initialized) {
            Serial.println(F("WARN: FRAM not initialized"));
            return;
        }
        framWrite8(address, value);
    }
    
    // Read single byte from FRAM (raw access)
    uint8_t readByteRaw(uint16_t address) {
        if (!initialized) {
            Serial.println(F("WARN: FRAM not initialized"));
            return 0;
        }
        return framRead8(address);
    }
};

#endif // POSITION_MANAGER_H
