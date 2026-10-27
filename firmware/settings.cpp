#include "settings.h"

#include <Preferences.h>

#if __has_include("secrets.h")
#include "secrets.h"
#endif

#ifndef WIFI_SSID
#define WIFI_SSID ""
#endif
#ifndef WIFI_PASS
#define WIFI_PASS ""
#endif

Config cfg;

namespace {
Preferences prefs;
constexpr const char* NS = "pendel";

void defaults() {
  cfg.callDurationSec = 60;
  cfg.bedtimeMin = 22 * 60;        // 22:00
  cfg.sleepStartMin = 22 * 60 + 30; // 22:30
  cfg.sleepEndMin = 7 * 60;        // 07:00
  strncpy(cfg.tz, "EET-2EEST,M3.5.0/3,M10.5.0/4", sizeof(cfg.tz) - 1);
  cfg.tz[sizeof(cfg.tz) - 1] = '\0';
  strncpy(cfg.ssid, WIFI_SSID, sizeof(cfg.ssid) - 1);
  cfg.ssid[sizeof(cfg.ssid) - 1] = '\0';
  strncpy(cfg.pass, WIFI_PASS, sizeof(cfg.pass) - 1);
  cfg.pass[sizeof(cfg.pass) - 1] = '\0';
}

// Дефолт, если значение вне допустимого диапазона (пустое/повреждённое).
void sanitize() {
  if (cfg.callDurationSec < 5 || cfg.callDurationSec > 600) cfg.callDurationSec = 60;
  if (cfg.bedtimeMin >= 1440) cfg.bedtimeMin = 22 * 60;
  if (cfg.sleepStartMin >= 1440) cfg.sleepStartMin = 22 * 60 + 30;
  if (cfg.sleepEndMin >= 1440) cfg.sleepEndMin = 7 * 60;
  cfg.tz[sizeof(cfg.tz) - 1] = '\0';
  cfg.ssid[sizeof(cfg.ssid) - 1] = '\0';
  cfg.pass[sizeof(cfg.pass) - 1] = '\0';
  if (cfg.tz[0] == '\0')
    strncpy(cfg.tz, "EET-2EEST,M3.5.0/3,M10.5.0/4", sizeof(cfg.tz) - 1);
}
}  // namespace

void settingsLoad() {
  defaults();
  if (!prefs.begin(NS, /*readOnly=*/true)) {
    Serial.println(F("[settings] NVS недоступен, применены дефолты"));
    return;
  }
  cfg.callDurationSec = prefs.getUInt("callDur", cfg.callDurationSec);
  cfg.bedtimeMin = prefs.getUShort("bedtime", cfg.bedtimeMin);
  cfg.sleepStartMin = prefs.getUShort("sleepS", cfg.sleepStartMin);
  cfg.sleepEndMin = prefs.getUShort("sleepE", cfg.sleepEndMin);
  prefs.getString("tz", cfg.tz, sizeof(cfg.tz));
  if (cfg.tz[0] == '\0')
    strncpy(cfg.tz, "EET-2EEST,M3.5.0/3,M10.5.0/4", sizeof(cfg.tz) - 1);
  if (prefs.isKey("ssid")) prefs.getString("ssid", cfg.ssid, sizeof(cfg.ssid));
  if (prefs.isKey("pass")) prefs.getString("pass", cfg.pass, sizeof(cfg.pass));
  prefs.end();
  sanitize();
}

void settingsSave() {
  if (!prefs.begin(NS, /*readOnly=*/false)) {
    Serial.println(F("[settings] не удалось открыть NVS на запись"));
    return;
  }
  prefs.putUInt("callDur", cfg.callDurationSec);
  prefs.putUShort("bedtime", cfg.bedtimeMin);
  prefs.putUShort("sleepS", cfg.sleepStartMin);
  prefs.putUShort("sleepE", cfg.sleepEndMin);
  prefs.putString("tz", cfg.tz);
  prefs.putString("ssid", cfg.ssid);
  prefs.putString("pass", cfg.pass);
  prefs.end();
}

void settingsPrint() {
  Serial.printf(
      "[settings] callDur=%lus bedtime=%02u:%02u sleep=%02u:%02u-%02u:%02u "
      "tz=%s ssid=%s\n",
      (unsigned long)cfg.callDurationSec, cfg.bedtimeMin / 60,
      cfg.bedtimeMin % 60, cfg.sleepStartMin / 60, cfg.sleepStartMin % 60,
      cfg.sleepEndMin / 60, cfg.sleepEndMin % 60, cfg.tz,
      cfg.ssid[0] ? cfg.ssid : "(пусто)");
}
