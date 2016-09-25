#include "common.h"
#include "i2c.h"
#include "fatfs/ff.h"
#include "screen.h"
#include "hid.h"
#include "flush.h"

#define BOOTLOADER_PAYLOAD_ADDRESS	0x24F00000
#define PAYLOAD_ADDRESS		0x23F00000
#define PAYLOAD_SIZE		0x00100000
#define A11_PAYLOAD_LOC     0x1FFF4C80  //keep in mind this needs to be changed in the ld script for screen_init too
#define A11_ENTRY			0x1FFFFFF8


extern u8 arm11bg_bin[];
extern u32 arm11bg_bin_size;

static inline void* copy_memory(void *dst, void *src, size_t amount)
{
	void *result = dst;
	while (amount--)
	{
		*((char*)(dst++)) = *((char*)(src++));
	}
	return result;
}


static void ownArm11()
{
	copy_memory((void*)A11_PAYLOAD_LOC, arm11bg_bin, arm11bg_bin_size);
	*(vu32 *)A11_ENTRY = 1;
	*((u32*)0x1FFAED80) = 0xE51FF004;
	*((u32*)0x1FFAED84) = A11_PAYLOAD_LOC;
	*((u8*)0x1FFFFFF0) = 2;

	//AXIWRAM isn't cached, so this should just work
	while(*(volatile uint32_t *)A11_ENTRY);
}

void loadAndRunPayload(const char* payloadName, u32 payloadAddress)
{
	FIL payload;
	u32 br;
	if(f_open(&payload, payloadName, FA_READ | FA_OPEN_EXISTING) == FR_OK)
	{
		ownArm11();
		turnOnBacklight();
		setupDefaultFramebuffers();
		f_read(&payload, (void*)payloadAddress, f_size(&payload), (UINT*)&br);
		flush_all_caches();
		((void (*)(void))payloadAddress)();
	}
}

int main()
{
	FATFS fs;
	f_mount(&fs, "0:", 0); //This never fails due to deferred mounting

	loadAndRunPayload("arm9loaderhax/arm9bootloader.bin", BOOTLOADER_PAYLOAD_ADDRESS);
	loadAndRunPayload("a9lh/arm9bootloader.bin", BOOTLOADER_PAYLOAD_ADDRESS);
	loadAndRunPayload("arm9bootloader/arm9bootloader.bin", BOOTLOADER_PAYLOAD_ADDRESS);
	loadAndRunPayload("arm9bootloader.bin", BOOTLOADER_PAYLOAD_ADDRESS);
	loadAndRunPayload("arm9loaderhax.bin", PAYLOAD_ADDRESS);
	
	i2cWriteRegister(I2C_DEV_MCU, 0x20, (u8)(1<<0));
	return 0;
}

