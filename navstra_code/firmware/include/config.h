// ============================================================================
// NAVSTRA — config.h
// Central pin map, tunable thresholds and (local, gitignored) credentials.
// See Report Section 6.10 — do not commit real Wi-Fi/MQTT credentials.
// ============================================================================
#pragma once

// ---------------------------------------------------------------------------
// Wi-Fi / MQTT credentials — replace with a local, gitignored secrets.h in
// a real deployment. Placeholder values only.
// ---------------------------------------------------------------------------
#define WIFI_SSID         "YOUR_WIFI_SSID"
#define WIFI_PASSWORD     "YOUR_WIFI_PASSWORD"
#define MQTT_BROKER_HOST  "192.168.1.100"
#define MQTT_BROKER_PORT  1883
#define MQTT_CLIENT_ID    "navstra-esp32-01"
#define MQTT_USERNAME     ""     // leave blank if broker has no auth
#define MQTT_PASSWORD     ""

// MQTT topics (Report Section 6.8)
#define TOPIC_TELEMETRY   "navstra/telemetry"
#define TOPIC_ALERTS      "navstra/alerts"
#define TOPIC_HEALTH      "navstra/health"

// ---------------------------------------------------------------------------
// Pin configuration (Report Table 5.1)
// ---------------------------------------------------------------------------
#define PIN_GPS_RX        16   // ESP32 RX2 <- GPS TX
#define PIN_GPS_TX        17   // ESP32 TX2 -> GPS RX
#define GPS_BAUD          9600

#define PIN_I2C_SDA       21
#define PIN_I2C_SCL       22

#define PIN_SD_CS         5
#define PIN_SD_MOSI       23
#define PIN_SD_MISO       19
#define PIN_SD_SCK        18

#define PIN_LDR_ADC       34
#define PIN_BUZZER        4
#define PIN_STATUS_LED    2

#define OLED_WIDTH        128
#define OLED_HEIGHT       64
#define OLED_I2C_ADDR     0x3C
#define BME280_I2C_ADDR   0x76

// ---------------------------------------------------------------------------
// Task timing
// ---------------------------------------------------------------------------
#define ACQUIRE_TASK_PERIOD_MS      20     // ~50 Hz IMU sampling target
#define FUSION_TASK_PERIOD_MS       20
#define TRUST_TASK_PERIOD_MS        50
#define TELEMETRY_TASK_PERIOD_MS    1000
#define ENV_SAMPLE_PERIOD_MS        2000   // BME280 / LDR (slow-varying)

// ---------------------------------------------------------------------------
// AHRS filter gains (Report Section 7.5)
// ---------------------------------------------------------------------------
#define MADGWICK_BETA               0.1f

// ---------------------------------------------------------------------------
// Kalman filter — process/measurement noise baselines (Report Section 7.7)
// Measurement noise R is inflated at runtime based on Trust Score.
// ---------------------------------------------------------------------------
#define KALMAN_Q_POSITION           0.05f   // process noise (DR uncertainty growth)
#define KALMAN_R_GNSS_BASE          2.0f    // measurement noise at full trust

// ---------------------------------------------------------------------------
// Navigation Trust Engine thresholds (Report Section 8.6, Appendix B)
// ---------------------------------------------------------------------------
#define TRUST_SCORE_MAX             100
#define TRUST_SCORE_MIN             0

#define TRUST_HIGH_ENTER            80   // recover to HIGH TRUST (hysteresis)
#define TRUST_HIGH_EXIT             70   // drop out of HIGH TRUST
#define TRUST_MEDIUM_ENTER          55   // recover to MEDIUM TRUST (hysteresis)
#define TRUST_MEDIUM_EXIT           40   // drop out of MEDIUM TRUST into LOW

#define PENALTY_HIGH_SEVERITY       25
#define PENALTY_MEDIUM_SEVERITY     12
#define PENALTY_LOW_SEVERITY        5
#define RECOVERY_RATE_PER_SEC       2.0f  // gradual score recovery when clean

// Rule thresholds
#define NAVR01_RESIDUAL_M            50.0f   // GNSS/DR residual distance, metres
#define NAVR02_JUMP_MAX_SPEED_MPS    25.0f   // implausible-jump speed threshold
#define NAVR03_MAX_PLAUSIBLE_SPEED   20.0f   // m/s, platform-dependent
#define NAVR04_HDOP_POOR             5.0f
#define NAVR05_SNR_STDDEV_MIN        2.0f    // dB-Hz; below this = suspiciously uniform
#define NAVR06_MAX_PARSE_FAIL_RATE   0.10f   // fraction of sentences failing checksum
#define NAVR07_HEADING_DISAGREE_DEG  30.0f
#define NAVR07_DWELL_MS              3000
#define NAVR08_MIN_SATELLITES        4
#define NAVR09_TIME_SKEW_S           5.0f
#define NAVR10_MQTT_TIMEOUT_MS       15000

// ---------------------------------------------------------------------------
// Local magnetic declination (degrees), site-specific — update for deployment
// location before use; 0.0 is a safe conservative default.
// ---------------------------------------------------------------------------
#define MAG_DECLINATION_DEG          0.0f
