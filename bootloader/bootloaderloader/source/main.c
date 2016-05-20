#include "common.h"
#include "sdmmc.h"
#include "i2c.h"
#include "ff.h"

#define BOOTLOADER_PAYLOAD_ADDRESS	0x24F00000
#define PAYLOAD_SIZE		0x00100000

FATFS fs;

void loadAndRunPayload(const char* payloadName, u32 payloadAddress)
{
	FIL payload;
	u32 br;
	if(f_open(&payload, payloadName, FA_READ | FA_OPEN_EXISTING) == FR_OK)
	{
		f_read(&payload, (void*)payloadAddress, PAYLOAD_SIZE, (UINT*)&br);
        f_close(&payload);
		f_mount(&fs, "0:", 1);
		((void (*)())payloadAddress)();
	}
}

int main()
{
	if(f_mount(&fs, "0:", 1) == FR_OK)
	{
		loadAndRunPayload("arm9loaderhax/arm9bootloader.bin", BOOTLOADER_PAYLOAD_ADDRESS);
		loadAndRunPayload("a9lh/arm9bootloader.bin", BOOTLOADER_PAYLOAD_ADDRESS);
		loadAndRunPayload("arm9bootloader/arm9bootloader.bin", BOOTLOADER_PAYLOAD_ADDRESS);
		loadAndRunPayload("arm9bootloader.bin", BOOTLOADER_PAYLOAD_ADDRESS);
	}
	
	i2cWriteRegister(I2C_DEV_MCU, 0x20, (u8)(1<<0));
    return 0;
}
