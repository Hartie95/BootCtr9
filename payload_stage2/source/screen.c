#include "screen.h"
#include "i2c.h"

void turnOnBacklight(void)
{
	i2cWriteRegister(3, 0x22, 0x2A); // 0x2A -> boot into firm with no backlight
}
