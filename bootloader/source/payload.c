#include "payload.h"
#include "log.h"
#include "helpers.h"
#include "constants.h"
#include "splash.h"

int checkPayload(configuration* app)
{
	debug("Checking payload");
	if (!strlen(app->path)) 
	{
    	debug("Section %s not found, or Path not set",app->section);
    	return ERROR_PATH_NOT_SET;
    }
	if(!app->payload)
	{	
		panic("Trying to load a 3dsx - this is not supported by arm9");
	}
	
	if(!file_exists(app->path))
	{
        debug("Target payload %s not found",app->path);
        return ERROR_PAYLOAD_NOT_FOUND;
	}
	return 0;
}

/* This does on the fly path patching, which is needed for lumas reboot patches */
int patchPath(u32 size,const char *path)
{
	debug("Fixing payload path");
	const u8 pattern[12] = {'s', 0, 'd', 0, 'm', 0, 'c', 0, ':', 0, '/', 0};
	for(int i=0; i<size;i++)
	{
		if((*(u8 *)(PAYLOAD_ADDRESS+i))==pattern[0] && (*(u8 *)(PAYLOAD_ADDRESS+i+1))==pattern[1])
		{
			if(!memcmp(pattern,(char *)(PAYLOAD_ADDRESS+i),12))
			{
				u32 strgsize=strlen(path);
				u32 shift=10;
				if(path[0]!='/')
					shift=12;
				for(int x=0;x<strgsize;x++)
				{
					*(u8 *)(PAYLOAD_ADDRESS+i+shift+(x*2))=path[x];
					*(u8 *)(PAYLOAD_ADDRESS+i+shift+(x*2)+1)=0;
				}
				*(u8 *)(PAYLOAD_ADDRESS+i+shift+(strgsize*2))=0;
				return 0;
			}
		}
	}
	debug("pattern not found");
	return 1;
}

int loadPayload(loaderConfiguration* loader, configuration* app, int payloadSource)
{
	FIL payload;
	unsigned int br=0;

	FIL latestFile;
	char latestFilePath[32]={0};

	drawSplash(app);

	info("Loading Payload: %s",app->path);
	
	if(f_open(&payload, app->path, FA_READ | FA_OPEN_EXISTING) != FR_OK)
		return ERROR_PAYLOAD_NOT_FOUND;

	// Read payload from sd to memory address 0x23F00000
	debug("Reading payload at offset %i",app->offset);
	if(app->offset)
		f_lseek (&payload, app->offset);
	f_read(&payload, (void*)PAYLOAD_ADDRESS, PAYLOAD_SIZE, &br);
	f_close(&payload);
	debug("Finished reading the payload");

	if(app->fixArm9Path)
		patchPath(br, app->path);

	// Rename arm9Companion payload on after loading
	if(payloadSource==TARGET_ARM9_COMPANION)
	{
		char newpath[64]={0};
		strcpy(newpath, app->path);
		strcat(newpath,".old");
		if(file_exists(newpath))
			f_unlink(newpath);
		f_rename (app->path, newpath);
		return 0;
	}

	// Write latest section to file on coldboot
    if(loader->enableAutosoftboot&&isColdboot())
    {
    	if(f_open(&latestFile, latestFilePath, FA_READ | FA_WRITE | FA_CREATE_ALWAYS )==FR_OK)
    	{
	    	debug("Writing latest section to file: [%s]",app->section);
			f_puts (app->section, &latestFile);
			f_close(&latestFile);
		}           
	}
	return 0;
}

void runPayload(configuration* app)
{
	// Configure screen for payload
	debug("Configuring Screen and jumping to the payload");
	setScreenState(app->screenEnabled);
	setBrightness(app->screenBrightness);

	// jump to the payload
	((void (*)())PAYLOAD_ADDRESS)();
	panic("Failed to jump to the Payload");
}
