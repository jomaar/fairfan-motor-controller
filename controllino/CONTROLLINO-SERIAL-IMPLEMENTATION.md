# Controllino Serial Implementation Guide

This guide explains how to implement dual-serial communication on your Controllino MAXI to work with the ESP32 web interface while maintaining USB debugging capability.

## Overview

The Controllino will:
- **Always send to Serial1** (ESP32) - primary production interface
- **Conditionally send to Serial0** (USB) - optional debugging when connected
- **Accept commands from both** Serial0 and Serial1
- **Avoid blocking** when USB is disconnected

## Hardware Connections

```
Controllino MAXI          ESP32 (TTGO T-Display)
─────────────────────     ──────────────────────
Pin 18 (TX1)          →   GPIO22 (RX)
Pin 19 (RX1)          ←   GPIO21 (TX)
GND                   ─   GND
```

**Note:** The ESP32 Config.h currently uses GPIO21/22, which should connect to Controllino Serial1 (pins 18/19).

## Implementation

### 1. Setup Function

Add Serial1 initialization in your `setup()`:

```cpp
void setup() {
  // USB Serial (Serial0) - for debugging
  Serial.begin(115200);
  while (!Serial && millis() < 3000) {
    // Wait up to 3 seconds for USB serial, then continue anyway
  }
  
  // Serial1 - for ESP32 communication
  Serial1.begin(115200);
  
  Serial.println(F("FairFan Motor Controller"));
  Serial.println(F("USB Serial: Serial0"));
  Serial.println(F("ESP32 Serial: Serial1"));
  
  // ... rest of your setup code
}
```

### 2. Communication Functions

Add these helper functions to your Controllino code:

```cpp
/**
 * Send status/response to both serial ports (non-blocking)
 * Always sends to Serial1 (ESP32), conditionally to Serial0 (USB)
 */
void sendResponse(const String& message) {
  // ALWAYS send to Serial1 (ESP32) - production interface
  Serial1.println(message);
  
  // Conditionally send to Serial0 (USB) - only if space available
  // This prevents blocking when USB is disconnected
  if (Serial && Serial.availableForWrite() > message.length() + 2) {
    Serial.println(message);
  }
}

/**
 * Overload for F() macro (flash strings)
 */
void sendResponse(const __FlashStringHelper* message) {
  Serial1.println(message);
  
  if (Serial && Serial.availableForWrite() > 50) {  // Estimate for flash strings
    Serial.println(message);
  }
}

/**
 * Process incoming commands from any serial port
 */
void processSerialCommand(const String& command) {
  command.trim();  // Remove whitespace
  
  if (command.length() == 0) {
    return;  // Ignore empty commands
  }
  
  // Echo command received (for debugging)
  sendResponse("CMD: " + command);
  
  // Process commands
  if (command == "go1") {
    startMotor1();
    sendResponse("Motor 1: Started");
    
  } else if (command.startsWith("deg")) {
    // Extract degrees: "deg180" -> 180
    int degrees = command.substring(3).toInt();
    setMotor1Degrees(degrees);
    sendResponse("Motor 1: Set to " + String(degrees) + "°");
    
  } else if (command == "stop1") {
    stopMotor1();
    sendResponse("Motor 1: Stopped");
    
  } else if (command == "home") {
    homeMotor2();
    sendResponse("Motor 2: Homing");
    
  } else if (command == "stop2") {
    stopMotor2();
    sendResponse("Motor 2: Stopped");
    
  } else if (command == "seq1") {
    startSequence();
    sendResponse("Sequence: Started");
    
  } else if (command == "stopseq") {
    stopSequence();
    sendResponse("Sequence: Stopped");
    
  } else if (command == "softstop") {
    softStop();
    sendResponse("Sequence: Soft Stop");
    
  } else if (command == "stopall") {
    emergencyStop();
    sendResponse("Emergency: All Stopped");
    
  } else if (command == "sync") {
    setDirectionMode(SYNC);
    sendResponse("Direction: Synchronized");
    
  } else if (command == "opposite") {
    setDirectionMode(OPPOSITE);
    sendResponse("Direction: Opposite");
    
  } else {
    sendResponse("ERROR: Unknown command '" + command + "'");
  }
}
```

### 3. Main Loop - Command Reading

Update your `loop()` to read from both serial ports:

```cpp
void loop() {
  // Check Serial0 (USB) for commands
  if (Serial.available()) {
    String command = Serial.readStringUntil('\n');
    processSerialCommand(command);
  }
  
  // Check Serial1 (ESP32) for commands
  if (Serial1.available()) {
    String command = Serial1.readStringUntil('\n');
    processSerialCommand(command);
  }
  
  // Your existing motor control logic
  updateMotors();
  
  // Send periodic status updates (every 500ms)
  static unsigned long lastStatusUpdate = 0;
  if (millis() - lastStatusUpdate > 500) {
    sendStatusUpdate();
    lastStatusUpdate = millis();
  }
}
```

### 4. Status Updates

Send unsolicited status updates for the web interface:

