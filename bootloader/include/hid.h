#pragma once

#include "common.h"

#define HID_STATE (*(volatile u32*)0x10146000)

#define KEY_A      (1 << 0)
#define KEY_B      (1 << 1)
#define KEY_SELECT (1 << 2)
#define KEY_START  (1 << 3)
#define KEY_DRIGHT (1 << 4)
#define KEY_DLEFT  (1 << 5)
#define KEY_DUP    (1 << 6)
#define KEY_DDOWN  (1 << 7)
#define KEY_R     (1 << 8)
#define KEY_L     (1 << 9)
#define KEY_X      (1 << 10)
#define KEY_Y      (1 << 11)


u32 WaitTimeForInput(u32 waittime);
u32 InputWait();
u32 GetInput();
