#include <SPI.h>
#include <TFT_eSPI.h>
#include <XPT2046_Touchscreen.h>

// -------------------------------
// TFT + Touch wiring (ESP32)
// -------------------------------
// TFT pins come from User_Setup.h (inside this project)
// Touch controller (XPT2046) on separate SPI:
// T_IRQ -> GPIO36
// T_OUT -> GPIO39 (MISO)
// T_DIN -> GPIO32 (MOSI)
// T_CS  -> GPIO33
// T_CLK -> GPIO25

#ifndef TOUCH_IRQ_PIN
#define TOUCH_IRQ_PIN 36
#endif
#ifndef TOUCH_MISO_PIN
#define TOUCH_MISO_PIN 39
#endif
#ifndef TOUCH_MOSI_PIN
#define TOUCH_MOSI_PIN 32
#endif
#ifndef TOUCH_CLK_PIN
#define TOUCH_CLK_PIN 25
#endif
#ifndef TOUCH_CS
#define TOUCH_CS 33
#endif

// Touch calibration (adjust if your touch points are offset)
constexpr int RAW_X_MIN = 220;
constexpr int RAW_X_MAX = 3850;
constexpr int RAW_Y_MIN = 180;
constexpr int RAW_Y_MAX = 3860;

// Orientation tuning:
// If axes are swapped, set TOUCH_SWAP_XY to 1.
// If one axis is reversed, set TOUCH_INVERT_X or TOUCH_INVERT_Y to 1.
#ifndef TOUCH_SWAP_XY
#define TOUCH_SWAP_XY 0
#endif
#ifndef TOUCH_INVERT_X
#define TOUCH_INVERT_X 1
#endif
#ifndef TOUCH_INVERT_Y
#define TOUCH_INVERT_Y 1
#endif

SPIClass touchSPI(HSPI);
XPT2046_Touchscreen ts(TOUCH_CS, TOUCH_IRQ_PIN);
TFT_eSPI tft = TFT_eSPI();

enum Mode : uint8_t {
  MODE_HOME = 0,
  MODE_PAINT = 1,
  MODE_BUBBLES = 2
};

Mode mode = MODE_HOME;

uint16_t paintColor = TFT_CYAN;
bool wasPressed = false;
uint32_t lastFrameMs = 0;

struct Bubble {
  bool alive;
  float x;
  float y;
  float r;
  float dr;
  uint16_t color;
};

Bubble bubbles[18];

const uint16_t palette[] = {
  TFT_RED, TFT_GREEN, TFT_BLUE, TFT_YELLOW, TFT_CYAN, TFT_MAGENTA, TFT_WHITE
};

uint16_t randomBrightColor() {
  return palette[random(0, sizeof(palette) / sizeof(palette[0]))];
}

void drawTopBar(const char* title) {
  tft.fillRect(0, 0, tft.width(), 28, TFT_NAVY);
  tft.setTextColor(TFT_WHITE, TFT_NAVY);
  tft.setTextDatum(TL_DATUM);
  tft.drawString(title, 8, 7, 2);
}

void drawTabs() {
  const int y = tft.height() - 32;
  const int w = tft.width() / 3;
  const char* labels[3] = {"HOME", "PAINT", "BUBBLES"};

  for (int i = 0; i < 3; i++) {
    uint16_t bg = (mode == i) ? TFT_ORANGE : TFT_DARKGREY;
    tft.fillRoundRect(i * w + 4, y + 4, w - 8, 24, 6, bg);
    tft.setTextColor(TFT_BLACK, bg);
    tft.setTextDatum(MC_DATUM);
    tft.drawString(labels[i], i * w + (w / 2), y + 16, 2);
  }
}

void drawHome() {
  tft.fillScreen(TFT_BLACK);
  drawTopBar("Touch Playground");
  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.setTextDatum(TL_DATUM);
  tft.drawString("Tap tabs below to switch modes", 10, 38, 2);
  drawTabs();
}

void drawPaintScreen() {
  tft.fillScreen(TFT_BLACK);
  drawTopBar("Paint Mode");
  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.drawString("Draw with finger | Tap color | CLEAR", 8, 34, 2);

  int px = 8;
  for (uint8_t i = 0; i < sizeof(palette) / sizeof(palette[0]); i++) {
    tft.fillRoundRect(px, 58, 28, 20, 5, palette[i]);
    if (palette[i] == paintColor) {
      tft.drawRoundRect(px - 2, 56, 32, 24, 6, TFT_WHITE);
    }
    px += 34;
  }

  tft.fillRoundRect(tft.width() - 72, 56, 64, 24, 5, TFT_DARKGREY);
  tft.setTextColor(TFT_WHITE, TFT_DARKGREY);
  tft.setTextDatum(MC_DATUM);
  tft.drawString("CLEAR", tft.width() - 40, 68, 2);

  tft.drawRect(0, 86, tft.width(), tft.height() - 118, TFT_DARKGREY);
  drawTabs();
}

void drawBubblesScreen() {
  tft.fillScreen(TFT_BLACK);
  drawTopBar("Bubbles Mode");
  tft.setTextColor(TFT_LIGHTGREY, TFT_BLACK);
  tft.drawString("Tap to create exploding rings", 8, 34, 2);
  drawTabs();
  for (auto& b : bubbles) b.alive = false;
}

