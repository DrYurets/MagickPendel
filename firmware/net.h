#pragma once

#include <Arduino.h>

void netBegin();
void netTick();
bool netConnected();
// Активен режим настройки: поднята точка доступа Pendel-Setup.
bool netProvisioning();
// IP-адрес для админки: STA-IP либо AP-IP в режиме настройки.
String netAdminIp();
// Перечитать cfg: переподключение к Wi-Fi и смена таймзоны без ребута.
void netApplyConfig();
// Время считается валидным после первой успешной NTP-синхронизации.
bool netTimeValid();
// Минуты от полуночи по локальному времени; -1 если время невалидно.
int netNowMinutes();
// Текущее время в tm; false если невалидно.
bool netLocalTime(struct tm& out);
