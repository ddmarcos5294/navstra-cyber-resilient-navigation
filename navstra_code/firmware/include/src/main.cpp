// ============================================================================
// NAVSTRA — main.cpp
// Task creation and scheduling (Report Section 4.6, 6.4).
//
// Pipeline: Sensors -> Acquisition -> Fusion (AHRS+DR+Kalman) ->
//           Trust Engine -> Decision (mode switch) -> Output
//           (OLED / SD / MQTT)  — see Report Figure 4.1.
// ============================================================================
#include <Arduino.h>
#include <Wire.h>
#include <math.h>

#include "config.h"
#include "nav_types.h"

#include "sensors/gps.h"
#include "sensors/imu.h"
#include "sensors/mag.h"
#include "sensors/bme.h"
#include "sensors/ldr.h"

#include "fusion/ahrs.h"
#include "fusion/dead_reckoning.h"
#include "fusion/kalman.h"

#include "trust/rules.h"
#include "trust/score.h"

#include "telemetry/mqtt_client.h"
#include "telemetry/sd_logger.h"
#include "telemetry/oled_ui.h"

// ---------------------------------------------------------------------------
// Shared state (protected by a mutex; single-writer-per-field discipline
// is used to keep this reference implementation simple — see Report
// Section 6.5 for the intended data-structure design).
// ---------------------------------------------------------------------------
static SemaphoreHandle_t stateMutex;

static RawSensors   g_raw;
static NavState     g_nav;
static RuleHistory  g_ruleHistory;

static AhrsFilter            g_ahrs(MADGWICK_BETA, 1000.0f / ACQUIRE_TASK_PERIOD_MS);
static DeadReckoner           g_dr;
static PositionKalmanFilter   g_kalman(KALMAN_Q_POSITION, KALMAN_R_GNSS_BASE);
static TrustEngine            g_trust;

static bool g_referenceSet = false;

// Local ENU conversion helpers (Report Section 7.2)
static const double EARTH_RADIUS_M = 6371000.0;

static void latLonToLocalEnu(double lat, double lon, double refLat, double refLon,
                              float &outX, float &outY) {
  double dLat = (lat - refLat) * DEG_TO_RAD;
  double dLon = (lon - refLon) * DEG_TO_RAD;
  outY = (float)(dLat * EARTH_RADIUS_M);
  outX = (float)(dLon * EARTH_RADIUS_M * cos(refLat * DEG_TO_RAD));
}

static void localEnuToLatLon(float x, float y, double refLat, double refLon,
                              double &outLat, double &outLon) {
  outLat = refLat + (y / EARTH_RADIUS_M) * RAD_TO_DEG;
  outLon = refLon + (x / (EARTH_RADIUS_M * cos(refLat * DEG_TO_RAD))) * RAD_TO_DEG;
}

// ---------------------------------------------------------------------------
// Task: Sensor Acquisition (Section 4.6, 6.6)
// ---------------------------------------------------------------------------
void acquisitionTask(void *pvParameters) {
  TickType_t lastWake = xTaskGetTickCount();
  uint32_t lastEnvSampleMs = 0;

  for (;;) {
    RawSensors local;

    xSemaphoreTake(stateMutex, portMAX_DELAY);
    local = g_raw; // seed with previous values (some sensors update slower)
    xSemaphoreGive(stateMutex);

    gpsUpdate(local);
    imuUpdate(local);
    magUpdate(local);

    if (millis() - lastEnvSampleMs > ENV_SAMPLE_PERIOD_MS) {
      bmeUpdate(local);
      ldrUpdate(local);
      lastEnvSampleMs = millis();
    }
    local.timestampMs = millis();

    xSemaphoreTake(stateMutex, portMAX_DELAY);
    g_raw = local;
    xSemaphoreGive(stateMutex);

    vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(ACQUIRE_TASK_PERIOD_MS));
  }
}

