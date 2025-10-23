/**
 * User_Setup.h for TFT_eSPI Library - ESP32-C6 with 1.47" ST7789 LCD
 * 
 * IMPORTANT: This file must be copied to your TFT_eSPI library directory
 * after installing the TFT_eSPI library via Arduino IDE Library Manager.
 * 
 * Location: Arduino/libraries/TFT_eSPI/User_Setup.h
 * 
 * OR uncomment your setup in User_Setup_Select.h
 */

#ifndef USER_SETUP_H
#define USER_SETUP_H

// ############################################################################
// ESP32-C6 with 1.47" ST7789 LCD Display Configuration
// ############################################################################

// Define to load this setup
#define USER_SETUP_LOADED

// ############################################################################
// Display Driver
// ############################################################################
#define ST7789_DRIVER    // Configuration for ST7789 displays (1.47" round/square)

// ############################################################################
// ESP32-C6 Pin Configuration for 1.47" LCD
// ############################################################################
// Adjust these pins according to your specific ESP32-C6 LCD board

#define TFT_MOSI 6   // SDA/MOSI pin (check your board schematic)
#define TFT_SCLK 7   // SCL/SCLK pin (check your board schematic)
#define TFT_CS   10  // Chip Select pin (check your board schematic)
#define TFT_DC   8   // Data/Command pin (check your board schematic)
#define TFT_RST  9   // Reset pin (check your board schematic)
#define TFT_BL   3   // Backlight control pin (check your board schematic)

// Alternative pin configurations (comment out the ones above and uncomment these if needed)
// Common ESP32-C6 LCD configurations:

// Configuration 1: Common pin layout
// #define TFT_MOSI 11
// #define TFT_SCLK 12
// #define TFT_CS   13
// #define TFT_DC   14
// #define TFT_RST  15
// #define TFT_BL   16

// Configuration 2: Alternative layout
// #define TFT_MOSI 18
// #define TFT_SCLK 19
// #define TFT_CS   20
// #define TFT_DC   21
// #define TFT_RST  22
// #define TFT_BL   23

// ############################################################################
// Display Resolution
// ############################################################################
#define TFT_WIDTH  240
#define TFT_HEIGHT 240

// ############################################################################
// SPI Settings
// ############################################################################
#define SPI_FREQUENCY       27000000  // 27MHz - reduce if you have display issues
#define SPI_READ_FREQUENCY  20000000  // 20MHz for reading
#define SPI_TOUCH_FREQUENCY  2500000  // 2.5MHz for touch (if applicable)

// ############################################################################
// Display Settings
// ############################################################################
#define TFT_BACKLIGHT_ON HIGH  // Backlight control signal polarity
#define TFT_INVERSION_ON       // Enable display inversion
#define TFT_RGB_ORDER TFT_RGB  // Color order (try TFT_BGR if colors are wrong)

// ############################################################################
// Font Support
// ############################################################################
#define LOAD_GLCD    // Font 1: Original Adafruit font needs ~1820 bytes in FLASH
#define LOAD_FONT2   // Font 2: Small 16 pixel high font, needs ~3534 bytes in FLASH
#define LOAD_FONT4   // Font 4: Medium 26 pixel high font, needs ~5848 bytes in FLASH
#define LOAD_FONT6   // Font 6: Large 48 pixel high font, needs ~2666 bytes in FLASH
#define LOAD_FONT7   // Font 7: 7 segment 48 pixel high font, needs ~2438 bytes in FLASH
#define LOAD_FONT8   // Font 8: Large 75 pixel high font, needs ~3256 bytes in FLASH
#define LOAD_GFXFF   // FreeFonts - needs ~17000 bytes in FLASH

// Enable smooth fonts
#define SMOOTH_FONT

// ############################################################################
// Optional Settings
// ############################################################################
// Uncomment if you want to use DMA (faster but uses more memory)
// #define SPI_DMA_CHANNEL 2

// Uncomment to enable UTF8 support
// #define ENABLE_UTF8

#endif // USER_SETUP_H
