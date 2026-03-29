# TFT Touch Playground (ESP32)

Fun touchscreen demo with 3 interactive modes:
- `HOME`: ambient lights + touch splashes
- `PAINT`: finger drawing with color palette + clear button
- `BUBBLES`: touch to spawn animated ring bursts

## Wiring Used

### TFT (ILI9341)
- `MISO` -> GPIO12
- `MOSI` -> GPIO13
- `SCK` -> GPIO14
- `CS` -> GPIO15
- `DC` -> GPIO2
- `RESET` -> EN/RESET (`TFT_RST = -1`)
- `LED` -> GPIO21

### Touch (XPT2046, separate SPI)
- `T_IRQ` -> GPIO36
- `T_OUT` -> GPIO39
- `T_DIN` -> GPIO32
- `T_CS` -> GPIO33
- `T_CLK` -> GPIO25

## Libraries
- `TFT_eSPI` by Bodmer
- `XPT2046_Touchscreen` by Paul Stoffregen

## Setup
1. Copy [`User_Setup.h`](/Users/sudayn/Documents/work/personal/esp32-projects/tft-touch-playground/User_Setup.h) to your TFT_eSPI library folder as `User_Setup.h`
2. Open [`tft-touch-playground.ino`](/Users/sudayn/Documents/work/personal/esp32-projects/tft-touch-playground/tft-touch-playground.ino)
3. Select your ESP32 board and upload

## If touch mapping feels wrong
Tune these values in the sketch:
- `RAW_X_MIN`, `RAW_X_MAX`
- `RAW_Y_MIN`, `RAW_Y_MAX`
