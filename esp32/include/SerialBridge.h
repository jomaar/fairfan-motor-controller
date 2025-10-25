#ifndef SERIAL_BRIDGE_H
#define SERIAL_BRIDGE_H

#include <Arduino.h>
#include <HardwareSerial.h>
#include "Config.h"

/**
 * SerialBridge - Handles bidirectional communication with Controllino
 * 
 * This class manages the serial connection to the Controllino MAXI,
 * allowing commands to be sent and responses to be received.
 */
class SerialBridge {
private:
    HardwareSerial& controllinoSerial;
    String receiveBuffer;
    String lastResponse;
    String lastCommand;
    unsigned long lastResponseTime;
    
    // Callback for received data
    typedef void (*ResponseCallback)(const String& response);
    ResponseCallback responseCallback;
    
public:
    SerialBridge() 
        : controllinoSerial(Serial1),  // ESP32-C6 uses Serial1 (UART1)
          receiveBuffer(""),
          lastResponse(""),
          lastCommand(""),
          lastResponseTime(0),
          responseCallback(nullptr) {
        receiveBuffer.reserve(Config::System::SERIAL_BUFFER_SIZE);
    }
    
    /**
     * Initialize serial connection to Controllino
     */
    void begin() {
        controllinoSerial.begin(
            Config::Serial::BAUD_RATE,
            SERIAL_8N1,
            Config::Serial::RX_PIN,
            Config::Serial::TX_PIN
        );
        
        Serial.println(F("[Bridge] Serial connection to Controllino initialized"));
        Serial.print(F("[Bridge] RX: GPIO"));
        Serial.print(Config::Serial::RX_PIN);
        Serial.print(F(", TX: GPIO"));
        Serial.println(Config::Serial::TX_PIN);
        Serial.print(F("[Bridge] Baud Rate: "));
        Serial.println(Config::Serial::BAUD_RATE);
    }
    
    /**
     * Set callback function for received responses
     */
    void setResponseCallback(ResponseCallback callback) {
        responseCallback = callback;
    }
    
    /**
     * Send command to Controllino
     * @param command Command string to send
     * @return true if sent successfully
     */
    bool sendCommand(const String& command) {
        if (command.length() == 0) {
            return false;
        }
        
        lastCommand = command;  // Store the command
        controllinoSerial.println(command);
        Serial.print(F("[Bridge] Sent: "));
        Serial.println(command);
        
        return true;
    }
    
    /**
     * Update - call this frequently from main loop
     * Reads incoming data from Controllino
     */
    void update() {
        // Read all available data
        while (controllinoSerial.available()) {
            char c = controllinoSerial.read();
            
            if (c == '\n' || c == '\r') {
                if (receiveBuffer.length() > 0) {
                    processResponse(receiveBuffer);
                    receiveBuffer = "";
                }
            } else {
                receiveBuffer += c;
                
                // Prevent buffer overflow
                if (receiveBuffer.length() >= Config::System::SERIAL_BUFFER_SIZE) {
                    Serial.println(F("[Bridge] Warning: Buffer overflow, clearing"));
                    receiveBuffer = "";
                }
            }
        }
    }
    
    /**
     * Get last received response
     */
    String getLastResponse() const {
        return lastResponse;
    }
    
    /**
     * Get last sent command
     */
    String getLastCommand() const {
        return lastCommand;
    }
    
    /**
     * Get time of last response (millis)
     */
    unsigned long getLastResponseTime() const {
        return lastResponseTime;
    }
    
    /**
     * Check if Controllino is responding
     * @return true if response received within timeout
     */
    bool isConnected() const {
        return (millis() - lastResponseTime) < Config::Serial::TIMEOUT_MS;
    }
    
private:
    /**
     * Process a complete response line from Controllino
     */
    void processResponse(const String& response) {
        lastResponse = response;
        lastResponseTime = millis();
        
        Serial.print(F("[Bridge] Received: "));
        Serial.println(response);
        
        // Call callback if set
        if (responseCallback != nullptr) {
            responseCallback(response);
        }
    }
};

#endif // SERIAL_BRIDGE_H
