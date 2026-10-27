#include "net.h"

#include <WiFi.h>

#include "settings.h"

namespace {
constexpr uint32_t RECONNECT_INTERVAL_MS = 10000;
constexpr uint32_t RESYNC_INTERVAL_MS = 3600000;  // раз в час
// Всё, что меньше — явно не синхронизированное время.
constexpr time_t VALID_EPOCH = 1700000000;  // ~ноябрь 2023
constexpr int MAX_STA_ATTEMPTS = 6;         // потом включается AP
constexpr const char* AP_SSID = "Pendel-Setup";

uint32_t lastReconnectTry = 0;
uint32_t lastResync = 0;
int staAttempts = 0;
bool staBegun = false;
bool apOn = false;
bool wasConnected = false;

void staConnect() {
  staBegun = true;
  lastReconnectTry = millis();
  staAttempts++;
  // В режиме AP держим комбинированный режим, иначе чистый STA.
  WiFi.mode(apOn ? WIFI_AP_STA : WIFI_STA);
  WiFi.begin(cfg.ssid, cfg.pass);
  Serial.printf("[net] подключение к %s (попытка %d)\n", cfg.ssid,
                staAttempts);
}

void startAp() {
  if (apOn) return;
  // Без креденшелов — чистая точка доступа, без STA-сканирования.
  WiFi.mode(staBegun ? WIFI_AP_STA : WIFI_AP);
  if (!WiFi.softAP(AP_SSID)) {
    Serial.println(F("[net] softAP не удался, повтор через 5 с"));
    return;
  }
  apOn = true;
  Serial.printf("[net] точка доступа %s, админка: http://%s/\n", AP_SSID,
                WiFi.softAPIP().toString().c_str());
}

void stopAp() {
  if (!apOn) return;
  WiFi.softAPdisconnect(true);
  WiFi.mode(WIFI_STA);
  apOn = false;
  Serial.println(F("[net] точка доступа отключена"));
}
}  // namespace

void netBegin() {
  // Питание от сети — modem sleep не нужен, а в AP-режиме без STA он
  // делает радио нестабильным (маячки пропадают, клиенты не подключаются).
  WiFi.setSleep(false);
  if (cfg.ssid[0] == '\0') {
    Serial.println(F("[net] Wi-Fi не настроен → режим настройки"));
    startAp();
    return;
  }
  staConnect();
  configTzTime(cfg.tz, "pool.ntp.org", "time.nist.gov");
}

void netTick() {
  uint32_t now = millis();
  bool connected = WiFi.status() == WL_CONNECTED;

  if (connected && !wasConnected) {
    Serial.printf("[net] подключено, IP: %s\n",
                  WiFi.localIP().toString().c_str());
    staAttempts = 0;
    stopAp();
  }
  wasConnected = connected;

  if (cfg.ssid[0] == '\0') {
    // Креденшелов нет — держим AP, ретраим при неудаче softAP.
    if (!apOn && now - lastReconnectTry >= 5000) {
      lastReconnectTry = now;
      startAp();
    }
    return;
  }

  if (!connected && now - lastReconnectTry >= RECONNECT_INTERVAL_MS) {
    Serial.println(F("[net] переподключение Wi-Fi"));
    WiFi.disconnect();
    staConnect();
    if (staAttempts >= MAX_STA_ATTEMPTS) startAp();
  }
  if (netTimeValid() && now - lastResync >= RESYNC_INTERVAL_MS) {
    lastResync = now;
    configTzTime(cfg.tz, "pool.ntp.org", "time.nist.gov");
  }
}

bool netConnected() { return WiFi.status() == WL_CONNECTED; }

bool netProvisioning() { return apOn; }

String netAdminIp() {
  if (apOn) return WiFi.softAPIP().toString();
  if (netConnected()) return WiFi.localIP().toString();
  return "-";
}

void netApplyConfig() {
  configTzTime(cfg.tz, "pool.ntp.org", "time.nist.gov");
  if (cfg.ssid[0] == '\0') {
    if (!apOn) startAp();
    return;
  }
  staAttempts = 0;
  WiFi.disconnect();
  staConnect();
}

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
