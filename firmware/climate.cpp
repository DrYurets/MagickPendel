#include "climate.h"

#include <Adafruit_AHTX0.h>
#include <Adafruit_BMP280.h>
#include <Arduino.h>

namespace {
constexpr uint32_t POLL_INTERVAL_MS = 5000;

Adafruit_AHTX0 aht;
Adafruit_BMP280 bmp;

bool ahtOk = false;
bool bmpOk = false;
bool valid = false;
float temp = NAN, hum = NAN, press = NAN;
uint32_t lastPoll = 0;
uint32_t logCounter = 0;
}  // namespace

void climateBegin() {
  ahtOk = aht.begin();
  bmpOk = bmp.begin(0x76) || bmp.begin(0x77);
  Serial.printf("[climate] AHT20: %s, BMP280: %s\n", ahtOk ? "OK" : "не найден",
                bmpOk ? "OK" : "не найден");
}

void climateTick() {
  uint32_t now = millis();
  if (now - lastPoll < POLL_INTERVAL_MS) return;
  lastPoll = now;

  bool ok = false;
  if (ahtOk) {
    sensors_event_t h, t;
    if (aht.getEvent(&h, &t)) {
      temp = t.temperature;
      hum = h.relative_humidity;
      ok = true;
    } else {
      ahtOk = aht.begin();  // попытка перезапуска
    }
  }
  if (bmpOk) {
    float p = bmp.readPressure();
    if (!isnan(p)) {
      press = p / 100.0f;
      ok = true;
    } else {
      bmpOk = bmp.begin(0x76) || bmp.begin(0x77);
    }
  }
  valid = ok;
  if (!ok && ++logCounter % 12 == 0)
    Serial.println(F("[climate] датчик не отвечает, повторяю попытки"));
}

bool climateValid() { return valid; }
float climateTemp() { return temp; }
float climateHum() { return hum; }
float climatePressure() { return press; }
