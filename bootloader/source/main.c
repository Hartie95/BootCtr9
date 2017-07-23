#include <stdio.h>
#include <stdlib.h>
#include "sdmmc.h"
#include "ff.h"
#include "config.h"
#include "hid.h"
#include "log.h"
#include "splash.h"
#include "helpers.h"
#include "constants.h"
#include "payload.h"
#include "draw.h"

#include "../../arm11bg/source/arm11bg/constants.h"

char latestSection[15]={0};

int getTargetPayload(loaderConfiguration* loader, configuration* app,
	FIL* configFile)
{
	// Check if arm9Companion support is enabled, the companion section defined and the payload available
	if(loader->enableArm9ComapnionBoot)
	{
		app->section=COMPANION_SECTION;
		readPayloadSection(app, configFile);
		if (!checkPayload(app)) 
		{
	    	 return TARGET_ARM9_COMPANION;
	    }
	}

	// Check if autoboot is enabled and currently softbooting
	if(loader->enableAutosoftboot && !isColdboot())
	{
		FIL latestFile;
		char latestFilePath[32]={0};
		unsigned int br=0;

		debug("Opening %s file",LASTEST_SECTION_FILE);
		checkFolders(LASTEST_SECTION_FILE, latestFilePath);

		if(f_open(&latestFile, latestFilePath, FA_READ) == FR_OK)
		{
			br=0;
		    debug("it's softreboot, read %s",LASTEST_SECTION_FILE);
			f_gets(latestSection, 15, &latestFile);
			br=strlen(latestSection);
			app->section=latestSection;
			f_close(&latestFile);
			if(br>4)
			{
				info("Latest Section: [%s]",app->section);
				readPayloadSection(app, configFile);
				if(!checkPayload(app)) 
			    	return TARGET_AUTOBOOT;
			}
		}
		info("Latest Section not found, falling back to key");
	}

	// Get section from key input if no companion or autoboot payload got loaded
	info("Wait %llu ms for Input",loader->keyDelay);
	u32 key = WaitTimeForInput(loader->keyDelay);

    // using X-macros to generate each if else rule
    // https://en.wikibooks.org/wiki/C_Programming/Preprocessor#X-Macros
    #define KEY(k) \
    if(key & KEY_##k) \
        app->section = "KEY_"#k; \
    else
    #include "keys.def"
        app->section = "DEFAULT";

	info("Key checked-selected section: [%s]",app->section);

	//Check if it's not a default payload boot and check for a bootpassword 
	if(strcmp(app->section,"DEFAULT"))
		if(!checkPassword(loader->bootPassword))
        	app->section = "DEFAULT";

	readPayloadSection(app, configFile);
	if (checkPayload(app)) 
	{
    	debug("Trying to load the [DEFAULT] section");
        app->section = "DEFAULT";
        // don't need to check error again
        readPayloadSection(app, configFile);
        if (checkPayload(app))
            panic("Failed to load the [DEFAULT] section");
    }

    return 0;
}

struct fb fbs[2] =
{
    {
        .top_left  = (u8 *)0x18300000,
        .top_right = (u8 *)0x18300000,
        .bottom    = (u8 *)0x18346500,
    },
    {
        .top_left  = (u8 *)0x18400000,
        .top_right = (u8 *)0x18400000,
        .bottom    = (u8 *)0x18446500,
    },
};

u8 *top_screen, *top_screen2, *bottom_screen;
extern bool forceScreenInit;

int main(int argc, char **argv, u32 magicWord) {
	FATFS fs;
	FIL configFile;
	int payloadSource = 0;

    loaderConfiguration loader =  {
	        .section = LOADER_SECTION,
	        .bootPassword = DEFAULT_BOOT_PASSWORD,
	        .keyDelay = DEFAULT_KEYDELAY,
	        .bootsplash = DEFAULT_BOOTSPLASH, 
            .bootsplash_image = DEFAULT_BOOTSPLASH_IMAGE,
            .enableSoftbootSplash = DEFAULT_SOFTBOOT_SPLASH_LOADER,
            .enableAutosoftboot = DEFAULT_AUTOSOFTBOOT,
            .fileLog = DEFAULT_LOG_FILE,
            .screenLog = DEFAULT_LOG_SCREEN,
	        .screenEnabled = DEFAULT_SCREEN,
	        .screenBrightness = DEFAULT_BOOTBRIGHTNESS,
			.deviceID=0,
			.CTCertPath=DEFAULT_EMPTY_PATH,
    };

	configuration app =  {
	        .section = DEFAULT_SECTION,
	        .path = DEFAULT_PATH,
	        .splashDelay = DEFAULT_SPLASHDELAY,
	        .payload = DEFAULT_PAYLOAD,
	        .offset = DEFAULT_OFFSET,
	        .splash = DEFAULT_SPLASH, 
	        .splash_image = DEFAULT_SPLASH_IMAGE,
	        .enableSoftbootSplash = DEFAULT_SOFTBOOT_SPLASH,
	        .screenEnabled = DEFAULT_SCREEN,
	        .screenBrightness = DEFAULT_BRIGHTNESS,
	        .fixArm9Path = 0,
	        .keysPath = DEFAULT_EMPTY_PATH,
    };

	if(argc >= 2) {
        // newer entrypoints
        struct fb* fbstmp=(struct fb*)(void *)argv[1];
        fbs[0].top_left=fbstmp[0].top_left;
        fbs[0].top_right=fbstmp[0].top_right;
        fbs[0].bottom=fbstmp[0].bottom;
        fbs[1].top_left=fbstmp[1].top_left;
        fbs[1].top_right=fbstmp[1].top_right;
        fbs[1].bottom=fbstmp[1].bottom;

        top_screen = fbstmp[0].top_left;
        top_screen2 = fbstmp[0].top_right;
        bottom_screen = fbstmp[0].bottom;
        forceScreenInit=true;
    } else {
		top_screen = (u8*)(*(u32*)0x23FFFE00);
        top_screen2 = (u8*)(*(u32*)0x23FFFE04);
        bottom_screen = (u8*)(*(u32*)0x23FFFE08);
    }

    f_mount(&fs, "0:", 0);

	if(!openIniFile(&configFile))
		panic("Config file not found.");

	*((volatile u32 *)ARM11LOADER_IDENTIFIER_ADDRESS)=magicWord;

	// Read and apply BootCTR9 configuration
	iniparse(handlerLoaderConfiguration,&loader,&configFile);
	initLog(loader.fileLog, loader.screenLog);

	setBrightness(loader.screenBrightness);
	setScreenState(loader.screenEnabled);

	//boot9strap
    if((magicWord&0xFFFF) == 0xBEEF){
		debug("Boot9Strap");
    }
    //arm9loaderhax from bootloaderloader
    else if (magicWord==0x00A91A91){
		debug("arm9loaderhax");
    }
    //unknown source
    else {
		debug("unknown");
    }

    debug("Bootet from Path: %s", argv[0]);

	//show splash only with screen init
	if(loader.screenEnabled)
	{
		drawBootSplash(&loader);
	}

	copyDeviceID(loader.deviceID);

	loadCTCert(loader.CTCertPath);

	// Read Global Section for default Configurations
	debug("Reading [GLOBAL] section");
	readPayloadSection(&app,&configFile);

	payloadSource = getTargetPayload(&loader, &app, &configFile);

	if(!loadPayload(&loader, &app, payloadSource))
	{
		debug("Closing files and unmount sd");
		f_close(&configFile);
		closeLogFile();
		f_mount(&fs, "0:", 1);

		runPayload(&app);	
	}
	panic("Failed to mount the sd-card or load the payload");
	return 0;
}
