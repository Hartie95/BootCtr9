#include "screen.h"
#include "i2c.h"
#include "../../../arm11bg/source/arm11bg/constants.h"

void turnOnBacklight(void)
{
	i2cWriteRegister(3, 0x22, 0x2A); // 0x2A -> boot into firm with no backlight
}

void setupDefaultFramebuffers(void)
{
	*(vu32*)0x80FFFC0 = FB_TOP_LEFT;  // framebuffer 1 top left
	*(vu32*)0x80FFFC4 = FB_TOP_LEFT2;  // framebuffer 2 top left
	*(vu32*)0x80FFFC8 = FB_TOP_RIGHT;  // framebuffer 1 top right
	*(vu32*)0x80FFFCC = FB_TOP_RIGHT2;  // framebuffer 2 top right
	*(vu32*)0x80FFFD0 = FB_BOTTOM;  // framebuffer 1 bottom
	*(vu32*)0x80FFFD4 = FB_BOTTOM2;  // framebuffer 2 bottom
	*(vu32*)0x80FFFD8 = 1;  // framebuffer select top
	*(vu32*)0x80FFFDC = 1;  // framebuffer select bottom

	//CakeBrah
	*(vu32*)0x23FFFE00 = FB_TOP_LEFT;
	*(vu32*)0x23FFFE04 = FB_TOP_RIGHT;
	*(vu32*)0x23FFFE08 = FB_BOTTOM;
}
