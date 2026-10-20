// ============================================================================
// NAVSTRA — sensors/imu.cpp
// ============================================================================
#include "imu.h"
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>
#include <Wire.h>

static Adafruit_MPU6050 mpu;
static float gyroBiasX = 0, gyroBiasY = 0, gyroBiasZ = 0;

bool imuInit() {
  if (!mpu.begin()) {
    return false;
  }
  mpu.setAccelerometerRange(MPU6050_RANGE_4_G);
  mpu.setGyroRange(MPU6050_RANGE_500_DEG);
  mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
  return true;
}

void imuCalibrateGyroBias(uint16_t numSamples) {
  double sx = 0, sy = 0, sz = 0;
  sensors_event_t a, g, temp;
  for (uint16_t i = 0; i < numSamples; i++) {
    mpu.getEvent(&a, &g, &temp);
    sx += g.gyro.x;
    sy += g.gyro.y;
    sz += g.gyro.z;
    delay(5);
  }
  gyroBiasX = sx / numSamples;
  gyroBiasY = sy / numSamples;
  gyroBiasZ = sz / numSamples;
}

void imuUpdate(RawSensors &out) {
  sensors_event_t a, g, temp;
  mpu.getEvent(&a, &g, &temp);

  out.axG = a.acceleration.x / 9.80665f;
  out.ayG = a.acceleration.y / 9.80665f;
  out.azG = a.acceleration.z / 9.80665f;

  // Apply bias correction; convert rad/s -> deg/s
  out.gxDps = (g.gyro.x - gyroBiasX) * 57.29578f;
  out.gyDps = (g.gyro.y - gyroBiasY) * 57.29578f;
  out.gzDps = (g.gyro.z - gyroBiasZ) * 57.29578f;
}
