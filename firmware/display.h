#pragma once

#include <Arduino.h>

void dispBegin();
// Перерисовка текущего экрана (~1 раз/сек), сама следит за периодом.
void dispTick();
void dispShowAlert(const char* targetLabel, const char* message);
void dispShowIdle();
// Экран режима настройки: SSID точки доступа и адрес админки.
void dispShowSetup();
// Вкл/выкл дисплей (режим сна).
void dispPowerSave(bool on);
