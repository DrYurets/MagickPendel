#pragma once

#include <Arduino.h>

// Распиновка ESP32-C3 SuperMini (см. openspec/changes/init-firmware/design.md)
constexpr uint8_t PIN_I2C_SDA = 4;
constexpr uint8_t PIN_I2C_SCL = 5;
constexpr uint8_t PIN_BUTTON = 10;   // INPUT_PULLUP, замыкание на GND
constexpr uint8_t PIN_BUZZER = 3;    // активный баззер, HIGH = звук
constexpr uint8_t PIN_LED = 7;       // внешний светодиод, HIGH = горит
constexpr uint8_t PIN_LED_ONBOARD = 8; // встроенный LED SuperMini, active low
