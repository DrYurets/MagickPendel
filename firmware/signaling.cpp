#include "signaling.h"

#include <Arduino.h>

#include "pins.h"

namespace {
constexpr uint16_t CALL_ON_MS = 200;
constexpr uint16_t CALL_OFF_MS = 800;
constexpr uint16_t WARN_ON_MS = 120;
constexpr uint16_t WARN_OFF_MS = 120;
constexpr int WARN_CYCLES = 3;

bool running = false;
bool phaseOn = false;
uint32_t phaseStart = 0;
uint16_t onMs = 0, offMs = 0;
int cyclesLeft = -1;  // -1 = бесконечно

void outputs(bool on) {
  digitalWrite(PIN_BUZZER, on ? HIGH : LOW);
  digitalWrite(PIN_LED, on ? HIGH : LOW);
}

void start(uint16_t on, uint16_t off, int cycles) {
  onMs = on;
  offMs = off;
  cyclesLeft = cycles;
  running = true;
  phaseOn = true;
  phaseStart = millis();
  outputs(true);
}
}  // namespace

void sigBegin() {
  pinMode(PIN_BUZZER, OUTPUT);
  pinMode(PIN_LED, OUTPUT);
  outputs(false);
}

void sigStartCall() { start(CALL_ON_MS, CALL_OFF_MS, -1); }

void sigWarn() {
  if (!running) start(WARN_ON_MS, WARN_OFF_MS, WARN_CYCLES);
}

void sigStop() {
  running = false;
  phaseOn = false;
  outputs(false);
}

bool sigActive() { return running; }

void sigTick() {
  if (!running) return;
  uint32_t elapsed = millis() - phaseStart;
  if (phaseOn && elapsed >= onMs) {
    phaseOn = false;
    phaseStart = millis();
    outputs(false);
  } else if (!phaseOn && elapsed >= offMs) {
    if (cyclesLeft > 0 && --cyclesLeft == 0) {
      sigStop();
      return;
    }
    phaseOn = true;
    phaseStart = millis();
    outputs(true);
  }
}
