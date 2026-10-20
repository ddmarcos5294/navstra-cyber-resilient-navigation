// ============================================================================
// NAVSTRA — trust/rules.h
// Detection rules NAV-R01 .. NAV-R10 (Report Section 8.3 / Table 8.2).
// Each rule is a pure function of the current RawSensors / NavState plus a
// small amount of rolling history, returning whether it fired and at what
// severity. This keeps the rule set auditable and independently testable
// (Report Section 4.12, Chapter 10).
// ============================================================================
#pragma once
#include "nav_types.h"

enum class Severity : uint8_t { NONE = 0, LOW, MEDIUM, HIGH };

struct RuleResult {
  bool fired = false;
  Severity severity = Severity::NONE;
  const char *ruleId = "";
  const char *description = "";
  float triggerValue = 0.0f;
};

// Rolling history needed across calls (kept outside RuleResult so callers
// can persist it between evaluation cycles).
struct RuleHistory {
  float lastHeadingDisagreeStartMs = -1;  // NAV-R07 dwell timer
  uint32_t lastMqttOkMs = 0;              // NAV-R10 watchdog
};

// Evaluate every rule against the current sensor/nav snapshot. Writes up to
// maxResults RuleResult entries into `outResults` (fired rules only, in
// NAV-R01..R10 order) and returns the count. This mirrors the report's
// Table 8.1 threat model 1:1.
int evaluateTrustRules(const RawSensors &raw,
                        const NavState &nav,
                        RuleHistory &history,
                        RuleResult *outResults,
                        int maxResults);
