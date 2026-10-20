// ============================================================================
// NAVSTRA — nav_types.h
// Shared data structures passed between acquisition, fusion, trust and
// telemetry tasks (Report Section 6.5).
// ============================================================================
#pragma once
#include <Arduino.h>

enum class NavMode : uint8_t {
  HIGH_TRUST = 0,
  MEDIUM_TRUST = 1,
  LOW_TRUST = 2
};

inline const char* navModeToString(NavMode m) {
  switch (m) {
    case NavMode::HIGH_TRUST:   return "HIGH_TRUST";
    case NavMode::MEDIUM_TRUST: return "MEDIUM_TRUST";
    case NavMode::LOW_TRUST:    return "LOW_TRUST";
  }
  return "UNKNOWN";
}

// Raw sensor snapshot, updated by the acquisition task.
struct RawSensors {
  // GPS / GNSS
  bool     gpsFixValid   = false;
  double   gpsLat        = 0.0;
  double   gpsLon        = 0.0;
  float    gpsAltitude   = 0.0f;
  float    gpsSpeedMps   = 0.0f;
  float    gpsCourseDeg  = 0.0f;
  uint8_t  gpsSatellites = 0;
  float    gpsHdop       = 99.9f;
  float    gpsSnrAvg     = 0.0f;
  float    gpsSnrStdDev  = 0.0f;
  uint32_t gpsFixAgeMs   = 0xFFFFFFFF;
  uint32_t nmeaParseFailCount = 0;
  uint32_t nmeaParseOkCount   = 0;

  // IMU
  float axG = 0, ayG = 0, azG = 0;         // accel, g
  float gxDps = 0, gyDps = 0, gzDps = 0;   // gyro, deg/s

  // Magnetometer
  float mxUt = 0, myUt = 0, mzUt = 0;      // microtesla

  // Environmental
  float tempC = 0, humidityPct = 0, pressureHpa = 0;
  int   ldrRaw = 0;

  uint32_t timestampMs = 0;
};

// Best current navigation state, produced by the fusion task.
struct NavState {
  double lat = 0.0;
  double lon = 0.0;
  float  headingDeg = 0.0f;
  float  speedMps = 0.0f;

  // Kalman internal state (simplified 2D loosely-coupled position filter)
  float  px = 0.0f, py = 0.0f;     // local ENU position, metres from reference
  float  vx = 0.0f, vy = 0.0f;     // local ENU velocity, m/s
  float  pCov = 1.0f;              // simplified scalar covariance proxy

  double refLat = 0.0, refLon = 0.0; // local-frame reference origin

  NavMode mode = NavMode::HIGH_TRUST;
  int     trustScore = 100;

  float   gnssDrResidualM = 0.0f;   // last computed residual (Section 7.8)
  uint32_t timestampMs = 0;
};

// A single Trust Engine rule-firing event (Report Section 6.5, 8.5).
struct RuleEvent {
  char     ruleId[10] = "";
  char     severity[8] = "";       // "LOW" | "MEDIUM" | "HIGH"
  char     description[64] = "";
  float    triggerValue = 0.0f;
  int      scoreAfter = 100;
  uint32_t timestampMs = 0;
};
