// ============================================================================
// NAVSTRA — telemetry/oled_ui.h
// Local status display (Report Section 5.7).
// ============================================================================
#pragma once
#include "nav_types.h"

bool oledInit();
void oledShowStatus(const NavState &nav, bool gpsFixValid, uint8_t satellites);
void oledShowBootMessage(const char *msg);
