#include "display.h"

#include <U8g2lib.h>

#include "bus.h"
#include "climate.h"
#include "net.h"
#include "settings.h"

namespace {
constexpr uint8_t OLED_ADDR = 0x3C;
U8G2_SSD1306_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, U8X8_PIN_NONE);

constexpr uint32_t REFRESH_MS = 1000;
constexpr uint32_t PROBE_MS = 10000;
uint32_t lastDraw = 0;
uint32_t lastProbe = 0;
bool asleep = false;
bool dirty = true;
bool present = false;

void center(const char* s, int y) {
  int w = u8g2.getUTF8Width(s);
  u8g2.drawUTF8((128 - w) / 2, y, s);
}

// Копирует не более maxChars UTF-8 символов, не разрывая многобайтные
// последовательности. Возвращает число прочитанных байт.
size_t utf8Prefix(char* dst, const char* src, int maxChars) {
  size_t i = 0;
  int n = 0;
  while (src[i] && n < maxChars) {
    uint8_t b = (uint8_t)src[i];
    size_t len = (b < 0x80) ? 1 : (b < 0xE0) ? 2 : (b < 0xF0) ? 3 : 4;
    for (size_t k = 0; k < len && src[i]; k++) dst[i] = src[i], i++;
    n++;
  }
  dst[i] = '\0';
  return i;
}
}  // namespace

void dispBegin() {
  present = busPresent(OLED_ADDR) || busProbe(OLED_ADDR);
  if (!present) {
    Serial.println(F("[display] OLED не найден, вывод отключён"));
    return;
  }
  u8g2.begin();
  u8g2.setBusClock(400000);
  u8g2.enableUTF8Print();
}

void dispPowerSave(bool on) {
  asleep = on;
  if (present) u8g2.setPowerSave(on ? 1 : 0);
  dirty = true;
}

void dispShowIdle() {
  if (!present) return;
  char clockStr[8] = "--:--";
  char countStr[24] = "до отбоя --:--";
  struct tm t;
  if (netLocalTime(t)) {
    snprintf(clockStr, sizeof(clockStr), "%02d:%02d", t.tm_hour, t.tm_min);
    int nowMin = t.tm_hour * 60 + t.tm_min;
    int left = (cfg.bedtimeMin - nowMin + 1440) % 1440;
    snprintf(countStr, sizeof(countStr), "до отбоя %d:%02d", left / 60,
             left % 60);
  }

  char climStr[24] = "--.-°  --%";
  if (climateValid())
    snprintf(climStr, sizeof(climStr), "%.1f°  %.0f%%", climateTemp(),
             climateHum());

  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_logisoso24_tn);
  center(clockStr, 30);
  u8g2.setFont(u8g2_font_9x15_t_cyrillic);
  center(climStr, 48);
  center(countStr, 63);
  u8g2.sendBuffer();
}

void dispShowAlert(const char* targetLabel, const char* message) {
  if (!present) return;
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_10x20_t_cyrillic);
  center(targetLabel, 26);
  if (message && message[0]) {
    u8g2.setFont(u8g2_font_9x15_t_cyrillic);
    // Максимум две строки по ~14 символов.
    char line[32];
    size_t used = utf8Prefix(line, message, 14);
    center(line, 48);
    if (message[used]) {
      utf8Prefix(line, message + used, 14);
      center(line, 63);
    }
  }
  u8g2.sendBuffer();
}

void dispShowSetup() {
  if (!present) return;
  u8g2.clearBuffer();
  u8g2.setFont(u8g2_font_9x15_t_cyrillic);
  center("Настройка Wi-Fi", 18);
  u8g2.setFont(u8g2_font_10x20_t_cyrillic);
  center("Pendel-Setup", 42);
  u8g2.setFont(u8g2_font_9x15_t_cyrillic);
  center("192.168.4.1", 62);
  u8g2.sendBuffer();
}

void dispTick() {
  if (asleep) return;
  uint32_t now = millis();
  if (!present) {
    // Периферия могла появиться позже — редкая проверка шины.
    if (now - lastProbe < PROBE_MS) return;
    lastProbe = now;
    if (busProbe(OLED_ADDR)) {
      present = true;
      dispBegin();
    }
    return;
  }
  if (!dirty && now - lastDraw < REFRESH_MS) return;
  lastDraw = now;
  dirty = false;
  if (netProvisioning()) {
    dispShowSetup();
  } else {
    dispShowIdle();
  }
}
