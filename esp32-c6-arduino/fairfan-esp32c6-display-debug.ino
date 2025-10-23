/**
 * FairFan ESP32-C6 Display Debug Version
 * 
 * This is a simplified version to debug display issues on ESP32-C6
 * Focus: Get the display working first, then add other features
 * 
 * @author jomaar
 * @date 2025
 */

#include <Arduino_GFX_Library.h>

// ============================================================================
// DISPLAY CONFIGURATION - DEBUGGING VERSION
// ============================================================================

// Try different pin configurations for ESP32-C6
// Configuration 1: Standard ESP32-C6 pins
#define TFT_DC    2     // Data/Command pin
#define TFT_CS    7     // Chip Select pin  
#define TFT_SCK   6     // SPI Clock pin
#define TFT_MOSI  4     // SPI MOSI pin
#define TFT_RST   1     // Reset pin
#define TFT_BL    3     // Backlight pin

// Alternative Configuration (uncomment if above doesn't work):
// #define TFT_DC    15    // Data/Command pin
// #define TFT_CS    14    // Chip Select pin
// #define TFT_SCK   7     // SPI Clock pin
// #define TFT_MOSI  6     // SPI MOSI pin
// #define TFT_RST   21    // Reset pin
// #define TFT_BL    22    // Backlight pin

// Display setup for Arduino_GFX
Arduino_DataBus *bus = new Arduino_ESP32SPI(TFT_DC, TFT_CS, TFT_SCK, TFT_MOSI, GFX_NOT_DEFINED);
Arduino_GFX *display = new Arduino_ST7789(
    bus, TFT_RST, 0 /* rotation */, true /* IPS */, 172 /* width */, 320 /* height */,
    34 /* col offset 1 */, 0 /* row offset 1 */, 34 /* col offset 2 */, 0 /* row offset 2 */
);

// Colors
#define BLACK       0x0000
#define WHITE       0xFFFF
#define RED         0xF800
#define GREEN       0x07E0
#define BLUE        0x001F
#define CYAN        0x07FF
#define MAGENTA     0xF81F
#define YELLOW      0xFFE0

void setup() {
    Serial.begin(115200);
    delay(1000);
    
    Serial.println();
    Serial.println("=== ESP32-C6 Display Debug ===");
    Serial.println();
    
    // Debug: Print pin configuration
    Serial.println("[Debug] Pin Configuration:");
    Serial.print("DC: GPIO"); Serial.println(TFT_DC);
    Serial.print("CS: GPIO"); Serial.println(TFT_CS);
    Serial.print("SCK: GPIO"); Serial.println(TFT_SCK);
    Serial.print("MOSI: GPIO"); Serial.println(TFT_MOSI);
    Serial.print("RST: GPIO"); Serial.println(TFT_RST);
    Serial.print("BL: GPIO"); Serial.println(TFT_BL);
    Serial.println();
    
    // Step 1: Initialize backlight pin
    Serial.println("[Debug] Step 1: Initializing backlight...");
    pinMode(TFT_BL, OUTPUT);
    
    // Try both backlight polarities
    Serial.println("[Debug] Testing backlight HIGH...");
    digitalWrite(TFT_BL, HIGH);  // Try active high
    delay(1000);
    
    Serial.println("[Debug] Testing backlight LOW...");
    digitalWrite(TFT_BL, LOW);   // Try active low
    delay(1000);
    
    Serial.println("[Debug] Setting backlight HIGH (assuming active high)...");
    digitalWrite(TFT_BL, HIGH);
    
    // Step 2: Initialize display
    Serial.println("[Debug] Step 2: Initializing display...");
    Serial.println("[Debug] Calling display->begin()...");
    
    if (!display->begin()) {
        Serial.println("[ERROR] Display initialization FAILED!");
        Serial.println("[Debug] Check wiring and pin connections");
        
        // Try manual reset
        Serial.println("[Debug] Trying manual reset...");
        pinMode(TFT_RST, OUTPUT);
        digitalWrite(TFT_RST, LOW);
        delay(100);
        digitalWrite(TFT_RST, HIGH);
        delay(100);
        
        // Try again
        if (!display->begin()) {
            Serial.println("[ERROR] Display still failed after reset!");
            return;
        }
    }
    
    Serial.println("[SUCCESS] Display initialized successfully!");
    
    // Step 3: Basic display test
    Serial.println("[Debug] Step 3: Testing display output...");
    
    // Fill screen with color
    Serial.println("[Debug] Filling screen RED...");
    display->fillScreen(RED);
    delay(1000);
    
    Serial.println("[Debug] Filling screen GREEN...");
    display->fillScreen(GREEN);
    delay(1000);
    
    Serial.println("[Debug] Filling screen BLUE...");
    display->fillScreen(BLUE);
    delay(1000);
    
    Serial.println("[Debug] Filling screen BLACK...");
    display->fillScreen(BLACK);
    delay(500);
    
    // Step 4: Text test
    Serial.println("[Debug] Step 4: Testing text output...");
    display->setTextColor(WHITE, BLACK);
    display->setTextSize(2);
    display->setCursor(10, 10);
    display->println("ESP32-C6");
    
    display->setTextSize(1);
    display->setCursor(10, 40);
    display->setTextColor(GREEN, BLACK);
    display->println("Display Working!");
    
    display->setCursor(10, 60);
    display->setTextColor(YELLOW, BLACK);
    display->println("Arduino_GFX Test");
    
    display->setCursor(10, 80);
    display->setTextColor(CYAN, BLACK);
    display->print("Free RAM: ");
    display->print(ESP.getFreeHeap());
    display->println(" bytes");
    
    // Step 5: Draw some shapes
    Serial.println("[Debug] Step 5: Testing graphics...");
    display->drawRect(10, 100, 150, 50, WHITE);
    display->fillCircle(85, 125, 20, MAGENTA);
    display->drawLine(10, 160, 160, 160, YELLOW);
    
    Serial.println();
    Serial.println("=== Display Test Complete ===");
    Serial.println("If you see colors and text on the display, it's working!");
    Serial.println("If display is still dark, check:");
    Serial.println("1. Wiring connections");
    Serial.println("2. Power supply (3.3V)");
    Serial.println("3. Display compatibility");
    Serial.println("4. Try different pin configuration");
    Serial.println();
}

void loop() {
    // Animate a moving dot to show display is alive
    static int x = 0;
    static int direction = 1;
    static unsigned long lastMove = 0;
    
    if (millis() - lastMove > 50) {
        // Clear previous dot
        display->fillCircle(x, 200, 3, BLACK);
        
        // Move dot
        x += direction * 2;
        if (x >= 165) direction = -1;
        if (x <= 5) direction = 1;
        
        // Draw new dot
        display->fillCircle(x, 200, 3, RED);
        
        lastMove = millis();
    }
    
    // Print status every 5 seconds
    static unsigned long lastStatus = 0;
    if (millis() - lastStatus > 5000) {
        Serial.println("[Status] Display test running... (moving dot should be visible)");
        lastStatus = millis();
    }
    
    delay(10);
}