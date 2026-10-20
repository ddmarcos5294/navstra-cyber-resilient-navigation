"""
NAVSTRA dashboard — db.py
SQLite schema and query helpers (Report Section 9.3, Appendix C).
"""
import os
import sqlite3
import threading
from contextlib import contextmanager

DB_PATH = os.path.join(os.path.dirname(__file__), "navstra_telemetry.db")

_lock = threading.Lock()

SCHEMA = """
CREATE TABLE IF NOT EXISTS telemetry (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    ts TEXT NOT NULL,
    latitude REAL,
    longitude REAL,
    heading REAL,
    speed REAL,
    gps_status TEXT,
    trust_score INTEGER,
    navigation_mode TEXT,
    alert TEXT
);

CREATE TABLE IF NOT EXISTS alerts (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    ts TEXT NOT NULL,
    rule_id TEXT,
    severity TEXT,
    description TEXT,
    trust_score_after INTEGER
);

CREATE TABLE IF NOT EXISTS health (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    ts TEXT NOT NULL,
    uptime_ms INTEGER,
    free_heap INTEGER,
    sd_ok INTEGER,
    wifi_rssi INTEGER
);
"""


@contextmanager
def get_conn():
    conn = sqlite3.connect(DB_PATH, check_same_thread=False)
    try:
        yield conn
    finally:
        conn.close()


def init_db():
    with _lock, get_conn() as conn:
        conn.executescript(SCHEMA)
        conn.commit()


def insert_telemetry(payload: dict):
    with _lock, get_conn() as conn:
        conn.execute(
            """INSERT INTO telemetry
               (ts, latitude, longitude, heading, speed, gps_status,
                trust_score, navigation_mode, alert)
               VALUES (:timestamp, :latitude, :longitude, :heading, :speed,
                       :gps_status, :trust_score, :navigation_mode, :alert)""",
            payload,
        )
        conn.commit()


def insert_alert(payload: dict):
    with _lock, get_conn() as conn:
        conn.execute(
            """INSERT INTO alerts (ts, rule_id, severity, description, trust_score_after)
               VALUES (:timestamp, :rule_id, :severity, :description, :trust_score_after)""",
            payload,
        )
        conn.commit()


def insert_health(payload: dict):
    with _lock, get_conn() as conn:
        conn.execute(
            """INSERT INTO health (ts, uptime_ms, free_heap, sd_ok, wifi_rssi)
               VALUES (:ts, :uptime_ms, :free_heap, :sd_ok, :wifi_rssi)""",
            payload,
        )
        conn.commit()


def fetch_recent_telemetry(limit: int = 500):
    with get_conn() as conn:
        conn.row_factory = sqlite3.Row
        cur = conn.execute(
            "SELECT * FROM telemetry ORDER BY id DESC LIMIT ?", (limit,)
        )
        rows = [dict(r) for r in cur.fetchall()]
        rows.reverse()
        return rows


def fetch_recent_alerts(limit: int = 100):
    with get_conn() as conn:
        conn.row_factory = sqlite3.Row
        cur = conn.execute(
            "SELECT * FROM alerts ORDER BY id DESC LIMIT ?", (limit,)
        )
        rows = [dict(r) for r in cur.fetchall()]
        rows.reverse()
        return rows


def fetch_latest_health():
    with get_conn() as conn:
        conn.row_factory = sqlite3.Row
        cur = conn.execute("SELECT * FROM health ORDER BY id DESC LIMIT 1")
        row = cur.fetchone()
        return dict(row) if row else None
