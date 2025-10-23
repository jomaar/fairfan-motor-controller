/*
  ESP32-C6 Display Pin Test - Simple Pin Verification
  
  This sketch tests different pin configurations to help you identify
  the correct wiring for your ESP32-C6 board and display.
  
  Based on research, Arduino_GFX library doesn't have ESP32-C6 specific
  pin defaults, so we need to use the hardware SPI pins manually.
  
  ESP32-C6 Hardware SPI pins:
  - SCK:  GPIO19 (SPI_CLK)
  - MOSI: GPIO18 (SPI_MOSI)
  
  You can try different CS, DC, RST, BL pin combinations.
*/

#include <Arduino_GFX_Library.h>

// Configuration 1: ESP32-C6 Hardware SPI + Common Control Pins
#define CONFIG1_SCK  19  // ESP32-C6 Hardware SPI Clock
#define CONFIG1_MOSI 18  // ESP32-C6 Hardware SPI Data
#define CONFIG1_CS   8   // Chip Select
#define CONFIG1_DC   9   // Data/Command
#define CONFIG1_RST  10  // Reset
#define CONFIG1_BL   11  // Backlight

// Configuration 2: Alternative control pins
#define CONFIG2_SCK  19  // ESP32-C6 Hardware SPI Clock
#define CONFIG2_MOSI 18  // ESP32-C6 Hardware SPI Data
#define CONFIG2_CS   7   // Chip Select (like ESP32-C3)
#define CONFIG2_DC   2   // Data/Command (like ESP32-C3)
#define CONFIG2_RST  1   // Reset (like ESP32-C3)
#define CONFIG2_BL   3   // Backlight (like ESP32-C3)

// Configuration 3: Your original pins
#define CONFIG3_SCK  7   // Your original
#define CONFIG3_MOSI 6   // Your original  
#define CONFIG3_CS   14  // Your original
#define CONFIG3_DC   15  // Your original
#define CONFIG3_RST  21  // Your original
#define CONFIG3_BL   22  // Your original

// Choose which configuration to test (1, 2, or 3)
// Config 3 CONFIRMED WORKING by user!
#define TEST_CONFIG 3

#if TEST_CONFIG == 1
  #define TFT_SCK  CONFIG1_SCK
  #define TFT_MOSI CONFIG1_MOSI
  #define TFT_CS   CONFIG1_CS
  #define TFT_DC   CONFIG1_DC
  #define TFT_RST  CONFIG1_RST
  #define TFT_BL   CONFIG1_BL
  #define CONFIG_NAME "Config 1: ESP32-C6 Hardware SPI"
#elif TEST_CONFIG == 2
  #define TFT_SCK  CONFIG2_SCK
  #define TFT_MOSI CONFIG2_MOSI
  #define TFT_CS   CONFIG2_CS
  #define TFT_DC   CONFIG2_DC
  #define TFT_RST  CONFIG2_RST
  #define TFT_BL   CONFIG2_BL
  #define CONFIG_NAME "Config 2: Hardware SPI + C3-style pins"
#else
  #define TFT_SCK  CONFIG3_SCK
  #define TFT_MOSI CONFIG3_MOSI
  #define TFT_CS   CONFIG3_CS
  #define TFT_DC   CONFIG3_DC
  #define TFT_RST  CONFIG3_RST
  #define TFT_BL   CONFIG3_BL
  #define CONFIG_NAME "Config 3: Your original pins"
#endif

Arduino_DataBus *bus = new Arduino_ESP32SPI(TFT_DC, TFT_CS, TFT_SCK, TFT_MOSI, GFX_NOT_DEFINED, FSPI);
Arduino_GFX *display = new Arduino_ST7789(bus, TFT_RST, 0 /* rotation */, true /* IPS */, 172, 320, 34, 0);

