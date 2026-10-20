// ============================================================================
// NAVSTRA — sensors/mag.cpp
// ============================================================================
#include "mag.h"
#include <QMC5883LCompass.h>

static QMC5883LCompass compass;
static MagCalibration g_cal;

bool magInit() {
  compass.init();
  return true;
}

void magSetCalibration(const MagCalibration &cal) {
  g_cal = cal;
}

void magUpdate(RawSensors &out) {
  compass.read();
  float rawX = (float)compass.getX();
  float rawY = (float)compass.getY();
  float rawZ = (float)compass.getZ();

  // Apply hard-iron offset then soft-iron scale (Section 6.7)
  out.mxUt = (rawX - g_cal.offsetX) * g_cal.scaleX;
  out.myUt = (rawY - g_cal.offsetY) * g_cal.scaleY;
  out.mzUt = (rawZ - g_cal.offsetZ) * g_cal.scaleZ;
}
