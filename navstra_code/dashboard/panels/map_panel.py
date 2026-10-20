"""
NAVSTRA dashboard — panels/map_panel.py
Position/track map panel (Report Section 9.5).
"""
import pandas as pd
import plotly.express as px
import streamlit as st


def render(telemetry_rows: list[dict]):
    st.subheader("Position")
    if not telemetry_rows:
        st.info("Waiting for telemetry...")
        return

    df = pd.DataFrame(telemetry_rows)
    df = df.dropna(subset=["latitude", "longitude"])
    if df.empty:
        st.info("No valid position fixes yet.")
        return

    latest = df.iloc[-1]
    col1, col2, col3 = st.columns(3)
    col1.metric("Latitude", f"{latest['latitude']:.6f}")
    col2.metric("Longitude", f"{latest['longitude']:.6f}")
    col3.metric("Heading / Speed", f"{latest['heading']:.0f}\u00b0 / {latest['speed']:.1f} m/s")

    fig = px.scatter_mapbox(
        df, lat="latitude", lon="longitude",
        color="navigation_mode",
        color_discrete_map={
            "HIGH_TRUST": "#2E7D32",
            "MEDIUM_TRUST": "#F9A825",
            "LOW_TRUST": "#C62828",
        },
        zoom=15, height=420,
    )
    fig.update_layout(mapbox_style="open-street-map", margin=dict(l=0, r=0, t=0, b=0))
    st.plotly_chart(fig, use_container_width=True)