void setup() {
  Serial.begin(115200);
  delay(2000);
  
  Serial.println("=== ESP32-C6 Display Pin Test ===");
  Serial.println(CONFIG_NAME);
  Serial.println();
  Serial.println("Pin Configuration:");
  Serial.printf("SCK:  GPIO%d\n", TFT_SCK);
  Serial.printf("MOSI: GPIO%d\n", TFT_MOSI);
  Serial.printf("CS:   GPIO%d\n", TFT_CS);
  Serial.printf("DC:   GPIO%d\n", TFT_DC);
  Serial.printf("RST:  GPIO%d\n", TFT_RST);
  Serial.printf("BL:   GPIO%d\n", TFT_BL);
  Serial.println();
  
  // Test backlight pin both ways
  Serial.println("Testing backlight pin...");
  pinMode(TFT_BL, OUTPUT);
  
  // Try HIGH first
  digitalWrite(TFT_BL, HIGH);
  Serial.println("Backlight set to HIGH - check if display lights up");
  delay(2000);
  
  // Try LOW 
  digitalWrite(TFT_BL, LOW);
  Serial.println("Backlight set to LOW - check if display lights up");
  delay(2000);
  
  // Set to the one that worked (assume HIGH for now)
  digitalWrite(TFT_BL, HIGH);
  Serial.println("Backlight set to HIGH for testing");
  
  // Initialize display
  Serial.println("Initializing display...");
  if (!display->begin()) {
    Serial.println("❌ FAILED: Display initialization failed!");
    Serial.println("Check your wiring and connections.");
    Serial.println();
    Serial.println("Troubleshooting tips:");
    Serial.println("1. Verify power connections (3.3V and GND)");
    Serial.println("2. Check SPI wiring matches pin definitions above");
    Serial.println("3. Try different backlight polarity");
    Serial.println("4. Verify display is ST7789 compatible");
    return;
  }
  
  Serial.println("✅ SUCCESS: Display initialized!");
  
  // Run display tests
  runDisplayTests();
}

void loop() {
  // Keep running tests in a loop
  runDisplayTests();
  delay(5000);
}

void runDisplayTests() {
  Serial.println("\n=== Running Display Tests ===");
  
  // Test 1: Clear screen
  Serial.println("Test 1: Clear screen (black)");
  display->fillScreen(BLACK);
  delay(1000);
  
  // Test 2: Color tests
  Serial.println("Test 2: Color test - RED");
  display->fillScreen(RED);
  delay(1000);
  
  Serial.println("Test 3: Color test - GREEN");
  display->fillScreen(GREEN);
  delay(1000);
  
  Serial.println("Test 4: Color test - BLUE");
  display->fillScreen(BLUE);
  delay(1000);
  
  Serial.println("Test 5: Color test - WHITE");
  display->fillScreen(WHITE);
  delay(1000);
  
  display->fillScreen(BLACK);
  
  // Test 3: Text
  Serial.println("Test 6: Text display");
  display->setTextColor(WHITE);
  display->setTextSize(2);
  display->setCursor(10, 10);
  display->println("ESP32-C6");
  
  display->setTextSize(1);
  display->setCursor(10, 50);
  display->println(CONFIG_NAME);
  
  display->setCursor(10, 80);
  display->printf("SCK:%d MOSI:%d", TFT_SCK, TFT_MOSI);
  
  display->setCursor(10, 100);
  display->printf("CS:%d DC:%d", TFT_CS, TFT_DC);
  
  display->setCursor(10, 120);
  display->printf("RST:%d BL:%d", TFT_RST, TFT_BL);
  
  delay(2000);
  
  // Test 4: Graphics
  Serial.println("Test 7: Graphics test");
  display->fillScreen(BLACK);
  
  // Draw some shapes
  display->drawRect(10, 10, 50, 30, WHITE);
  display->fillRect(70, 10, 30, 30, RED);
  display->drawCircle(40, 80, 20, GREEN);
  display->fillCircle(80, 80, 15, BLUE);
  
  // Draw lines
  for (int i = 0; i < 10; i++) {
    display->drawLine(120, 10 + i*5, 160, 10 + i*5, CYAN);
  }
  
  delay(2000);
  
  // Test 5: Moving dot to verify refresh
  Serial.println("Test 8: Moving dot test");
  display->fillScreen(BLACK);
  
  for (int x = 0; x < 150; x += 5) {
    display->fillCircle(x, 50, 3, WHITE);
    delay(50);
    display->fillCircle(x, 50, 3, BLACK);  // Erase
  }
  
  Serial.println("✅ All tests completed!");
  Serial.println("If you saw colors, text, and graphics, your display is working!");
  Serial.println();
}
