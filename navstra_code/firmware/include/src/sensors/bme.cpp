// ============================================================================
// NAVSTRA — sensors/bme.cpp
// ============================================================================
#include "bme.h"
#include "config.h"
#include <Adafruit_BME280.h>

static Adafruit_BME280 bme;

bool bmeInit() {
  return bme.begin(BME280_I2C_ADDR);
}

void bmeUpdate(RawSensors &out) {
  out.tempC       = bme.readTemperature();
  out.humidityPct = bme.readHumidity();
  out.pressureHpa = bme.readPressure() / 100.0f;
}
