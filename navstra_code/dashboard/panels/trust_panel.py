"""
NAVSTRA dashboard — panels/trust_panel.py
Trust Score time-series and current mode panel (Report Section 9.5, 8.6).
"""
import pandas as pd
import plotly.express as px
import streamlit as st

MODE_COLORS = {
    "HIGH_TRUST": "#2E7D32",
    "MEDIUM_TRUST": "#F9A825",
    "LOW_TRUST": "#C62828",
}


def render(telemetry_rows: list[dict]):
    st.subheader("Navigation Trust Score")
    if not telemetry_rows:
        st.info("Waiting for telemetry...")
        return

    df = pd.DataFrame(telemetry_rows)
    latest_mode = df.iloc[-1]["navigation_mode"]
    latest_score = df.iloc[-1]["trust_score"]

    badge_color = MODE_COLORS.get(latest_mode, "#616161")
    st.markdown(
        f"<div style='padding:10px;border-radius:6px;background:{badge_color};"
        f"color:white;font-weight:bold;text-align:center;'>"
        f"{latest_mode} — Trust Score {latest_score}</div>",
        unsafe_allow_html=True,
    )

    fig = px.line(df, x="ts", y="trust_score", height=260)
    fig.add_hrect(y0=70, y1=100, fillcolor="#2E7D32", opacity=0.08, line_width=0)
    fig.add_hrect(y0=40, y1=70, fillcolor="#F9A825", opacity=0.08, line_width=0)
    fig.add_hrect(y0=0, y1=40, fillcolor="#C62828", opacity=0.08, line_width=0)
    fig.update_layout(margin=dict(l=0, r=0, t=10, b=0), yaxis_range=[0, 100])
    st.plotly_chart(fig, use_container_width=True)
