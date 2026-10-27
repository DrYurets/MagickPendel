#pragma once

enum class Mode { Idle, Alert, Sleep };
enum class CallTarget { Child1, Child2, Both };

void modesBegin();
void modesTick();
Mode modesCurrent();
void triggerCall(CallTarget target, const char* message);
const char* modesAlertTargetLabel();
const char* modesAlertMessage();