```cpp
/**
 * Send periodic status updates to ESP32/USB
 */
void sendStatusUpdate() {
  // Build status message
  String status = "STATUS:";
  status += " M1=" + String(motor1.position);
  status += " M2=" + String(motor2.position);
  status += " SEQ=" + String(sequenceRunning ? "RUN" : "STOP");
  status += " MODE=" + String(directionMode == SYNC ? "SYNC" : "OPP");
  
  sendResponse(status);
}

/**
 * Send event-driven updates (call these when events occur)
 */
void notifyMotor1Complete() {
  sendResponse("EVENT: Motor 1 movement complete");
}

void notifyMotor2Homed() {
  sendResponse("EVENT: Motor 2 homing complete");
}

void notifyLimitSwitch(int switchNumber) {
  sendResponse("EVENT: Limit switch " + String(switchNumber) + " triggered");
}

void notifyError(const String& errorMsg) {
  sendResponse("ERROR: " + errorMsg);
}
```

## Testing

### 1. Test USB Communication

1. Upload code to Controllino
2. Open Arduino Serial Monitor (115200 baud)
3. Send commands: `go1`, `stop1`, `deg180`, etc.
4. Verify responses appear in Serial Monitor

### 2. Test ESP32 Communication

1. Connect ESP32 TX/RX to Controllino Serial1 (pins 18/19)
2. Power both devices
3. Connect to ESP32 web interface
4. Send commands from web interface
5. Verify responses appear in web interface
6. Check that status updates appear automatically

### 3. Test Non-blocking Behavior

1. Disconnect USB cable from Controllino
2. Verify ESP32 web interface still works normally
3. Controllino should NOT freeze or slow down
4. Reconnect USB and verify Serial Monitor shows messages again

## Protocol Specification

### Command Format
Commands are sent as plain text strings, terminated by newline (`\n`):
```
go1\n
deg180\n
stop1\n
```

### Response Format
Responses follow a structured format:
```
CMD: <echoed-command>
<category>: <message>
```

Examples:
```
CMD: go1
Motor 1: Started

CMD: deg180
Motor 1: Set to 180°

STATUS: M1=180 M2=0 SEQ=STOP MODE=SYNC

EVENT: Motor 1 movement complete

ERROR: Unknown command 'invalid'
```

## Troubleshooting

### Issue: ESP32 not receiving messages
- Check wiring: TX→RX, RX→TX (crossover)
- Verify GND connection
- Check baud rate matches (115200)
- Test with Serial1.println() directly

### Issue: Controllino freezes
- Check that you're using non-blocking sends
- Verify `availableForWrite()` check is in place
- Don't use `delay()` in command processing
- Ensure Serial buffer isn't being flooded

### Issue: Duplicate messages
- ESP32 code already handles duplicate filtering
- Each response should be unique or timestamped
- Avoid sending identical status messages rapidly

### Issue: Commands not recognized
- Check string trimming (whitespace)
- Verify newline handling
- Test command format matches exactly
- Add debug output: `Serial.println("Received: '" + command + "'");`

## Performance Considerations

### Serial Buffer Sizes
- Default Arduino Serial buffer: 64 bytes
- Keep messages under 60 bytes to avoid overflow
- For longer messages, consider splitting

### Update Frequency
- Status updates: 500ms - 1000ms (recommended)
- Event-driven updates: Immediate
- Command responses: Immediate
- Avoid flooding with >10 messages/second

### Memory Usage
- Each `String` uses heap memory
- Consider using `F()` macro for constants: `F("Motor Started")`
- Clear/reuse String objects when possible

## Advanced Options

### Option: Increase Serial Buffer Size
Add to top of your Controllino sketch:
```cpp
// Increase Serial1 buffer (before setup)
#define SERIAL_TX_BUFFER_SIZE 128
#define SERIAL_RX_BUFFER_SIZE 128
```

### Option: Message Queuing
For high-frequency updates, implement a message queue:
```cpp
CircularBuffer<String, 10> messageQueue;

void queueMessage(const String& msg) {
  if (!messageQueue.isFull()) {
    messageQueue.push(msg);
  }
}

void sendQueuedMessages() {
  while (!messageQueue.isEmpty() && Serial1.availableForWrite() > 64) {
    Serial1.println(messageQueue.shift());
  }
}
```

### Option: Acknowledgment System
For critical commands, implement ACK/NACK:
```cpp
void processSerialCommand(const String& command) {
  if (validateCommand(command)) {
    executeCommand(command);
    sendResponse("ACK: " + command);
  } else {
    sendResponse("NACK: " + command);
  }
}
```

## Summary

✅ **Do:**
- Always send to Serial1 (ESP32)
- Check `availableForWrite()` before sending to Serial0 (USB)
- Use non-blocking serial writes
- Send structured, parseable messages
- Handle commands from both serial ports

❌ **Don't:**
- Block waiting for Serial.print() to complete
- Flood serial with excessive messages (>10/sec)
- Assume USB is always connected
- Use `delay()` in command handlers
- Send unbounded string lengths

## Next Steps

1. ✅ Implement the communication functions above
2. ✅ Test USB communication first
3. ✅ Add ESP32 and test dual communication
4. ✅ Verify non-blocking behavior
5. ✅ Add your specific command handlers
6. ✅ Implement status update logic

---

**Questions or issues?** Check that:
- Baud rates match (115200)
- Wiring is correct (TX→RX crossover)
- GND is connected
- Both devices are powered
- Commands end with newline character
