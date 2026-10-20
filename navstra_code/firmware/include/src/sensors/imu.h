// ============================================================================
// NAVSTRA — sensors/imu.h
// MEMS IMU (accel + gyro) acquisition and calibration (Report Section 6.7).
// ============================================================================
#pragma once
#include "nav_types.h"

bool imuInit();
// Averages a stationary sample window to estimate gyro bias. Call once at
// boot with the unit held still (Report Section 6.7).
void imuCalibrateGyroBias(uint16_t numSamples = 200);
void imuUpdate(RawSensors &out);
