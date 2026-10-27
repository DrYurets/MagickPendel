#pragma once

// Неблокирующий паттерн-движок: светодиод + активный баззер играют
// один и тот же ритм (on/off). Циклический — для вызова, однократный —
// для предупреждений перед отбоем.

void sigBegin();
void sigStartCall();  // бесконечный цикл до sigStop()
void sigWarn();       // короткая серия, завершается сама
void sigStop();
void sigTick();
bool sigActive();
