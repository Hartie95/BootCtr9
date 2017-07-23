#include "payload.h"
#include "log.h"
#include "helpers.h"
#include "constants.h"
#include "splash.h"
#include "firm.h"
#include "screen.h"
#include "draw.h"
#include "cache.h"
#include "a9lh.h"

int checkPayload(configuration* app)
{
	debug("Checking payload");
	if (!strlen(app->path)) 
	{
    	debug("Section %s not found, or Path not set",app->section);
    	return ERROR_PATH_NOT_SET;
    }

	if(!file_exists(app->path))
	{
        debug("Target payload %s not found",app->path);
        return ERROR_PAYLOAD_NOT_FOUND;
	}


    /* identify the payloadtype if type is set to auto */
	if(app->payload==-1){
		debug("checking payload type");
		char *dot = strrchr(app->path, '.');
		if(!dot){
			app->payload=1;
		}
		else
		{
			if (!strcmp(dot, ".firm"))
			{
				app->payload=2;
				debug("payload type detected: firm");
			}
			else if (!strcmp(dot, ".dat")||!strcmp(dot, ".bin"))
			{
				app->payload=1;
				debug("payload type detected: arm9binary");
			}
			else if (!strcmp(dot, ".3dsx"))
			{
				app->payload=0;
				debug("payload type detected: .3dsx");
			}
		}
	}

	if(!app->payload)
	{
		panic("Trying to load a 3dsx - this is not supported by arm9");
	}

	return 0;
}

/* This does on the fly path patching, which is needed for lumas reboot patches on older a9lh versions */
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


Firm *firm = (Firm *)0x24000000;

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

	switch(app->payload)
	{
		//Firm
		case 2:
			f_read(&payload, (void*)0x24000000, PAYLOAD_SIZE, &br);
			f_close(&payload);

			for(u32 sectionNum = 0; sectionNum < 4 && firm->section[sectionNum].size != 0; sectionNum++)
			memcpy(firm->section[sectionNum].address, (u8 *)firm + firm->section[sectionNum].offset, firm->section[sectionNum].size);
			break;

	    //arm9binary
		case 1:
		default:
			// Read payload from sd to memory address 0x23F00000
			debug("Reading payload at offset %i",app->offset);
			if(app->offset)
				f_lseek (&payload, app->offset);
			f_read(&payload, (void*)PAYLOAD_ADDRESS, PAYLOAD_SIZE, &br);
			f_close(&payload);
			debug("Finished reading the payload");

			if(app->fixArm9Path)
				patchPath(br, app->path);
	}

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
    if(loader->enableAutosoftboot && isColdboot())
    {
    	checkFolders(LASTEST_SECTION_FILE, latestFilePath);

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

	setupKeys(app->keysPath);

	debug("prepearing jump");

	// jump to the payload
	switch(app->payload)
	{
		//Firm
		case 2:
			debug("Set ARM11 entrypoint");
			//Set ARM11 entrypoint
		    *(vu32 *)0x1FFFFFFC = (u32)firm->arm11Entry;

		    //Jump to ARM9 entrypoint. give it the path as parameter
			debug("Jump to ARM9 entrypoint");
			u32 argc=1;
		    char *argv[2];


			if(firm->reserved2[0] & 1)
			{
				struct fb fbstmp[2] =
			    {
			        {
			            .top_left  = (u8*)*(volatile u32*)0x80FFFC0,
			            .top_right = (u8*)*(volatile u32*)0x80FFFC8,
			            .bottom    = (u8*)*(volatile u32*)0x80FFFD0,
			        },
			        {
			            .top_left  = (u8*)*(volatile u32*)0x80FFFC4 ,
			            .top_right = (u8*)*(volatile u32*)0x80FFFCC,
			            .bottom    = (u8*)*(volatile u32*)0x80FFFD4,
			        },
			    };

		        argv[1] = (char *)&fbstmp;
		        argc = 2;
		    }

			char absPath[24 + _MAX_LFN];
	        sprintf(absPath, "sdmc:%s", app->path);
	        argv[0]=absPath;

	        flushCaches();

		    ((void (*)(int, char**, u32))firm->arm9Entry)(argc, argv, 0x0000BEEF);
		    break;

	    //arm9binary TODO
	    case 1:
	    default:
			loadStub();

			/* todo better compability with older a9lh payloads */
			inita9lh();
			flushCaches();

			debug("Jump to ARM9 entrypoint");
			((void (*)())PAYLOAD_ADDRESS)();
}
	panic("Failed to jump to the Payload");
}
