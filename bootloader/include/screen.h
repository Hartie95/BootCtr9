#pragma once

#include "common.h"

typedef void (*arm11Function)(void); 
void startArm11BackgroundProcess();
void changeBrightness(u32 _brightness);
bool screenInit();
void screenShutdown();