// ---------------------------------------------------------------------------
// Task: Sensor Fusion — AHRS + Dead Reckoning + Kalman (Section 7)
// ---------------------------------------------------------------------------
void fusionTask(void *pvParameters) {
  TickType_t lastWake = xTaskGetTickCount();
  uint32_t lastMs = millis();

  for (;;) {
    uint32_t nowMs = millis();
    float dt = (nowMs - lastMs) / 1000.0f;
    if (dt <= 0) dt = ACQUIRE_TASK_PERIOD_MS / 1000.0f;
    lastMs = nowMs;

    RawSensors raw;
    xSemaphoreTake(stateMutex, portMAX_DELAY);
    raw = g_raw;
    xSemaphoreGive(stateMutex);

    // --- AHRS update (Section 7.5) ---
    g_ahrs.update(raw.gxDps, raw.gyDps, raw.gzDps,
                  raw.axG, raw.ayG, raw.azG,
                  raw.mxUt, raw.myUt, raw.mzUt, dt);
    float headingDeg = g_ahrs.getYawDeg() + MAG_DECLINATION_DEG;
    if (headingDeg < 0) headingDeg += 360.0f;
    if (headingDeg >= 360.0f) headingDeg -= 360.0f;

    // --- Establish local ENU reference on first valid fix ---
    if (!g_referenceSet && raw.gpsFixValid) {
      xSemaphoreTake(stateMutex, portMAX_DELAY);
      g_nav.refLat = raw.gpsLat;
      g_nav.refLon = raw.gpsLon;
      xSemaphoreGive(stateMutex);
      g_referenceSet = true;
    }

    // --- Dead reckoning propagation (Section 7.4) ---
    // Prefer GNSS speed when a fix is available and currently trusted;
    // otherwise coast on the last known speed estimate (bounded by DR).
    float speedForDr = raw.gpsFixValid ? raw.gpsSpeedMps : g_dr.getState().vx;
    g_dr.propagate(headingDeg, speedForDr, dt);

    float residual = 0.0f;
    if (g_referenceSet) {
      DrState drState = g_dr.getState();
      g_kalman.predict(drState.vx, drState.vy, dt);

      if (raw.gpsFixValid) {
        float gnssX, gnssY;
        latLonToLocalEnu(raw.gpsLat, raw.gpsLon, g_nav.refLat, g_nav.refLon, gnssX, gnssY);
        float rScale = TrustEngine::modeToRScale(g_trust.getMode());
        residual = g_kalman.update(gnssX, gnssY, rScale);
      }
    }

    KalmanState kState = g_kalman.getState();
    double fusedLat, fusedLon;
    if (g_referenceSet) {
      localEnuToLatLon(kState.px, kState.py, g_nav.refLat, g_nav.refLon, fusedLat, fusedLon);
    } else {
      fusedLat = raw.gpsLat;
      fusedLon = raw.gpsLon;
    }

    xSemaphoreTake(stateMutex, portMAX_DELAY);
    g_nav.lat = fusedLat;
    g_nav.lon = fusedLon;
    g_nav.headingDeg = headingDeg;
    g_nav.speedMps = sqrtf(kState.vx * kState.vx + kState.vy * kState.vy);
    g_nav.px = kState.px; g_nav.py = kState.py;
    g_nav.vx = kState.vx; g_nav.vy = kState.vy;
    g_nav.gnssDrResidualM = residual;
    g_nav.timestampMs = nowMs;
    xSemaphoreGive(stateMutex);

    vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(FUSION_TASK_PERIOD_MS));
  }
}

// ---------------------------------------------------------------------------
// Task: Trust Engine — rule evaluation, score, mode switching (Chapter 8)
// ---------------------------------------------------------------------------
void trustTask(void *pvParameters) {
  TickType_t lastWake = xTaskGetTickCount();
  uint32_t lastMs = millis();
  static RuleResult fired[10];

  for (;;) {
    uint32_t nowMs = millis();
    float dt = (nowMs - lastMs) / 1000.0f;
    lastMs = nowMs;

    RawSensors raw;
    NavState nav;
    xSemaphoreTake(stateMutex, portMAX_DELAY);
    raw = g_raw;
    nav = g_nav;
    xSemaphoreGive(stateMutex);

    int numFired = evaluateTrustRules(raw, nav, g_ruleHistory, fired, 10);
    int score = g_trust.updateScore(fired, numFired, dt);
    NavMode mode = g_trust.updateMode(score);

    for (int i = 0; i < numFired; i++) {
      sdLogRuleEvent(fired[i], score);
      if (telemetryMqttIsConnected()) {
        publishAlert(fired[i], score);
      }
      if (fired[i].severity == Severity::HIGH) {
        digitalWrite(PIN_BUZZER, HIGH);
        delay(50);
        digitalWrite(PIN_BUZZER, LOW);
      }
    }

    xSemaphoreTake(stateMutex, portMAX_DELAY);
    g_nav.trustScore = score;
    g_nav.mode = mode;
    xSemaphoreGive(stateMutex);

    vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(TRUST_TASK_PERIOD_MS));
  }
}

