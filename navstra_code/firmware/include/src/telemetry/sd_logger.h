// ============================================================================
// NAVSTRA — telemetry/sd_logger.h
// Buffered CSV logging to microSD (Report Section 6.9, 6.11).
// ============================================================================
#pragma once
#include "nav_types.h"
#include "trust/rules.h"

bool sdLoggerInit();
bool sdLoggerIsOk();

// Appends one CSV row for the current navigation state.
void sdLogNavState(const NavState &nav);

// Appends one CSV row for a fired rule event.
void sdLogRuleEvent(const RuleResult &rule, int scoreAfter);

// Flushes any buffered writes to the card. Call periodically (not every
// loop iteration) to reduce filesystem-induced jitter (Section 6.9).
void sdLoggerFlush();
