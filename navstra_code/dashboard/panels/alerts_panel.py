"""
NAVSTRA dashboard — panels/alerts_panel.py
Threat/alert timeline panel (Report Section 9.5, Chapter 8 rule set).
"""
import pandas as pd
import streamlit as st

SEVERITY_ICON = {"HIGH": "\U0001F534", "MEDIUM": "\U0001F7E1", "LOW": "\u26AA"}


def render(alert_rows: list[dict]):
    st.subheader("Active / Recent Alerts")
    if not alert_rows:
        st.success("No rule firings recorded.")
        return

    df = pd.DataFrame(alert_rows).sort_values("ts", ascending=False)
    for _, row in df.head(20).iterrows():
        icon = SEVERITY_ICON.get(row["severity"], "\u26AA")
        st.markdown(
            f"{icon} **{row['rule_id']}** ({row['severity']}) — {row['description']}  "
            f"\n<small>{row['ts']} · trust score after: {row['trust_score_after']}</small>",
            unsafe_allow_html=True,
        )
