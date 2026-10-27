#pragma once

#include <Arduino.h>

// Настройки устройства, хранятся в NVS (Preferences, namespace "pendel").
// Время суток — в минутах от полуночи.
struct Config {
  uint32_t callDurationSec;  // длительность вызова, сек
  uint16_t bedtimeMin;       // отбой
  uint16_t sleepStartMin;    // начало окна сна
  uint16_t sleepEndMin;      // конец окна сна
  char tz[64];               // POSIX TZ-строка
  char ssid[33];             // Wi-Fi
  char pass[65];
};

extern Config cfg;

void settingsLoad();
void settingsSave();
void settingsPrint();  // диагностика в Serial
