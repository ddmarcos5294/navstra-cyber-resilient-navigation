"""
NAVSTRA — algorithms/trust_score_reference.py

Offline, hardware-independent Python reference implementation of the
Navigation Trust Score logic (Report Section 8.6), mirroring
firmware/src/trust/score.cpp. Used for:
  - Unit-testing the score/hysteresis behaviour without ESP32 hardware
  - Offline analysis of logged CSV data (Report Chapter 11)
  - Experiment 10 (Trust-Score recovery dynamics, Report Section 10.11)

This file intentionally duplicates the firmware logic in Python rather
than being imported by it — the firmware is C++/embedded and this is a
desktop analysis tool (Report Section 6.3, algorithms/ folder purpose).
"""
from dataclasses import dataclass
from enum import Enum


class Severity(Enum):
    LOW = "LOW"
    MEDIUM = "MEDIUM"
    HIGH = "HIGH"


PENALTY = {Severity.HIGH: 25, Severity.MEDIUM: 12, Severity.LOW: 5}
RECOVERY_RATE_PER_SEC = 2.0

TRUST_HIGH_ENTER = 80
TRUST_HIGH_EXIT = 70
TRUST_MEDIUM_ENTER = 55
TRUST_MEDIUM_EXIT = 40


class NavMode(Enum):
    HIGH_TRUST = "HIGH_TRUST"
    MEDIUM_TRUST = "MEDIUM_TRUST"
    LOW_TRUST = "LOW_TRUST"


@dataclass
class TrustEngineState:
    score: int = 100
    mode: NavMode = NavMode.HIGH_TRUST


def update_score(state: TrustEngineState, fired_severities: list[Severity], dt_seconds: float) -> int:
    if not fired_severities:
        state.score += int(RECOVERY_RATE_PER_SEC * dt_seconds)
    else:
        for sev in fired_severities:
            state.score -= PENALTY[sev]
    state.score = max(0, min(100, state.score))
    return state.score


def update_mode(state: TrustEngineState) -> NavMode:
    """Hysteresis-gated mode transition (Report Section 8.6)."""
    if state.mode == NavMode.HIGH_TRUST:
        if state.score < TRUST_HIGH_EXIT:
            state.mode = NavMode.LOW_TRUST if state.score < TRUST_MEDIUM_EXIT else NavMode.MEDIUM_TRUST
    elif state.mode == NavMode.MEDIUM_TRUST:
        if state.score >= TRUST_HIGH_ENTER:
            state.mode = NavMode.HIGH_TRUST
        elif state.score < TRUST_MEDIUM_EXIT:
            state.mode = NavMode.LOW_TRUST
    elif state.mode == NavMode.LOW_TRUST:
        if state.score >= TRUST_MEDIUM_ENTER:
            state.mode = NavMode.MEDIUM_TRUST
    return state.mode


if __name__ == "__main__":
    # Small demonstration mirroring Experiment 10 (Report Section 10.11):
    # a single HIGH-severity rule fires once, then the input is clean.
    state = TrustEngineState()
    print("t=0s  score=%d mode=%s (initial)" % (state.score, state.mode.value))

    update_score(state, [Severity.HIGH], dt_seconds=1.0)
    update_mode(state)
    print("t=1s  score=%d mode=%s (rule fired)" % (state.score, state.mode.value))

    for t in range(2, 20):
        update_score(state, [], dt_seconds=1.0)
        update_mode(state)
        print(f"t={t}s  score={state.score} mode={state.mode.value}")
