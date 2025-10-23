// Simple TTGO T-Display test to verify display hardware
#include <TFT_eSPI.h>

TFT_eSPI tft = TFT_eSPI();

void setup() {
    Serial.begin(115200);
    Serial.println("TTGO T-Display Test");
    
    // Initialize display
    tft.init();
    tft.setRotation(1);  // Landscape mode
    
    // Turn on backlight (GPIO 4)
    pinMode(4, OUTPUT);
    digitalWrite(4, HIGH);
    Serial.println("Backlight ON");
    
    // Fill screen with colors
    Serial.println("Testing display...");
    
    tft.fillScreen(TFT_RED);
    delay(1000);
    
    tft.fillScreen(TFT_GREEN);
    delay(1000);
    
    tft.fillScreen(TFT_BLUE);
    delay(1000);
    
    tft.fillScreen(TFT_BLACK);
    
    // Draw text
    tft.setTextColor(TFT_WHITE, TFT_BLACK);
    tft.setTextSize(2);
    tft.setCursor(10, 10);
    tft.println("TTGO T-Display");
    tft.setCursor(10, 40);
    tft.println("Test OK!");
    
    Serial.println("Display test complete!");
}

void loop() {
    delay(1000);
}
