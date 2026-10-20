// ============================================================================
// NAVSTRA — sensors/gps.cpp
// ============================================================================
#include "gps.h"
#include "config.h"
#include <HardwareSerial.h>
#include <TinyGPSPlus.h>

static HardwareSerial gpsSerial(2); // ESP32 UART2
static TinyGPSPlus gps;

void gpsInit() {
  gpsSerial.begin(GPS_BAUD, SERIAL_8N1, PIN_GPS_RX, PIN_GPS_TX);
}

static float computeSnrStats(float &avgOut) {
  // TinyGPSPlus does not expose per-satellite SNR directly without a custom
  // NMEA sentence handler (GSV). For a production build, register a custom
  // TinyGPSCustom set for GSV SNR fields. Here we expose the aggregate hook
  // so the Trust Engine (NAV-R05) has a defined integration point; replace
  // this stub with real GSV parsing during hardware bring-up.
  avgOut = 0.0f;
  return 0.0f;
}

void gpsUpdate(RawSensors &out) {
  while (gpsSerial.available() > 0) {
    char c = gpsSerial.read();
    bool encoded = gps.encode(c);
    if (c == '\n') {
      // A full sentence boundary just passed; TinyGPSPlus internally
      // validates checksums and silently drops bad sentences, so we
      // approximate parse-ok/fail counts via the library's pass-through
      // sentence counter versus checksum-fail counter.
      out.nmeaParseOkCount = gps.passedChecksum();
      out.nmeaParseFailCount = gps.failedChecksum();
    }
  }

  if (gps.location.isValid() && gps.location.isUpdated()) {
    out.gpsFixValid = true;
    out.gpsLat = gps.location.lat();
    out.gpsLon = gps.location.lng();
    out.gpsFixAgeMs = gps.location.age();
  } else if (gps.location.age() > 3000 || gps.location.age() == (uint32_t)-1) {
    out.gpsFixValid = false;
  }

  if (gps.altitude.isValid())   out.gpsAltitude  = gps.altitude.meters();
  if (gps.speed.isValid())      out.gpsSpeedMps  = gps.speed.mps();
  if (gps.course.isValid())     out.gpsCourseDeg = gps.course.deg();
  if (gps.satellites.isValid()) out.gpsSatellites = gps.satellites.value();
  if (gps.hdop.isValid())       out.gpsHdop      = gps.hdop.hdop();

  float snrAvg;
  float snrStd = computeSnrStats(snrAvg);
  out.gpsSnrAvg = snrAvg;
  out.gpsSnrStdDev = snrStd;

  out.timestampMs = millis();
}