void switchMode(Mode next) {
  if (mode == next) return;
  mode = next;
  if (mode == MODE_HOME) drawHome();
  if (mode == MODE_PAINT) drawPaintScreen();
  if (mode == MODE_BUBBLES) drawBubblesScreen();
}

bool mapTouchToScreen(TS_Point p, int16_t& x, int16_t& y) {
  int32_t rawX = p.x;
  int32_t rawY = p.y;

  if (TOUCH_SWAP_XY) {
    int32_t tmp = rawX;
    rawX = rawY;
    rawY = tmp;
  }

  int16_t mx = TOUCH_INVERT_X
    ? map(rawX, RAW_X_MAX, RAW_X_MIN, 0, tft.width() - 1)
    : map(rawX, RAW_X_MIN, RAW_X_MAX, 0, tft.width() - 1);

  int16_t my = TOUCH_INVERT_Y
    ? map(rawY, RAW_Y_MAX, RAW_Y_MIN, 0, tft.height() - 1)
    : map(rawY, RAW_Y_MIN, RAW_Y_MAX, 0, tft.height() - 1);

  x = constrain(mx, 0, tft.width() - 1);
  y = constrain(my, 0, tft.height() - 1);
  return true;
}

void handleTabTouch(int16_t x, int16_t y) {
  if (y < tft.height() - 32) return;
  int w = tft.width() / 3;
  if (x < w) switchMode(MODE_HOME);
  else if (x < 2 * w) switchMode(MODE_PAINT);
  else switchMode(MODE_BUBBLES);
}

void handlePaintTouch(int16_t x, int16_t y, bool freshTap) {
  handleTabTouch(x, y);

  if (y >= 58 && y <= 78) {
    int px = 8;
    for (uint8_t i = 0; i < sizeof(palette) / sizeof(palette[0]); i++) {
      if (x >= px && x <= px + 28) {
        paintColor = palette[i];
        drawPaintScreen();
        return;
      }
      px += 34;
    }
  }

  if (x >= tft.width() - 72 && x <= tft.width() - 8 && y >= 56 && y <= 80) {
    tft.fillRect(1, 87, tft.width() - 2, tft.height() - 119, TFT_BLACK);
    return;
  }

  if (y > 90 && y < tft.height() - 34) {
    int r = freshTap ? 7 : 5;
    tft.fillCircle(x, y, r, paintColor);
  }
}

void spawnBubble(int16_t x, int16_t y) {
  for (auto& b : bubbles) {
    if (!b.alive) {
      b.alive = true;
      b.x = x;
      b.y = y;
      b.r = 2;
      b.dr = random(2, 5) * 0.55f;
      b.color = randomBrightColor();
      return;
    }
  }
}

void updateBubbles() {
  int top = 50;
  int bottom = tft.height() - 36;
  tft.fillRect(0, top, tft.width(), bottom - top, TFT_BLACK);

  for (auto& b : bubbles) {
    if (!b.alive) continue;
    tft.drawCircle((int)b.x, (int)b.y, (int)b.r, b.color);
    tft.drawCircle((int)b.x, (int)b.y, (int)(b.r + 1), b.color);
    b.r += b.dr;
    if (b.r > 45) b.alive = false;
  }
}

void handleBubblesTouch(int16_t x, int16_t y, bool freshTap) {
  handleTabTouch(x, y);
  if (y > 48 && y < tft.height() - 34 && freshTap) {
    for (int i = 0; i < 3; i++) spawnBubble(x + random(-10, 11), y + random(-10, 11));
  }
}

void handleHomeTouch(int16_t x, int16_t y, bool freshTap) {
  handleTabTouch(x, y);
  if (freshTap && y > 48 && y < tft.height() - 36) {
    tft.fillCircle(x, y, random(8, 18), randomBrightColor());
  }
}

void renderHomeAmbient() {
  static uint32_t lastSpark = 0;
  if (millis() - lastSpark < 120) return;
  lastSpark = millis();
  int x = random(0, tft.width());
  int y = random(48, tft.height() - 40);
  uint16_t c = palette[random(0, 6)];
  tft.drawPixel(x, y, c);
}

void setup() {
  Serial.begin(115200);
  delay(150);

  randomSeed(esp_random());

  tft.init();
  tft.setRotation(1);

  pinMode(TFT_BL, OUTPUT);
  digitalWrite(TFT_BL, TFT_BACKLIGHT_ON);

  touchSPI.begin(TOUCH_CLK_PIN, TOUCH_MISO_PIN, TOUCH_MOSI_PIN, TOUCH_CS);
  ts.begin(touchSPI);
  ts.setRotation(1);

  drawHome();
  Serial.println("Touch Playground started.");
}

void loop() {
  if (millis() - lastFrameMs > 35) {
    lastFrameMs = millis();
    if (mode == MODE_HOME) renderHomeAmbient();
    if (mode == MODE_BUBBLES) updateBubbles();
  }

  bool pressed = ts.touched();
  bool freshTap = pressed && !wasPressed;
  wasPressed = pressed;
  if (!pressed) return;

  TS_Point p = ts.getPoint();
  int16_t x = 0, y = 0;
  if (!mapTouchToScreen(p, x, y)) return;

  if (mode == MODE_HOME) handleHomeTouch(x, y, freshTap);
  if (mode == MODE_PAINT) handlePaintTouch(x, y, freshTap);
  if (mode == MODE_BUBBLES) handleBubblesTouch(x, y, freshTap);
}
