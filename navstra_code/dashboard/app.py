"""
NAVSTRA dashboard — app.py
Streamlit entry point for the SOC-style navigation dashboard
(Report Chapter 9, Appendix C).

Run with:  streamlit run app.py
Requires processor.py to be running (separately) to populate the database
from the live MQTT stream; this file only reads and renders.
"""
import time

import streamlit as st
from streamlit_autorefresh import st_autorefresh

import db
from panels import alerts_panel, health_panel, map_panel, trust_panel

st.set_page_config(page_title="NAVSTRA SOC Dashboard", layout="wide", page_icon="\U0001F6F0")

REFRESH_INTERVAL_S = 2
TELEMETRY_WINDOW = 300


def main():
    db.init_db()

    st.title("NAVSTRA — Navigation Security Operations Center")
    st.caption(
        "SOC-style monitoring for a single navigation prototype. "
        "Not a general-purpose IT SOC/SIEM (Report Section 9.6)."
    )

    telemetry_rows = db.fetch_recent_telemetry(limit=TELEMETRY_WINDOW)
    alert_rows = db.fetch_recent_alerts(limit=100)
    health = db.fetch_latest_health()

    col_left, col_right = st.columns([2, 1])
    with col_left:
        map_panel.render(telemetry_rows)
        trust_panel.render(telemetry_rows)
    with col_right:
        alerts_panel.render(alert_rows)
        health_panel.render(health)

    st.divider()
    st.caption(
        f"Auto-refreshing every {REFRESH_INTERVAL_S}s. Last updated: "
        f"{time.strftime('%Y-%m-%d %H:%M:%S')}"
    )
    st_autorefresh(
        interval=REFRESH_INTERVAL_S * 1000, 
        key="navstra_dashboard_refresh"
    )


if __name__ == "__main__":
    main()
