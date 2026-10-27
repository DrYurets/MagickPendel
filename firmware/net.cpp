#include "net.h"

#include <WiFi.h>

#include "settings.h"

namespace {
constexpr uint32_t RECONNECT_INTERVAL_MS = 10000;
constexpr uint32_t RESYNC_INTERVAL_MS = 3600000;  // раз в час
// Всё, что меньше — явно не синхронизированное время.
constexpr time_t VALID_EPOCH = 1700000000;  // ~ноябрь 2023

uint32_t lastReconnectTry = 0;
uint32_t lastResync = 0;
bool begun = false;
}  // namespace

void netBegin() {
  if (cfg.ssid[0] == '\0') {
    Serial.println(F("[net] Wi-Fi не настроен, работа офлайн"));
    return;
  }
  WiFi.mode(WIFI_STA);
  WiFi.begin(cfg.ssid, cfg.pass);
  begun = true;
  lastReconnectTry = millis();
  Serial.printf("[net] подключение к %s...\n", cfg.ssid);
  configTzTime(cfg.tz, "pool.ntp.org", "time.nist.gov");
}

void netTick() {
  if (!begun) return;
  uint32_t now = millis();
  if (WiFi.status() != WL_CONNECTED &&
      now - lastReconnectTry >= RECONNECT_INTERVAL_MS) {
    lastReconnectTry = now;
    Serial.println(F("[net] переподключение Wi-Fi"));
    WiFi.disconnect();
    WiFi.begin(cfg.ssid, cfg.pass);
  }
  if (netTimeValid() && now - lastResync >= RESYNC_INTERVAL_MS) {
    lastResync = now;
    configTzTime(cfg.tz, "pool.ntp.org", "time.nist.gov");
  }
}

bool netConnected() { return begun && WiFi.status() == WL_CONNECTED; }

bool netTimeValid() { return time(nullptr) >= VALID_EPOCH; }

int netNowMinutes() {
  struct tm t;
  if (!netLocalTime(t)) return -1;
  return t.tm_hour * 60 + t.tm_min;
}

bool netLocalTime(struct tm& out) {
  if (!netTimeValid()) return false;
  time_t now = time(nullptr);
  localtime_r(&now, &out);
  return true;
}
