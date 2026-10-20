// ============================================================================
// NAVSTRA — sensors/mag.h
// Magnetometer acquisition with hard/soft-iron calibration (Section 6.7).
// ============================================================================
#pragma once
#include "nav_types.h"

bool magInit();

// Hard-iron offsets (uT) and soft-iron scale factors, obtained by rotating
// the sensor through representative orientations during bench calibration
// and fitting an ellipsoid to the recorded samples. Replace with values
// measured for the specific assembled unit.
struct MagCalibration {
  float offsetX = 0.0f, offsetY = 0.0f, offsetZ = 0.0f;
  float scaleX = 1.0f,  scaleY = 1.0f,  scaleZ = 1.0f;
};
void magSetCalibration(const MagCalibration &cal);

void magUpdate(RawSensors &out);
