// ============================================================================
// NAVSTRA — telemetry/mqtt_client.h
// MQTT publish of telemetry/alerts/health (Report Section 6.8, Appendix D).
// ============================================================================
#pragma once
#include "nav_types.h"
#include "trust/rules.h"

bool telemetryWifiConnect();
bool telemetryMqttConnect();
void telemetryMqttLoop();  // call frequently from the telemetry task
bool telemetryMqttIsConnected();

// Publishes the navstra/telemetry JSON payload (Appendix D format).
void publishTelemetry(const NavState &nav, bool gpsFixValid, const char *alertSummary);

// Publishes a navstra/alerts JSON payload for a single fired rule.
void publishAlert(const RuleResult &rule, int scoreAfter);

// Publishes a navstra/health JSON payload.
void publishHealth(uint32_t uptimeMs, uint32_t freeHeap, bool sdOk);
