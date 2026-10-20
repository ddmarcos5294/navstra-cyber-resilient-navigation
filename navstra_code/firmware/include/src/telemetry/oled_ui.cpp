// ============================================================================
// NAVSTRA — telemetry/oled_ui.cpp
// ============================================================================
#include "oled_ui.h"
#include "config.h"
#include <Adafruit_SSD1306.h>
#include <Adafruit_GFX.h>
#include <Wire.h>

static Adafruit_SSD1306 display(OLED_WIDTH, OLED_HEIGHT, &Wire, -1);

bool oledInit() {
  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_I2C_ADDR)) {
    return false;
  }
  display.clearDisplay();
  display.setTextColor(SSD1306_WHITE);
  display.display();
  return true;
}

void oledShowBootMessage(const char *msg) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);
  display.println("NAVSTRA");
  display.println(msg);
  display.display();
}

void oledShowStatus(const NavState &nav, bool gpsFixValid, uint8_t satellites) {
  display.clearDisplay();
  display.setTextSize(1);
  display.setCursor(0, 0);

  display.printf("Mode: %s\n", navModeToString(nav.mode));
  display.printf("Trust: %d%%\n", nav.trustScore);
  display.printf("Fix:%s Sat:%d\n", gpsFixValid ? "Y" : "N", satellites);
  display.printf("Lat:%.5f\n", nav.lat);
  display.printf("Lon:%.5f\n", nav.lon);
  display.printf("Hdg:%.0f Spd:%.1f", nav.headingDeg, nav.speedMps);

  display.display();
}
