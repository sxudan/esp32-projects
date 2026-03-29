// TFT_eSPI custom setup for ESP32 + ILI9341 TFT (used by tft-touch-playground)
// Copy this file into TFT_eSPI library as User_Setup.h
// (or include it via User_Setup_Select.h)

#define USER_SETUP_INFO "ESP32_ILI9341_TOUCH_PLAYGROUND"

#define ILI9341_DRIVER

#define TFT_WIDTH  240
#define TFT_HEIGHT 320

#define TFT_MISO 12
#define TFT_MOSI 13
#define TFT_SCLK 14
#define TFT_CS   15
#define TFT_DC    2
#define TFT_RST  -1

#define TFT_BL   21
#define TFT_BACKLIGHT_ON HIGH

// Touch chip-select used by many TFT_eSPI touch examples.
#define TOUCH_CS 33

// Project touch wiring reference (XPT2046 on separate SPI bus in sketch):
#define TOUCH_IRQ_PIN  36
#define TOUCH_MISO_PIN 39
#define TOUCH_MOSI_PIN 32
#define TOUCH_CLK_PIN  25

#define LOAD_GLCD
#define LOAD_FONT2
#define LOAD_FONT4
#define LOAD_FONT6
#define LOAD_FONT7
#define LOAD_FONT8
#define LOAD_GFXFF

#define SMOOTH_FONT

#define SPI_FREQUENCY       40000000
#define SPI_READ_FREQUENCY  20000000
#define SPI_TOUCH_FREQUENCY 2500000
