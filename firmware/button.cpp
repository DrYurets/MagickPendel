#include "button.h"

#include <Arduino.h>

#include "pins.h"

namespace {
constexpr uint32_t DEBOUNCE_MS = 30;
bool lastRaw = HIGH;        // INPUT_PULLUP: LOW = нажата
bool stable = HIGH;
uint32_t lastChange = 0;
}  // namespace

void btnBegin() { pinMode(PIN_BUTTON, INPUT_PULLUP); }

bool btnTick() {
  bool raw = digitalRead(PIN_BUTTON);
  if (raw != lastRaw) {
    lastRaw = raw;
    lastChange = millis();
  }
  if (raw != stable && millis() - lastChange >= DEBOUNCE_MS) {
    stable = raw;
    if (stable == LOW) return true;  // фронт нажатия
  }
  return false;
}
