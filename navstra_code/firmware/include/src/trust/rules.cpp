// ============================================================================
// NAVSTRA — trust/rules.cpp
// ============================================================================
#include "rules.h"
#include "config.h"
#include <Arduino.h>
#include <math.h>

static void push(RuleResult *out, int &n, int max, bool fired, Severity sev,
                  const char *id, const char *desc, float trig) {
  if (!fired || n >= max) return;
  out[n].fired = true;
  out[n].severity = sev;
  out[n].ruleId = id;
  out[n].description = desc;
  out[n].triggerValue = trig;
  n++;
}

int evaluateTrustRules(const RawSensors &raw, const NavState &nav,
                        RuleHistory &history, RuleResult *outResults, int maxResults) {
  int n = 0;

  // NAV-R01 — GNSS/DR residual (Section 8.3)
  push(outResults, n, maxResults,
       nav.gnssDrResidualM > NAVR01_RESIDUAL_M, Severity::HIGH,
       "NAV-R01", "GNSS/DR residual exceeds threshold", nav.gnssDrResidualM);

  // NAV-R02 — Position jump: implied speed between consecutive fixes
  // exceeds any physically plausible platform speed. `gpsSpeedMps` here is
  // used as a proxy for "implied speed since last fix" — a full
  // implementation compares consecutive fix coordinates directly; see
  // Appendix B note for the complete version wired into main.cpp.
  push(outResults, n, maxResults,
       raw.gpsFixValid && raw.gpsSpeedMps > NAVR02_JUMP_MAX_SPEED_MPS, Severity::HIGH,
       "NAV-R02", "Position jump implies implausible speed", raw.gpsSpeedMps);

  // NAV-R03 — Implausible velocity
  push(outResults, n, maxResults,
       raw.gpsFixValid && raw.gpsSpeedMps > NAVR03_MAX_PLAUSIBLE_SPEED, Severity::HIGH,
       "NAV-R03", "GNSS-reported speed exceeds plausible maximum", raw.gpsSpeedMps);

  // NAV-R04 — DOP anomaly
  push(outResults, n, maxResults,
       raw.gpsFixValid && raw.gpsHdop > NAVR04_HDOP_POOR, Severity::MEDIUM,
       "NAV-R04", "HDOP abnormally poor for a reported fix", raw.gpsHdop);

  // NAV-R05 — SNR anomaly (suspiciously uniform SNR across satellites)
  push(outResults, n, maxResults,
       raw.gpsFixValid && raw.gpsSatellites >= 4 &&
           raw.gpsSnrStdDev < NAVR05_SNR_STDDEV_MIN, Severity::MEDIUM,
       "NAV-R05", "Per-satellite SNR abnormally uniform", raw.gpsSnrStdDev);

  // NAV-R06 — NMEA integrity
  {
    uint32_t total = raw.nmeaParseOkCount + raw.nmeaParseFailCount;
    float failRate = total > 0 ? (float)raw.nmeaParseFailCount / (float)total : 0.0f;
    push(outResults, n, maxResults,
         total > 10 && failRate > NAVR06_MAX_PARSE_FAIL_RATE, Severity::LOW,
         "NAV-R06", "NMEA checksum/parse failure rate elevated", failRate);
  }

  // NAV-R07 — Sensor disagreement (GNSS course-over-ground vs AHRS heading)
  if (raw.gpsFixValid && raw.gpsSpeedMps > 1.0f) {
    float diff = fabsf(raw.gpsCourseDeg - nav.headingDeg);
    if (diff > 180.0f) diff = 360.0f - diff;
    if (diff > NAVR07_HEADING_DISAGREE_DEG) {
      if (history.lastHeadingDisagreeStartMs < 0) {
        history.lastHeadingDisagreeStartMs = millis();
      } else if (millis() - history.lastHeadingDisagreeStartMs > NAVR07_DWELL_MS) {
        push(outResults, n, maxResults, true, Severity::MEDIUM,
             "NAV-R07", "AHRS heading disagrees with GNSS course-over-ground", diff);
      }
    } else {
      history.lastHeadingDisagreeStartMs = -1;
    }
  } else {
    history.lastHeadingDisagreeStartMs = -1;
  }

  // NAV-R08 — GNSS loss
  push(outResults, n, maxResults,
       !raw.gpsFixValid || raw.gpsSatellites < NAVR08_MIN_SATELLITES, Severity::MEDIUM,
       "NAV-R08", "GNSS fix lost or satellite count below minimum", raw.gpsSatellites);

  // NAV-R09 — Timestamp anomaly (placeholder hook: compare GPS time to
  // ESP32 free-running clock once RTC/time sync is wired up in main.cpp)
  // push(outResults, n, maxResults, timeSkewSeconds > NAVR09_TIME_SKEW_S, ...);

  // NAV-R10 — Telemetry/MQTT integrity
  push(outResults, n, maxResults,
       history.lastMqttOkMs != 0 &&
           (millis() - history.lastMqttOkMs) > NAVR10_MQTT_TIMEOUT_MS, Severity::MEDIUM,
       "NAV-R10", "MQTT telemetry channel timeout", (float)(millis() - history.lastMqttOkMs));

  return n;
}
