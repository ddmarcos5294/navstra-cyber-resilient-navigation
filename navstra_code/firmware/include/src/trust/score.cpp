// ============================================================================
// NAVSTRA — trust/score.cpp
// ============================================================================
#include "score.h"
#include "config.h"
#include <algorithm>

int TrustEngine::updateScore(const RuleResult *firedRules, int numFired, float dtSeconds) {
  if (numFired == 0) {
    // Gradual recovery (Section 8.6) — avoids instantaneous jumps back to
    // full trust immediately after a single clean cycle.
    score_ += (int)(RECOVERY_RATE_PER_SEC * dtSeconds);
  } else {
    for (int i = 0; i < numFired; i++) {
      switch (firedRules[i].severity) {
        case Severity::HIGH:   score_ -= PENALTY_HIGH_SEVERITY;   break;
        case Severity::MEDIUM: score_ -= PENALTY_MEDIUM_SEVERITY; break;
        case Severity::LOW:    score_ -= PENALTY_LOW_SEVERITY;    break;
        default: break;
      }
    }
  }
  score_ = std::max(TRUST_SCORE_MIN, std::min(TRUST_SCORE_MAX, score_));
  return score_;
}

NavMode TrustEngine::updateMode(int score) {
  // Hysteresis-gated thresholds (Section 8.6 / Figure, state diagram
  // Section 4.11): recovering to a higher-trust mode requires a stricter
  // threshold than dropping out of it, to prevent mode-chattering.
  switch (mode_) {
    case NavMode::HIGH_TRUST:
      if (score < TRUST_HIGH_EXIT) {
        mode_ = (score < TRUST_MEDIUM_EXIT) ? NavMode::LOW_TRUST : NavMode::MEDIUM_TRUST;
      }
      break;
    case NavMode::MEDIUM_TRUST:
      if (score >= TRUST_HIGH_ENTER) {
        mode_ = NavMode::HIGH_TRUST;
      } else if (score < TRUST_MEDIUM_EXIT) {
        mode_ = NavMode::LOW_TRUST;
      }
      break;
    case NavMode::LOW_TRUST:
      if (score >= TRUST_MEDIUM_ENTER) {
        mode_ = NavMode::MEDIUM_TRUST;
      }
      break;
  }
  return mode_;
}

float TrustEngine::modeToRScale(NavMode mode) {
  switch (mode) {
    case NavMode::HIGH_TRUST:   return 1.0f;
    case NavMode::MEDIUM_TRUST: return 8.0f;
    case NavMode::LOW_TRUST:    return 1000.0f; // effectively ignore GNSS
  }
  return 1.0f;
}
