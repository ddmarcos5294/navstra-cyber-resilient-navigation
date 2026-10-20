// ============================================================================
// NAVSTRA — sensors/ldr.cpp
// ============================================================================
#include "ldr.h"
#include "config.h"
#include <Arduino.h>

void ldrInit() {
  pinMode(PIN_LDR_ADC, INPUT);
}

void ldrUpdate(RawSensors &out) {
  out.ldrRaw = analogRead(PIN_LDR_ADC);
}
