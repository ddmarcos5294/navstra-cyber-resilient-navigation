// ============================================================================
// NAVSTRA — telemetry/mqtt_client.cpp
// ============================================================================
#include "mqtt_client.h"
#include "config.h"
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>

static WiFiClient wifiClient;
static PubSubClient mqttClient(wifiClient);

bool telemetryWifiConnect() {
  WiFi.mode(WIFI_STA);
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED && millis() - start < 15000) {
    delay(250);
  }
  return WiFi.status() == WL_CONNECTED;
}

bool telemetryMqttConnect() {
  mqttClient.setServer(MQTT_BROKER_HOST, MQTT_BROKER_PORT);
  if (mqttClient.connected()) return true;
  const char *user = (strlen(MQTT_USERNAME) > 0) ? MQTT_USERNAME : nullptr;
  const char *pass = (strlen(MQTT_PASSWORD) > 0) ? MQTT_PASSWORD : nullptr;
  return mqttClient.connect(MQTT_CLIENT_ID, user, pass);
}

void telemetryMqttLoop() {
  if (!mqttClient.connected()) {
    telemetryMqttConnect();
  }
  mqttClient.loop();
}

bool telemetryMqttIsConnected() {
  return mqttClient.connected();
}

// Small helper to format ISO-8601-ish timestamps without an RTC/NTP
// dependency baked into this module; main.cpp should set the system time
// (e.g. via NTP once Wi-Fi is up) for a fully correct timestamp field.
static void isoTimestamp(char *buf, size_t bufLen) {
  time_t now;
  time(&now);
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  strftime(buf, bufLen, "%Y-%m-%dT%H:%M:%SZ", &timeinfo);
}

void publishTelemetry(const NavState &nav, bool gpsFixValid, const char *alertSummary) {
  StaticJsonDocument<256> doc;
  char ts[32];
  isoTimestamp(ts, sizeof(ts));

  doc["timestamp"]        = ts;
  doc["latitude"]         = nav.lat;
  doc["longitude"]        = nav.lon;
  doc["heading"]          = nav.headingDeg;
  doc["speed"]            = nav.speedMps;
  doc["gps_status"]       = gpsFixValid ? "FIX" : "NO_FIX";
  doc["trust_score"]      = nav.trustScore;
  doc["navigation_mode"]  = navModeToString(nav.mode);
  doc["alert"]            = alertSummary;

  char payload[256];
  size_t len = serializeJson(doc, payload, sizeof(payload));
  mqttClient.publish(TOPIC_TELEMETRY, (const uint8_t *)payload, len, false);
}

void publishAlert(const RuleResult &rule, int scoreAfter) {
  StaticJsonDocument<192> doc;
  char ts[32];
  isoTimestamp(ts, sizeof(ts));

  const char *sevStr = "LOW";
  if (rule.severity == Severity::HIGH) sevStr = "HIGH";
  else if (rule.severity == Severity::MEDIUM) sevStr = "MEDIUM";

  doc["timestamp"]          = ts;
  doc["rule_id"]            = rule.ruleId;
  doc["severity"]           = sevStr;
  doc["description"]        = rule.description;
  doc["trust_score_after"]  = scoreAfter;

  char payload[192];
  size_t len = serializeJson(doc, payload, sizeof(payload));
  mqttClient.publish(TOPIC_ALERTS, (const uint8_t *)payload, len, false);
}

void publishHealth(uint32_t uptimeMs, uint32_t freeHeap, bool sdOk) {
  StaticJsonDocument<192> doc;
  doc["uptime_ms"]  = uptimeMs;
  doc["free_heap"]  = freeHeap;
  doc["sd_ok"]      = sdOk;
  doc["wifi_rssi"]  = WiFi.RSSI();

  char payload[192];
  size_t len = serializeJson(doc, payload, sizeof(payload));
  mqttClient.publish(TOPIC_HEALTH, (const uint8_t *)payload, len, false);
}
