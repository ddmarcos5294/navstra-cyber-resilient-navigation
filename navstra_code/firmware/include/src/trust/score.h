// ============================================================================
// NAVSTRA — trust/score.h
// Navigation Trust Score computation, hysteresis-gated mode switching, and
// the mapping from mode to Kalman measurement-noise scale (Section 8.6/8.7).
// ============================================================================
#pragma once
#include "nav_types.h"
#include "rules.h"

class TrustEngine {
 public:
  // Applies penalties for every fired rule this cycle, then gradually
  // recovers the score toward 100 when no rules are firing. Updates and
  // returns the current score (0-100), and evaluates hysteresis to decide
  // whether `mode` should change.
  int updateScore(const RuleResult *firedRules, int numFired, float dtSeconds);

  NavMode updateMode(int score);

  // Measurement-noise scale factor for the Kalman filter (Section 8.7):
  // 1.0 at HIGH TRUST, larger under MEDIUM/LOW TRUST so GNSS is
  // down-weighted relative to dead reckoning.
  static float modeToRScale(NavMode mode);

  int getScore() const { return score_; }
  NavMode getMode() const { return mode_; }

 private:
  int score_ = 100;
  NavMode mode_ = NavMode::HIGH_TRUST;
};
