#include "admin.h"

#include <Arduino.h>
#include <WebServer.h>
#include <WiFi.h>

#include "climate.h"
#include "modes.h"
#include "net.h"
#include "settings.h"

namespace {
WebServer srv(80);

String hm(uint16_t minutes) {
  char b[8];
  snprintf(b, sizeof(b), "%02u:%02u", minutes / 60, minutes % 60);
  return String(b);
}

const char* modeName() {
  switch (modesCurrent()) {
    case Mode::Idle: return "Ожидание";
    case Mode::Alert: return "Вызов";
    case Mode::Sleep: return "Сон";
  }
  return "?";
}

String page(const char* notice) {
  String s;
  s.reserve(5000);
  s += F("<!doctype html><html><head><meta charset='utf-8'>"
         "<meta name='viewport' content='width=device-width,initial-scale=1'>"
         "<title>Pendel</title><style>"
         "body{font-family:sans-serif;max-width:26em;margin:2em auto;padding:0 1em}"
         "label{display:block;margin:.7em 0 .2em;font-weight:600}"
         "input{width:100%;box-sizing:border-box;padding:.4em;font-size:1em}"
         "button{margin-top:1.2em;padding:.6em 1.6em;font-size:1em}"
         "table{border-collapse:collapse}td{padding:.15em .8em .15em 0}"
         ".ok{background:#dfd;padding:.5em;margin:.8em 0}"
         "</style></head><body><h1>Pendel</h1>");

  if (notice && notice[0]) {
    s += F("<div class='ok'>");
    s += notice;
    s += F("</div>");
  }

  // --- Статус ---
  s += F("<h2>Статус</h2><table>");
  s += F("<tr><td>Wi-Fi</td><td>");
  if (netConnected()) {
    s += String(cfg.ssid) + " — " + WiFi.localIP().toString();
  } else if (netProvisioning()) {
    s += F("режим настройки (AP Pendel-Setup)");
  } else {
    s += F("подключение...");
  }
  s += F("</td></tr>");
  s += F("<tr><td>Админка</td><td>http://");
  s += netAdminIp();
  s += F("/</td></tr>");
  s += F("<tr><td>Время</td><td>");
  s += netTimeValid() ? F("синхронизировано") : F("не синхронизировано");
  s += F("</td></tr><tr><td>Климат</td><td>");
  if (climateValid()) {
    char b[48];
    snprintf(b, sizeof(b), "%.1f°C, %.0f%%, %.1f гПа", climateTemp(),
             climateHum(), climatePressure());
    s += b;
  } else {
    s += F("датчик недоступен");
  }
  s += F("</td></tr><tr><td>Режим</td><td>");
  s += modeName();
  s += F("</td></tr></table>");

  // --- Форма настроек ---
  s += F("<h2>Настройки</h2><form method='post' action='/save'>"
         "<label>Wi-Fi SSID</label><input name='ssid' value='");
  s += cfg.ssid;
  s += F("'>"
         "<label>Wi-Fi пароль (пусто — не менять)</label>"
         "<input type='password' name='pass'>"
         "<label>Время отбоя</label><input type='time' name='bedtime' value='");
  s += hm(cfg.bedtimeMin);
  s += F("'><label>Сон с</label><input type='time' name='sleepS' value='");
  s += hm(cfg.sleepStartMin);
  s += F("'><label>Сон до</label><input type='time' name='sleepE' value='");
  s += hm(cfg.sleepEndMin);
  s += F("'><label>Длительность вызова, сек</label>"
         "<input type='number' name='callDur' min='5' max='600' value='");
  s += String(cfg.callDurationSec);
  s += F("'><label>Часовой пояс (POSIX TZ)</label>"
         "<input name='tz' value='");
  s += cfg.tz;
  s += F("'><label>Имя первого ребёнка</label><input name='child1' value='");
  s += cfg.child1;
  s += F("'><label>Имя второго ребёнка</label><input name='child2' value='");
  s += cfg.child2;
  s += F("'><label>Токен Telegram-бота (пусто — не менять)</label>"
         "<input type='password' name='tgTok'>"
         "<button type='submit'>Сохранить</button></form></body></html>");
  return s;
}

bool parseHm(const String& v, uint16_t& out) {
  unsigned h, m;
  if (sscanf(v.c_str(), "%u:%u", &h, &m) != 2) return false;
  if (h > 23 || m > 59) return false;
  out = h * 60 + m;
  return true;
}

void copyArg(char* dst, size_t n, const String& name) {
  if (!srv.hasArg(name)) return;
  String v = srv.arg(name);
  v.trim();
  strncpy(dst, v.c_str(), n - 1);
  dst[n - 1] = '\0';
}

void handleRoot() { srv.send(200, "text/html", page("")); }

void handleSave() {
  bool wifiChanged = false;

  if (srv.hasArg("ssid")) {
    String v = srv.arg("ssid");
    v.trim();
    if (strcmp(v.c_str(), cfg.ssid) != 0) {
      strncpy(cfg.ssid, v.c_str(), sizeof(cfg.ssid) - 1);
      cfg.ssid[sizeof(cfg.ssid) - 1] = '\0';
      wifiChanged = true;
    }
  }
  if (srv.hasArg("pass") && srv.arg("pass").length() > 0) {
    String v = srv.arg("pass");
    strncpy(cfg.pass, v.c_str(), sizeof(cfg.pass) - 1);
    cfg.pass[sizeof(cfg.pass) - 1] = '\0';
    wifiChanged = true;
  }
  uint16_t m;
  if (srv.hasArg("bedtime") && parseHm(srv.arg("bedtime"), m))
    cfg.bedtimeMin = m;
  if (srv.hasArg("sleepS") && parseHm(srv.arg("sleepS"), m))
    cfg.sleepStartMin = m;
  if (srv.hasArg("sleepE") && parseHm(srv.arg("sleepE"), m))
    cfg.sleepEndMin = m;
  if (srv.hasArg("callDur")) {
    long d = srv.arg("callDur").toInt();
    if (d >= 5 && d <= 600) cfg.callDurationSec = d;
  }
  copyArg(cfg.tz, sizeof(cfg.tz), "tz");
  copyArg(cfg.child1, sizeof(cfg.child1), "child1");
  copyArg(cfg.child2, sizeof(cfg.child2), "child2");
  if (srv.hasArg("tgTok") && srv.arg("tgTok").length() > 0)
    copyArg(cfg.tgToken, sizeof(cfg.tgToken), "tgTok");

  settingsSave();
  settingsPrint();
  if (wifiChanged) netApplyConfig();

  srv.send(200, "text/html", page("Настройки сохранены"));
}

void handleNotFound() {
  srv.sendHeader("Location", "/", true);
  srv.send(302, "text/plain", "");
}
}  // namespace

void adminBegin() {
  srv.on("/", HTTP_GET, handleRoot);
  srv.on("/save", HTTP_POST, handleSave);
  srv.onNotFound(handleNotFound);
  srv.begin();
  Serial.printf("[admin] админка: http://%s/\n", netAdminIp().c_str());
}

void adminTick() { srv.handleClient(); }
