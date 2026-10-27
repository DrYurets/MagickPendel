#pragma once

#include <Arduino.h>

void netBegin();
void netTick();
bool netConnected();
// Время считается валидным после первой успешной NTP-синхронизации.
bool netTimeValid();
// Минуты от полуночи по локальному времени; -1 если время невалидно.
int netNowMinutes();
// Текущее время в tm; false если невалидно.
bool netLocalTime(struct tm& out);
