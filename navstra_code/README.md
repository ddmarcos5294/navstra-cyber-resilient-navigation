# NAVSTRA — Source Code

Companion source code to the NAVSTRA Final Year Project Report
("NAVSTRA: A Cyber-Resilient Hybrid Navigation System for GPS-Denied and
GPS-Spoofed Maritime Environments"). See the full report for design
rationale, formulas, and section references cited throughout these files.

## Structure

- **firmware/** — ESP32 C++/PlatformIO source. Implements sensor
  acquisition (GPS/IMU/magnetometer/BME280/LDR), Madgwick AHRS,
  dead-reckoning, a loosely-coupled Kalman GNSS/DR fusion filter, the
  NAV-R01–NAV-R10 rule-based Trust Engine, and MQTT/SD/OLED telemetry —
  all wired together as FreeRTOS tasks in `src/main.cpp`.
- **dashboard/** — Python/Streamlit SOC-style dashboard. `processor.py`
  subscribes to MQTT and persists telemetry to SQLite (`db.py`); `app.py`
  renders the live dashboard from `panels/`.
- **algorithms/** — Standalone, hardware-independent Python reference
  implementation of the Trust Score/hysteresis logic, for offline testing
  and analysis without ESP32 hardware.
- **tests/** — Unit tests for the Trust Engine logic (`pytest`), verified
  to pass against the reference implementation.

## Quick start

**Firmware:**
```
cd firmware
# create include/secrets.h or edit include/config.h with your Wi-Fi/MQTT details
pio run -t upload
```

**Dashboard:**
```
cd dashboard
pip install -r requirements.txt
python processor.py &      # background MQTT subscriber
streamlit run app.py
```

**Tests:**
```
cd tests
python -m pytest test_trust_engine.py -v
```

## Status

This code implements the architecture described in the report (Chapters
4–9). It has been checked for syntax correctness and the Trust Engine
logic has been unit-tested (6/6 passing), but it has **not** been run on
assembled NAVSTRA hardware — bring-up, calibration, and the Chapter 10
experiments remain to be carried out per the report's Completeness
Checklist. Review pin assignments, library versions, and calibration
constants in `firmware/include/config.h` before flashing to real hardware.
