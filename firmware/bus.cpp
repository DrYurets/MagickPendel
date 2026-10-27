#include "bus.h"

#include <Wire.h>

namespace {
bool present[128] = {};
}  // namespace

void busScan() {
  Serial.println(F("[i2c] сканирование шины:"));
  bool found = false;
  for (uint8_t addr = 1; addr < 127; addr++) {
    present[addr] = busProbe(addr);
    if (present[addr]) {
      Serial.printf("  найдено устройство 0x%02X\n", addr);
      found = true;
    }
  }
  if (!found) Serial.println(F("  устройств не найдено"));
}

bool busPresent(uint8_t addr) { return addr < 128 && present[addr]; }

bool busProbe(uint8_t addr) {
  Wire.beginTransmission(addr);
  return Wire.endTransmission() == 0;
}
