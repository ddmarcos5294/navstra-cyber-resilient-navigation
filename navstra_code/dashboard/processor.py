"""
NAVSTRA dashboard — processor.py
MQTT subscriber / data-processor service (Report Section 9.3).

Subscribes to navstra/telemetry, navstra/alerts and navstra/health,
validates and persists each JSON payload, and logs basic diagnostics.
Run this as a standalone background process (e.g. `python processor.py`)
alongside the Streamlit app, which reads from the same SQLite database.
"""
import json
import logging
import time

import paho.mqtt.client as mqtt

import db

logging.basicConfig(level=logging.INFO, format="%(asctime)s %(levelname)s %(message)s")
log = logging.getLogger("navstra.processor")

MQTT_BROKER_HOST = "localhost"
MQTT_BROKER_PORT = 1883
MQTT_USERNAME = None   # set if the broker requires auth
MQTT_PASSWORD = None

TOPIC_TELEMETRY = "navstra/telemetry"
TOPIC_ALERTS = "navstra/alerts"
TOPIC_HEALTH = "navstra/health"

REQUIRED_TELEMETRY_FIELDS = {
    "timestamp", "latitude", "longitude", "heading", "speed",
    "gps_status", "trust_score", "navigation_mode", "alert",
}
REQUIRED_ALERT_FIELDS = {"timestamp", "rule_id", "severity", "description", "trust_score_after"}


def _validate(payload: dict, required: set) -> bool:
    missing = required - payload.keys()
    if missing:
        log.warning("Dropping malformed payload, missing fields: %s", missing)
        return False
    return True


def on_connect(client, userdata, flags, rc):
    if rc == 0:
        log.info("Connected to MQTT broker %s:%s", MQTT_BROKER_HOST, MQTT_BROKER_PORT)
        client.subscribe([(TOPIC_TELEMETRY, 0), (TOPIC_ALERTS, 0), (TOPIC_HEALTH, 0)])
    else:
        log.error("MQTT connect failed with code %s", rc)


def on_message(client, userdata, msg):
    try:
        payload = json.loads(msg.payload.decode("utf-8"))
    except (UnicodeDecodeError, json.JSONDecodeError) as exc:
        log.warning("Dropping non-JSON payload on %s: %s", msg.topic, exc)
        return

    if msg.topic == TOPIC_TELEMETRY:
        if _validate(payload, REQUIRED_TELEMETRY_FIELDS):
            db.insert_telemetry(payload)
    elif msg.topic == TOPIC_ALERTS:
        if _validate(payload, REQUIRED_ALERT_FIELDS):
            db.insert_alert(payload)
            log.info("ALERT %s [%s]: %s (score=%s)",
                      payload["rule_id"], payload["severity"],
                      payload["description"], payload["trust_score_after"])
    elif msg.topic == TOPIC_HEALTH:
        payload.setdefault("ts", payload.get("timestamp", time.strftime("%Y-%m-%dT%H:%M:%SZ")))
        db.insert_health(payload)


def main():
    db.init_db()
    client = mqtt.Client(client_id="navstra-dashboard-processor")
    if MQTT_USERNAME:
        client.username_pw_set(MQTT_USERNAME, MQTT_PASSWORD)
    client.on_connect = on_connect
    client.on_message = on_message

    while True:
        try:
            client.connect(MQTT_BROKER_HOST, MQTT_BROKER_PORT, keepalive=30)
            client.loop_forever()
        except (ConnectionRefusedError, OSError) as exc:
            log.error("MQTT connection error: %s — retrying in 5s", exc)
            time.sleep(5)


if __name__ == "__main__":
    main()
