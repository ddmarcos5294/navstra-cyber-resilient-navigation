// ============================================================================
// NAVSTRA — sensors/gps.h
// GNSS UART acquisition and NMEA parsing (Report Section 6.6).
// ============================================================================
#pragma once
#include "nav_types.h"

void gpsInit();
// Reads all available UART bytes, feeds the NMEA parser, and updates `out`
// in place. Increments nmeaParseOkCount / nmeaParseFailCount as sentences
// are processed. Non-blocking — call frequently from the acquisition task.
void gpsUpdate(RawSensors &out);
