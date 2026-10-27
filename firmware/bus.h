#pragma once

#include <Arduino.h>

// Сканирование I2C-шины с выводом в Serial и запоминанием найденных адресов.
void busScan();
// Устройство найдено на шине при последнем сканировании.
bool busPresent(uint8_t addr);
// Одноразовая проверка адреса прямо сейчас.
bool busProbe(uint8_t addr);
