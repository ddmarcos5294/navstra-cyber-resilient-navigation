// ============================================================================
// NAVSTRA — telemetry/sd_logger.cpp
// ============================================================================
#include "sd_logger.h"
#include "config.h"
#include <SD.h>
#include <SPI.h>

static bool sdOk = false;
static File navLogFile;
static File eventLogFile;

static const char *NAV_LOG_PATH = "/navstra_nav.csv";
static const char *EVENT_LOG_PATH = "/navstra_events.csv";

bool sdLoggerInit() {
  SPI.begin(PIN_SD_SCK, PIN_SD_MISO, PIN_SD_MOSI, PIN_SD_CS);
  sdOk = SD.begin(PIN_SD_CS);
  if (!sdOk) return false;

  bool navExists = SD.exists(NAV_LOG_PATH);
  navLogFile = SD.open(NAV_LOG_PATH, FILE_APPEND);
  if (navLogFile && !navExists) {
    navLogFile.println("timestamp_ms,lat,lon,heading_deg,speed_mps,mode,trust_score,residual_m");
  }

  bool eventExists = SD.exists(EVENT_LOG_PATH);
  eventLogFile = SD.open(EVENT_LOG_PATH, FILE_APPEND);
  if (eventLogFile && !eventExists) {
    eventLogFile.println("timestamp_ms,rule_id,severity,description,trigger_value,score_after");
  }

  return navLogFile && eventLogFile;
}

bool sdLoggerIsOk() {
  return sdOk && navLogFile && eventLogFile;
}

void sdLogNavState(const NavState &nav) {
  if (!sdLoggerIsOk()) return;
  navLogFile.printf("%lu,%.6f,%.6f,%.2f,%.2f,%s,%d,%.2f\n",
                     (unsigned long)nav.timestampMs, nav.lat, nav.lon,
                     nav.headingDeg, nav.speedMps, navModeToString(nav.mode),
                     nav.trustScore, nav.gnssDrResidualM);
}

void sdLogRuleEvent(const RuleResult &rule, int scoreAfter) {
  if (!sdLoggerIsOk()) return;
  const char *sevStr = "LOW";
  if (rule.severity == Severity::HIGH) sevStr = "HIGH";
  else if (rule.severity == Severity::MEDIUM) sevStr = "MEDIUM";

  eventLogFile.printf("%lu,%s,%s,%s,%.3f,%d\n",
                       (unsigned long)millis(), rule.ruleId, sevStr,
                       rule.description, rule.triggerValue, scoreAfter);
}

void sdLoggerFlush() {
  if (!sdLoggerIsOk()) return;
  navLogFile.flush();
  eventLogFile.flush();
}
