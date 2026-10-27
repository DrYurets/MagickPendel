// Pendel — устройство вызова ребёнка из детской.
// Архитектура и распиновка: openspec/changes/init-firmware/design.md

#include <Arduino.h>
#include <Wire.h>

#include "button.h"
#include "climate.h"
#include "display.h"
#include "modes.h"
#include "net.h"
#include "pins.h"
#include "settings.h"
#include "signaling.h"

namespace {
char serialBuf[96];
uint8_t serialLen = 0;

void i2cScan() {
  Serial.println(F("[i2c] сканирование шины:"));
  bool found = false;
  for (uint8_t addr = 1; addr < 127; addr++) {
    Wire.beginTransmission(addr);
    if (Wire.endTransmission() == 0) {
      Serial.printf("  найдено устройство 0x%02X\n", addr);
      found = true;
    }
  }
  if (!found) Serial.println(F("  устройств не найдено"));
}

void printStatus() {
  Serial.printf("[status] mode=%d wifi=%s time=%s climate=%s\n",
                (int)modesCurrent(),
                netConnected() ? "OK" : "нет",
                netTimeValid() ? "OK" : "нет",
                climateValid() ? "OK" : "нет");
}

// call 1|2|both [текст] | status | scan
void handleCommand(char* line) {
  if (strncmp(line, "call ", 5) == 0) {
    char* arg = line + 5;
    char* msg = strchr(arg, ' ');
    if (msg) *msg++ = '\0';
    CallTarget t;
    if (strcmp(arg, "1") == 0) t = CallTarget::Child1;
    else if (strcmp(arg, "2") == 0) t = CallTarget::Child2;
    else if (strcmp(arg, "both") == 0) t = CallTarget::Both;
    else {
      Serial.println(F("цель: 1, 2 или both"));
      return;
    }
    triggerCall(t, msg ? msg : "");
    return;
  }
  if (strcmp(line, "status") == 0) return printStatus();
  if (strcmp(line, "scan") == 0) return i2cScan();
  Serial.println(F("команды: call 1|2|both [текст], status, scan"));
}

void serialTick() {
  while (Serial.available()) {
    char c = (char)Serial.read();
    if (c == '\r') continue;
    if (c == '\n' || serialLen >= sizeof(serialBuf) - 1) {
      serialBuf[serialLen] = '\0';
      serialLen = 0;
      if (serialBuf[0]) handleCommand(serialBuf);
    } else {
      serialBuf[serialLen++] = c;
    }
  }
}
}  // namespace

void setup() {
  Serial.begin(115200);
  delay(300);
  Serial.println(F("\n=== Pendel ==="));

  pinMode(PIN_LED_ONBOARD, OUTPUT);
  digitalWrite(PIN_LED_ONBOARD, HIGH);  // active low → выключен

  settingsLoad();
  settingsPrint();

  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
  i2cScan();

  sigBegin();
  climateBegin();
  dispBegin();
  netBegin();
  modesBegin();

  Serial.println(F("готово. команды: call 1|2|both [текст], status, scan"));
}

void loop() {
  netTick();
  climateTick();
  sigTick();
  modesTick();
  if (modesCurrent() == Mode::Idle) dispTick();
  serialTick();
}
