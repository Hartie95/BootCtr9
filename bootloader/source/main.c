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

int main() {
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
    };

    f_mount(&fs, "0:", 0);
	
	if(!openIniFile(&configFile))
		panic("Config file not found.");

	// Read and apply BootCTR9 configuration
	iniparse(handlerLoaderConfiguration,&loader,&configFile);
	initLog(loader.fileLog, loader.screenLog);
	setBrightness(loader.screenBrightness);
	setScreenState(loader.screenEnabled);

	//show splash only with screen init
	if(loader.screenEnabled)
	{
		drawBootSplash(&loader);
	}

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
	panic("Failed to mount the sd-card or laod the payload");
	return 0;
}
