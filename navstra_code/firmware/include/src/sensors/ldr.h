// ============================================================================
// NAVSTRA — sensors/ldr.h
// Ambient light sensor (auxiliary logging channel, Section 5.6).
// ============================================================================
#pragma once
#include "nav_types.h"

void ldrInit();
void ldrUpdate(RawSensors &out);
