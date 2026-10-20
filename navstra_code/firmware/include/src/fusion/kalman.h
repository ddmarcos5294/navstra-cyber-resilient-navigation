// ============================================================================
// NAVSTRA — fusion/kalman.h
// Simplified, loosely-coupled 2D position/velocity Kalman filter fusing
// dead-reckoned prediction with GNSS position measurements (Section 7.7).
// Measurement noise R is set at runtime by the caller based on the current
// Navigation Trust Score, so the filter leans on dead reckoning when GNSS
// looks untrustworthy (this is the link between Chapter 7 and Chapter 8).
// ============================================================================
#pragma once

struct KalmanState {
  float px = 0.0f, py = 0.0f;  // fused local ENU position, metres
  float vx = 0.0f, vy = 0.0f;  // fused local ENU velocity, m/s
};

class PositionKalmanFilter {
 public:
  PositionKalmanFilter(float processNoiseQ, float measurementNoiseRBase);

  // Predict step: advance the filter using the dead-reckoning velocity
  // estimate as the control input (Section 7.7, prediction equation).
  void predict(float drVx, float drVy, float dtSeconds);

  // Update step: correct with a GNSS position measurement (in the same
  // local ENU frame as the filter state). `rScale` >= 1 inflates the
  // measurement-noise covariance R to reduce trust in a suspect GNSS fix
  // (rScale = 1.0 at full HIGH TRUST, larger under MEDIUM/LOW TRUST).
  // Returns the pre-update residual distance (metres), which the Trust
  // Engine consumes directly for NAV-R01 (Section 7.8, 8.3).
  float update(float gnssPx, float gnssPy, float rScale);

  KalmanState getState() const { return state_; }
  void reset(float px, float py, float vx, float vy);

 private:
  KalmanState state_;
  float p_ = 1.0f;       // simplified scalar covariance proxy (isotropic)
  float q_;              // process noise
  float rBase_;           // baseline measurement noise
};
