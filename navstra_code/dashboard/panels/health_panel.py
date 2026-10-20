"""
NAVSTRA dashboard — panels/health_panel.py
System health panel (Report Section 9.5).
"""
import streamlit as st


def render(health: dict | None):
    st.subheader("System Health")
    if not health:
        st.info("No health telemetry received yet.")
        return

    col1, col2, col3, col4 = st.columns(4)
    uptime_s = (health.get("uptime_ms") or 0) / 1000
    col1.metric("Uptime", f"{uptime_s:,.0f} s")
    col2.metric("Free Heap", f"{(health.get('free_heap') or 0):,} B")
    col3.metric("SD Card", "OK" if health.get("sd_ok") else "FAULT")
    col4.metric("Wi-Fi RSSI", f"{health.get('wifi_rssi', 'N/A')} dBm")