// ---------------------------------------------------------------------------
// Task: Telemetry — OLED, SD, MQTT (Section 6.8, 6.9, 5.7)
// ---------------------------------------------------------------------------
void telemetryTask(void *pvParameters) {
  TickType_t lastWake = xTaskGetTickCount();
  uint32_t lastFlushMs = 0;
  uint32_t lastHealthMs = 0;

  for (;;) {
    telemetryMqttLoop();

    RawSensors raw;
    NavState nav;
    xSemaphoreTake(stateMutex, portMAX_DELAY);
    raw = g_raw;
    nav = g_nav;
    xSemaphoreGive(stateMutex);

    oledShowStatus(nav, raw.gpsFixValid, raw.gpsSatellites);
    sdLogNavState(nav);

    if (telemetryMqttIsConnected()) {
      publishTelemetry(nav, raw.gpsFixValid, nav.trustScore < TRUST_MEDIUM_EXIT ? "ACTIVE" : "NONE");
      g_ruleHistory.lastMqttOkMs = millis();
    }

    if (millis() - lastFlushMs > 5000) {
      sdLoggerFlush();
      lastFlushMs = millis();
    }

    if (millis() - lastHealthMs > 10000) {
      publishHealth(millis(), ESP.getFreeHeap(), sdLoggerIsOk());
      lastHealthMs = millis();
    }

    vTaskDelayUntil(&lastWake, pdMS_TO_TICKS(TELEMETRY_TASK_PERIOD_MS));
  }
}

// ---------------------------------------------------------------------------
// Setup / Loop
// ---------------------------------------------------------------------------
void setup() {
  Serial.begin(115200);
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_STATUS_LED, OUTPUT);

  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);

  stateMutex = xSemaphoreCreateMutex();

  bool oledOk = oledInit();
  if (oledOk) oledShowBootMessage("Booting...");

  gpsInit();

  bool imuOk = imuInit();
  if (imuOk) {
    if (oledOk) oledShowBootMessage("Calibrating IMU\n(hold still)");
    imuCalibrateGyroBias();
  }

  bool magOk = magInit();
  bool bmeOk = bmeInit();
  ldrInit();

  bool sdOk = sdLoggerInit();

  bool wifiOk = telemetryWifiConnect();
  bool mqttOk = wifiOk && telemetryMqttConnect();

  Serial.printf("NAVSTRA boot: OLED=%d IMU=%d MAG=%d BME=%d SD=%d WiFi=%d MQTT=%d\n",
                oledOk, imuOk, magOk, bmeOk, sdOk, wifiOk, mqttOk);

  if (oledOk) oledShowBootMessage(sdOk ? "Ready." : "WARN: SD fail");

  xTaskCreatePinnedToCore(acquisitionTask, "acquire", 4096, nullptr, 3, nullptr, 1);
  xTaskCreatePinnedToCore(fusionTask,      "fusion",  4096, nullptr, 2, nullptr, 1);
  xTaskCreatePinnedToCore(trustTask,       "trust",   4096, nullptr, 2, nullptr, 0);
  xTaskCreatePinnedToCore(telemetryTask,   "telemetry", 6144, nullptr, 1, nullptr, 0);
}

void loop() {
  // All work happens in FreeRTOS tasks (Section 4.6); the Arduino loop()
  // is intentionally left idle.
  vTaskDelay(pdMS_TO_TICKS(1000));
  digitalWrite(PIN_STATUS_LED, !digitalRead(PIN_STATUS_LED)); // heartbeat
}
