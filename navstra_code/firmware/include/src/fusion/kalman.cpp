// ============================================================================
// NAVSTRA — fusion/kalman.cpp
// NOTE: this is intentionally a simplified *scalar-covariance* filter (a
// single isotropic covariance term shared by x/y) rather than a full 4x4
// matrix implementation, to keep the reference implementation easy to read
// and verify by hand — appropriate for the report's stated scope
// (Section 7.7: "a 2-4 state loosely-coupled ... filter is comfortably
// within ESP32 compute and memory budgets"). A full matrix EKF is a
// straightforward drop-in extension (Chapter 13, future work).
// ============================================================================
#include "kalman.h"
#include <math.h>

PositionKalmanFilter::PositionKalmanFilter(float processNoiseQ, float measurementNoiseRBase)
    : q_(processNoiseQ), rBase_(measurementNoiseRBase) {}

void PositionKalmanFilter::reset(float px, float py, float vx, float vy) {
  state_.px = px; state_.py = py;
  state_.vx = vx; state_.vy = vy;
  p_ = 1.0f;
}

void PositionKalmanFilter::predict(float drVx, float drVy, float dtSeconds) {
  // x_k- = F x_{k-1} + B u_k  (Section 7.7)
  state_.vx = drVx;
  state_.vy = drVy;
  state_.px += state_.vx * dtSeconds;
  state_.py += state_.vy * dtSeconds;

  // Covariance grows with process noise each prediction step.
  p_ += q_ * dtSeconds;
}

float PositionKalmanFilter::update(float gnssPx, float gnssPy, float rScale) {
  float r = rBase_ * rScale;

  // Innovation / residual (Section 7.8) — Euclidean distance in the local
  // ENU frame between the GNSS measurement and the current prediction.
  float dx = gnssPx - state_.px;
  float dy = gnssPy - state_.py;
  float residual = sqrtf(dx * dx + dy * dy);

  // Kalman gain: K = P / (P + R)  (scalar form of K = P H^T (H P H^T + R)^-1
  // with H = identity for a direct position measurement).
  float k = p_ / (p_ + r);

  state_.px += k * dx;
  state_.py += k * dy;

  // P_k = (1 - K) P_k-
  p_ = (1.0f - k) * p_;

  return residual;
}
