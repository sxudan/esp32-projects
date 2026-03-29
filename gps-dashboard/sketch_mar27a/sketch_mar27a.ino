// ESP32-WROVER-E + NEO-7M GPS on TFT_eSPI
// No User_Setup.h changes required (uses your existing TFT_eSPI config)
//
// Wiring:
// GPS TX -> GPIO27 (ESP32 RX)
// GPS RX -> GPIO26 (ESP32 TX, optional)
// GPS GND -> ESP32 GND
// GPS VCC -> module-required supply (often 5V on breakout boards)

#include <SPI.h>
#include <TFT_eSPI.h>
#include <TinyGPS++.h>

#define GPS_RX 27
#define GPS_TX 26
#define MONITOR_BAUD 115200
#define GPS_BAUD 9600

HardwareSerial gpsSerial(2);
TinyGPSPlus gps;
TFT_eSPI tft = TFT_eSPI();

unsigned long lastUiMs = 0;

const uint16_t BG = TFT_BLACK;
const uint16_t FG = TFT_WHITE;
const uint16_t ACCENT = TFT_CYAN;
const uint16_t COL_OK = TFT_GREEN;
const uint16_t WARN = TFT_ORANGE;
const uint16_t ERR = TFT_RED;

void drawLabelValue(int y, const char* label, String value, uint16_t valueColor = FG) {
  tft.setTextFont(2);
  tft.setTextColor(ACCENT, BG);
  tft.setCursor(8, y);
  tft.print(label);

  tft.fillRect(120, y - 2, tft.width() - 124, 18, BG);
  tft.setTextColor(valueColor, BG);
  tft.setCursor(120, y);
  tft.print(value);
}

String twoDigits(uint8_t n) {
  if (n < 10) return "0" + String(n);
  return String(n);
}

void drawStaticUi() {
  tft.fillScreen(BG);
  tft.setTextFont(2);
  tft.setTextColor(FG, BG);
  tft.setCursor(8, 6);
  tft.print("GPS Dashboard");

  tft.drawFastHLine(0, 24, tft.width(), TFT_DARKGREY);

  drawLabelValue(34, "Status", "Starting...", WARN);
  drawLabelValue(56, "Lat", "-");
  drawLabelValue(78, "Lon", "-");
  drawLabelValue(100, "Sats", "-");
  drawLabelValue(122, "Alt(m)", "-");
  drawLabelValue(144, "Speed(km/h)", "-");
  drawLabelValue(166, "UTC", "-");
}

void setup() {
  Serial.begin(MONITOR_BAUD);
  delay(300);

  gpsSerial.begin(GPS_BAUD, SERIAL_8N1, GPS_RX, GPS_TX);

  tft.init();
  tft.setRotation(1);  // Landscape; change to 0 if your display is portrait-mounted
  drawStaticUi();

  Serial.println();
  Serial.println("GPS TFT dashboard started");
}

void loop() {
  while (gpsSerial.available()) {
    gps.encode((char)gpsSerial.read());
  }

  if (millis() - lastUiMs < 1000) return;
  lastUiMs = millis();

  if (gps.charsProcessed() < 10) {
    drawLabelValue(34, "Status", "NO UART DATA", ERR);
    drawLabelValue(56, "Lat", "-");
    drawLabelValue(78, "Lon", "-");
    drawLabelValue(100, "Sats", "-");
    drawLabelValue(122, "Alt(m)", "-");
    drawLabelValue(144, "Speed(km/h)", "-");
    drawLabelValue(166, "UTC", "-");
    Serial.println("NO_GPS_UART_DATA");
    return;
  }

  if (!gps.location.isValid()) {
    String sats = gps.satellites.isValid() ? String(gps.satellites.value()) : "?";
    drawLabelValue(34, "Status", "Searching fix...", WARN);
    drawLabelValue(56, "Lat", "-");
    drawLabelValue(78, "Lon", "-");
    drawLabelValue(100, "Sats", sats);
    drawLabelValue(122, "Alt(m)", "-");
    drawLabelValue(144, "Speed(km/h)", "-");
    drawLabelValue(166, "UTC", "-");
    Serial.println("No fix yet");
    return;
  }

  String lat = String(gps.location.lat(), 6);
  String lon = String(gps.location.lng(), 6);
  String sats = gps.satellites.isValid() ? String(gps.satellites.value()) : "?";
  String alt = gps.altitude.isValid() ? String(gps.altitude.meters(), 1) : "?";
  String spd = gps.speed.isValid() ? String(gps.speed.kmph(), 1) : "?";

  String utc = "?";
  if (gps.time.isValid()) {
    utc = twoDigits(gps.time.hour()) + ":" + twoDigits(gps.time.minute()) + ":" + twoDigits(gps.time.second());
  }

  drawLabelValue(34, "Status", "FIX OK", COL_OK);
  drawLabelValue(56, "Lat", lat);
  drawLabelValue(78, "Lon", lon);
  drawLabelValue(100, "Sats", sats);
  drawLabelValue(122, "Alt(m)", alt);
  drawLabelValue(144, "Speed(km/h)", spd);
  drawLabelValue(166, "UTC", utc);

  Serial.print("FIX | lat=");
  Serial.print(lat);
  Serial.print(" lon=");
  Serial.print(lon);
  Serial.print(" | sats=");
  Serial.print(sats);
  Serial.print(" | alt_m=");
  Serial.print(alt);
  Serial.print(" | speed_kmph=");
  Serial.print(spd);
  Serial.print(" | time_utc=");
  Serial.println(utc);
}
