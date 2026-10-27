#include "modes.h"

#include <Arduino.h>

#include "button.h"
#include "display.h"
#include "net.h"
#include "settings.h"
#include "signaling.h"

namespace {
constexpr int WARN_1_MIN = 15;
constexpr int WARN_2_MIN = 5;
constexpr uint32_t MS_PER_SEC = 1000;

Mode mode = Mode::Idle;
uint32_t callStart = 0;
CallTarget callTarget = CallTarget::Both;
char callMsg[64] = "";
const char* targetLabels[] = {"РЕБЁНОК 1", "РЕБЁНОК 2", "ОБА"};

bool warned1 = false, warned2 = false;
int lastWarnDay = -1;

bool inSleepWindow(int nowMin) {
  uint16_t s = cfg.sleepStartMin, e = cfg.sleepEndMin;
  return s <= e ? (nowMin >= s && nowMin < e) : (nowMin >= s || nowMin < e);
}

void enterMode(Mode m) {
  if (mode == m) return;
  Serial.printf("[modes] %d -> %d\n", (int)mode, (int)m);
  mode = m;
  switch (m) {
    case Mode::Idle:
      sigStop();
      dispPowerSave(false);
      break;
    case Mode::Alert:
      callStart = millis();
      sigStartCall();
      dispPowerSave(false);
      dispShowAlert(targetLabels[(int)callTarget], callMsg);
      break;
    case Mode::Sleep:
      sigStop();
      dispPowerSave(true);
      break;
  }
}

void resetWarnFlagsIfNewDay() {
  struct tm t;
  if (!netLocalTime(t)) return;
  if (t.tm_yday != lastWarnDay) {
    lastWarnDay = t.tm_yday;
    warned1 = warned2 = false;
  }
}
}  // namespace

void modesBegin() {
  btnBegin();
  mode = Mode::Idle;
}

Mode modesCurrent() { return mode; }

void triggerCall(CallTarget target, const char* message) {
  if (mode == Mode::Sleep) {
    Serial.println(F("[modes] вызов в режиме сна — игнор"));
    return;
  }
  callTarget = target;
  strncpy(callMsg, message ? message : "", sizeof(callMsg) - 1);
  callMsg[sizeof(callMsg) - 1] = '\0';
  enterMode(Mode::Alert);
}

const char* modesAlertTargetLabel() { return targetLabels[(int)callTarget]; }
const char* modesAlertMessage() { return callMsg; }

void modesTick() {
  bool pressed = btnTick();
  int nowMin = netNowMinutes();
  bool timeOk = nowMin >= 0;
  resetWarnFlagsIfNewDay();

  // Переходы по расписанию сна (только при валидном времени).
  if (timeOk) {
    bool inSleep = inSleepWindow(nowMin);
    if (inSleep && mode != Mode::Sleep) {
      enterMode(Mode::Sleep);
      return;
    }
    if (!inSleep && mode == Mode::Sleep) {
      enterMode(Mode::Idle);
      return;
    }
  }

  switch (mode) {
    case Mode::Idle: {
      if (timeOk) {
        int left = (cfg.bedtimeMin - nowMin + 1440) % 1440;
        if (left == WARN_1_MIN && !warned1) {
          warned1 = true;
          sigWarn();
          Serial.println(F("[modes] предупреждение: 15 мин до отбоя"));
        }
        if (left == WARN_2_MIN && !warned2) {
          warned2 = true;
          sigWarn();
          Serial.println(F("[modes] предупреждение: 5 мин до отбоя"));
        }
      }
      break;
    }
    case Mode::Alert:
      if (pressed) {
        Serial.println(F("[modes] вызов подтверждён кнопкой"));
        enterMode(Mode::Idle);
      } else if (millis() - callStart >=
                 cfg.callDurationSec * MS_PER_SEC) {
        Serial.println(F("[modes] вызов: таймаут"));
        enterMode(Mode::Idle);
      }
      break;
    case Mode::Sleep:
      break;
  }
}
