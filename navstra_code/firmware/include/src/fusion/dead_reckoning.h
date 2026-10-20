// ============================================================================
// NAVSTRA — fusion/dead_reckoning.h
// IMU-driven position propagation in a local ENU frame (Report Section 7.4).
// Prefers GNSS-derived speed when trusted, falling back to acceleration
// integration only when necessary, since double-integrated MEMS accel
// diverges quickly (Report Section 7.9).
// ============================================================================
#pragma once

struct DrState {
  float px = 0.0f, py = 0.0f;  // local ENU position, metres
  float vx = 0.0f, vy = 0.0f;  // local ENU velocity, m/s
};

class DeadReckoner {
 public:
  // Propagate position by dtSeconds using the current heading (deg, 0=N,
  // clockwise) and a speed estimate (m/s). If gnssSpeedTrusted is true,
  // gnssSpeedMps is used directly; otherwise fall back to a short-horizon
  // accel-integration estimate (accelForwardMps2, body-frame forward accel
  // already gravity-compensated and rotated to nav frame by the caller).
  void propagate(float headingDeg, float speedMps, float dtSeconds);

  void reset(float px = 0.0f, float py = 0.0f, float vx = 0.0f, float vy = 0.0f);
  DrState getState() const { return state_; }

 private:
  DrState state_;
};
