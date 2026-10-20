"""
NAVSTRA — tests/test_trust_engine.py

Unit tests for the offline Trust Score reference logic
(algorithms/trust_score_reference.py), exercising the scenarios of
Report Chapter 10, Experiments 4, 5 and 10 (position jump, implausible
velocity, and score-recovery dynamics) at the algorithm level, independent
of any ESP32 hardware.

Run with:  python -m pytest tests/test_trust_engine.py -v
"""
import sys
import os

sys.path.insert(0, os.path.join(os.path.dirname(__file__), "..", "algorithms"))

from trust_score_reference import (  # noqa: E402
    TrustEngineState, Severity, NavMode, update_score, update_mode,
)


def test_initial_state_is_high_trust():
    state = TrustEngineState()
    assert state.score == 100
    assert state.mode == NavMode.HIGH_TRUST


def test_single_high_severity_rule_drops_score():
    state = TrustEngineState()
    update_score(state, [Severity.HIGH], dt_seconds=1.0)
    assert state.score == 75


def test_repeated_high_severity_firings_drop_into_low_trust():
    # Mirrors Experiment 4 (position jump) sustained over several cycles.
    state = TrustEngineState()
    for _ in range(3):
        update_score(state, [Severity.HIGH], dt_seconds=1.0)
        update_mode(state)
    assert state.score <= 40
    assert state.mode in (NavMode.MEDIUM_TRUST, NavMode.LOW_TRUST)


def test_score_never_goes_below_zero():
    state = TrustEngineState()
    for _ in range(10):
        update_score(state, [Severity.HIGH, Severity.HIGH], dt_seconds=1.0)
    assert state.score == 0


def test_score_never_exceeds_hundred():
    state = TrustEngineState()
    for _ in range(100):
        update_score(state, [], dt_seconds=1.0)
    assert state.score == 100


def test_hysteresis_prevents_immediate_recovery_to_high_trust():
    # Mirrors Experiment 10: after dropping to MEDIUM/LOW, recovering to
    # HIGH_TRUST requires crossing TRUST_HIGH_ENTER (80), not just
    # TRUST_HIGH_EXIT (70) — this is the anti-chattering behaviour.
    state = TrustEngineState()
    for _ in range(3):
        update_score(state, [Severity.HIGH], dt_seconds=1.0)
        update_mode(state)
    assert state.mode != NavMode.HIGH_TRUST

    # Recover just to 70-79 range; mode must NOT jump back to HIGH_TRUST yet.
    while state.score < 75:
        update_score(state, [], dt_seconds=1.0)
        update_mode(state)
    assert state.mode != NavMode.HIGH_TRUST

    # Continue recovering past 80; now it should.
    while state.score < 80:
        update_score(state, [], dt_seconds=1.0)
        update_mode(state)
    assert state.mode == NavMode.HIGH_TRUST
