// ============================================================================
// NAVSTRA — sensors/bme.h
// BME280 environmental sensor (Section 5.5 / 6.6). Low sample rate.
// ============================================================================
#pragma once
#include "nav_types.h"

bool bmeInit();
void bmeUpdate(RawSensors &out);
