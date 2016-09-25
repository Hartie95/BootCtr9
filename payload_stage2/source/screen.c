#include "screen.h"
#include "i2c.h"

void turnOnBacklight(void)
{
	i2cWriteRegister(3, 0x22, 0x2A); // 0x2A -> boot into firm with no backlight
}

void setupDefaultFramebuffers(void)
{
	*(vu32*)0x80FFFC0 = 0x18300000;  // framebuffer 1 top left
	*(vu32*)0x80FFFC4 = 0x18300000;  // framebuffer 2 top left
	*(vu32*)0x80FFFC8 = 0x18300000;  // framebuffer 1 top right
	*(vu32*)0x80FFFCC = 0x18300000;  // framebuffer 2 top right
	*(vu32*)0x80FFFD0 = 0x18346500;  // framebuffer 1 bottom
	*(vu32*)0x80FFFD4 = 0x18346500;  // framebuffer 2 bottom
	*(vu32*)0x80FFFD8 = 1;  // framebuffer select top
	*(vu32*)0x80FFFDC = 1;  // framebuffer select bottom

	//CakeBrah
	*(vu32*)0x23FFFE00 = 0x18300000;
	*(vu32*)0x23FFFE04 = 0x18300000;
	*(vu32*)0x23FFFE08 = 0x18346500;
}
