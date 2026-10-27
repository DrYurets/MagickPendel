#include "settings.h"

#include <Preferences.h>

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
  cfg.ssid[0] = '\0';  // SSID и пароль задаются через веб-админку
  cfg.pass[0] = '\0';
  strncpy(cfg.child1, "Ребёнок 1", sizeof(cfg.child1) - 1);
  cfg.child1[sizeof(cfg.child1) - 1] = '\0';
  strncpy(cfg.child2, "Ребёнок 2", sizeof(cfg.child2) - 1);
  cfg.child2[sizeof(cfg.child2) - 1] = '\0';
  cfg.tgToken[0] = '\0';
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
  cfg.child1[sizeof(cfg.child1) - 1] = '\0';
  cfg.child2[sizeof(cfg.child2) - 1] = '\0';
  cfg.tgToken[sizeof(cfg.tgToken) - 1] = '\0';
  if (cfg.child1[0] == '\0')
    strncpy(cfg.child1, "Ребёнок 1", sizeof(cfg.child1) - 1);
  if (cfg.child2[0] == '\0')
    strncpy(cfg.child2, "Ребёнок 2", sizeof(cfg.child2) - 1);
  if (cfg.tz[0] == '\0')
    strncpy(cfg.tz, "EET-2EEST,M3.5.0/3,M10.5.0/4", sizeof(cfg.tz) - 1);
}
}  // namespace

void settingsLoad() {
  defaults();
  if (!prefs.begin(NS, /*readOnly=*/true)) {
    // Namespace ещё не создан (первый запуск) — это не ошибка NVS.
    Serial.println(F("[settings] настроек нет, применены дефолты"));
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
  if (prefs.isKey("child1"))
    prefs.getString("child1", cfg.child1, sizeof(cfg.child1));
  if (prefs.isKey("child2"))
    prefs.getString("child2", cfg.child2, sizeof(cfg.child2));
  if (prefs.isKey("tgTok"))
    prefs.getString("tgTok", cfg.tgToken, sizeof(cfg.tgToken));
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
  prefs.putString("child1", cfg.child1);
  prefs.putString("child2", cfg.child2);
  prefs.putString("tgTok", cfg.tgToken);
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
  Serial.printf("[settings] дети: %s / %s, tg: %s\n", cfg.child1, cfg.child2,
                cfg.tgToken[0] ? "задан" : "пусто");
}
