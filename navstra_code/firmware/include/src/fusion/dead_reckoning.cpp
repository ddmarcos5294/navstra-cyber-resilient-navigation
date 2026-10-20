// ============================================================================
// NAVSTRA — fusion/dead_reckoning.cpp
// ============================================================================
#include "dead_reckoning.h"
#include <math.h>

void DeadReckoner::reset(float px, float py, float vx, float vy) {
  state_.px = px; state_.py = py;
  state_.vx = vx; state_.vy = vy;
}

void DeadReckoner::propagate(float headingDeg, float speedMps, float dtSeconds) {
  // x(t) = x(t-1) + v*dt*cos(heading); y(t) = y(t-1) + v*dt*sin(heading)
  // (Report Section 7.4, equirectangular local ENU frame)
  float headingRad = headingDeg * 0.0174533f;
  state_.vx = speedMps * sinf(headingRad); // East component
  state_.vy = speedMps * cosf(headingRad); // North component
  state_.px += state_.vx * dtSeconds;
  state_.py += state_.vy * dtSeconds;
}
