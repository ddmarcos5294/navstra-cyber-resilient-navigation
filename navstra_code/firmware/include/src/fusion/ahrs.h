// ============================================================================
// NAVSTRA — fusion/ahrs.h
// Madgwick gradient-descent AHRS filter (Report Section 7.5).
// Produces a quaternion orientation estimate from gyro + accel + mag.
// ============================================================================
#pragma once

class AhrsFilter {
 public:
  explicit AhrsFilter(float beta = 0.1f, float sampleFreqHz = 50.0f);

  // gx,gy,gz in deg/s; ax,ay,az in g (any consistent unit ok, normalised
  // internally); mx,my,mz in any consistent unit (normalised internally).
  void update(float gx, float gy, float gz,
              float ax, float ay, float az,
              float mx, float my, float mz,
              float dtSeconds);

  float getRollDeg() const;
  float getPitchDeg() const;
  float getYawDeg() const;   // heading, magnetic north referenced

  void getQuaternion(float &w, float &x, float &y, float &z) const;

 private:
  float beta_;
  float sampleFreq_;
  float q0_ = 1, q1_ = 0, q2_ = 0, q3_ = 0;

  static float invSqrt(float x);
};
