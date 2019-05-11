#ifndef PINS_DISPLAY_H
#define PINS_DISPLAY_H

// For optimized ILI9341_t3 library
#define DISPLAY_TFT_DC       9
#define DISPLAY_TFT_CS      10
#define DISPLAY_TFT_RST      6  // 255 = unused, then connect to 3.3V
#define DISPLAY_TFT_MOSI    11
#define DISPLAY_TFT_MISO    12
#define DISPLAY_TFT_SCLK    13

// The Rest
#define DISPLAY_TFT_BACKLIGHT 5

#endif