// Pendel — устройство вызова ребёнка из детской.
// Архитектура и распиновка: openspec/changes/init-firmware/design.md

#include <Arduino.h>
#include <WiFi.h>
#include <Wire.h>

#include "admin.h"
#include "bus.h"
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

void printStatus() {
  Serial.printf("[status] mode=%d wifi=%s ap=%s stations=%d wifimode=%d "
                "time=%s climate=%s heap=%u\n",
                (int)modesCurrent(),
                netConnected() ? "OK" : "нет",
                netProvisioning() ? "ON" : "off",
                (int)WiFi.softAPgetStationNum(), (int)WiFi.getMode(),
                netTimeValid() ? "OK" : "нет",
                climateValid() ? "OK" : "нет",
                ESP.getFreeHeap());
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
  if (strcmp(line, "scan") == 0) return busScan();
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

  // Сеть и админка первыми — точка доступа не должна ждать периферию.
  sigBegin();
  netBegin();
  adminBegin();

  Wire.begin(PIN_I2C_SDA, PIN_I2C_SCL);
  busScan();
  climateBegin();
  dispBegin();
  modesBegin();

  Serial.println(F("готово. команды: call 1|2|both [текст], status, scan"));
}

void loop() {
  netTick();
  climateTick();
  sigTick();
  modesTick();
  if (modesCurrent() == Mode::Idle) dispTick();
  adminTick();
  serialTick();
}
