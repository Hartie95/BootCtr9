#include "common.h"
#include "i2c.h"
#include "firm.h"
#include "cache.h"

//#include "ff.h"
#include "../../../arm11bg/source/arm11bg/constants.h"

extern u8 BootCTR9_firm[];
extern u32 BootCTR9_firm_size;

void fastmempy(vu8* destination, vu8* source, u32 size)
{
	u32 end=size%4;
	size/=4;
	
	while(size--)
	{
		*(vu32*)destination=*(vu32*)source;
		destination+=sizeof(u32);
		source+=sizeof(u32);
	}

	while(end--){
		*destination=*source;
		destination++;
		source++;
	}
}

#define PAYLOAD_SIZE		0x00100000

int main()
{
/*	loading from files */
//	FIL payload;
//	unsigned int br=0;
//	FATFS fs;
//	f_mount(&fs, "0:", 0);

//  Firm *firm = (Firm *)0x24000000;
//	if(f_open(&payload, "/boot.firm", FA_READ | FA_OPEN_EXISTING) != FR_OK)
//		i2cWriteRegister(I2C_DEV_MCU, 0x20, (u8)(1<<0));

//	f_read(&payload, (void*)firm, PAYLOAD_SIZE, &br);
//	f_close(&payload);

	Firm *firm = (Firm *)BootCTR9_firm;
	
	for(u32 sectionNum = 0; sectionNum < 4 && firm->section[sectionNum].size != 0; sectionNum++)
    	memcpy(firm->section[sectionNum].address, (u8 *)firm + firm->section[sectionNum].offset, firm->section[sectionNum].size);
    
    *((vu32 *)ARM11LOADER_IDENTIFIER_ADDRESS)=ARM9LOADERHAX_IDENTIFIER;
	flushCaches();

	*(vu32 *)0x1FFFFFF8 = (u32)firm->arm11Entry;

    //Jump to ARM9 entrypoint. give it the path as parameter
    char *argv[1] = {""};

	flushCaches();

    ((void (*)(int, char**, u32))firm->arm9Entry)(1, argv, ARM9LOADERHAX_IDENTIFIER);

	//We should not end here
	i2cWriteRegister(I2C_DEV_MCU, 0x20, (u8)(1<<0));
    return 0;
}
