// TFT_eSPI User_Setup.h Configuration for ESP32-C6-LCD-1.47
// 
// INSTALLATION INSTRUCTIONS:
// After PlatformIO downloads the TFT_eSPI library (during first build),
// this file needs to be copied to the library folder.
//
// AUTOMATIC METHOD (run after first build attempt):
//   cd esp32
//   cp User_Setup.h .pio/libdeps/esp32-c6-devkitc-1/TFT_eSPI/User_Setup.h
//
// MANUAL METHOD:
//   1. Build project once (it will fail, but downloads libraries)
//   2. Find TFT_eSPI in: .pio/libdeps/esp32-c6-devkitc-1/TFT_eSPI/
//   3. Copy this file there as User_Setup.h
//   4. Build again

#ifndef USER_SETUP_LOADED
#define USER_SETUP_LOADED

// Driver selection
#define ST7789_DRIVER

// Display size
#define TFT_WIDTH  172
#define TFT_HEIGHT 320

// ESP32-C6 Pin Configuration (adjust based on your specific board)
#define TFT_MOSI 6   // SPI MOSI
#define TFT_SCLK 7   // SPI Clock
#define TFT_CS   10  // Chip select
#define TFT_DC   8   // Data/Command
#define TFT_RST  9   // Reset
#define TFT_BL   3   // Backlight control

// SPI frequency
#define SPI_FREQUENCY  40000000  // 40MHz

// Optional: Enable backlight control
#define TFT_BACKLIGHT_ON HIGH

// Color order (may need adjustment)
#define TFT_RGB_ORDER TFT_BGR

// Fonts to load (comment out unused ones to save memory)
#define LOAD_GLCD   // Font 1. Original Adafruit 8 pixel font needs ~1820 bytes in FLASH
#define LOAD_FONT2  // Font 2. Small 16 pixel high font, needs ~3534 bytes in FLASH, 96 characters
#define LOAD_FONT4  // Font 4. Medium 26 pixel high font, needs ~5848 bytes in FLASH, 96 characters
#define LOAD_FONT6  // Font 6. Large 48 pixel font, needs ~2666 bytes in FLASH, only characters 1234567890:-.apm
#define LOAD_FONT7  // Font 7. 7 segment 48 pixel font, needs ~2438 bytes in FLASH, only characters 1234567890:.
#define LOAD_FONT8  // Font 8. Large 75 pixel font needs ~3256 bytes in FLASH, only characters 1234567890:-.
#define LOAD_GFXFF  // FreeFonts. Include access to the 48 Adafruit_GFX free fonts FF1 to FF48 and custom fonts

// Smooth font rendering
#define SMOOTH_FONT

#endif // USER_SETUP_LOADED
