/* FILE and Folder names*/
#define INI_FILE 	"/boot_config.ini"
#define LOGNAME 	"/arm9bootloader.log"
#define LASTESTSETIONFILE "/last.section"
#define SUPPORTEDFOLDERS "/arm9loaderhax","/a9lh","arm9bootloader"

/* Memory constands */
#define PAYLOAD_ADDRESS		0x23F00000
#define PAYLOAD_SIZE		0x00100000

#define CFG_BOOTENV 0x10010000
#define COLDBOOT 0


/* Screen constants */
#define TOP_SCREENL (u8*)(*(u32*)(0x23FFFE00))
#define TOP_SCREENR (u8*)(*(u32*)(0x23FFFE04))
#define BOT_SCREEN  (u8*)(*(u32*)(0x23FFFE08))

#define TOP_FB_SIZE (240*400*3)
#define BOTTOM_FB_SIZE (240*320*3)

#define DEBUGSCREEN BOT_SCREEN

/* Splash constants */

#define TMPSPLASHADDRESS 0x24100000

/* Default configuration values */
#define LOADER_SECTION 			"BOOTCTR9"
#define DEFAULT_SPLASHDELAY 	0
#define DEFAULT_SCREEN_LOADER	1 /* 0 - use payloadConfiguration in the end, 1 - screen init at start */
#define DEFAULT_LOG_FILE 		0
#define DEFAULT_LOG_SCREEN 		0
#define DEFAULT_BOOTSPLASH 		0 /* 0 - disabled, 1 - splash screen, 2 - asciiSplash */ 
#define DEFAULT_BOOTSPLASH_IMAGE {0} 
#define DEFAULT_BOOTBRIGHTNESS	0xFF
#define DEFAULT_SOFTBOOT_SPLASH_LOADER 0
#define DEFAULT_AUTOSOFTBOOT 	0

#define DEFAULT_SECTION 	"GLOBAL"
#define DEFAULT_PATH 		{0}
#define DEFAULT_KEYDELAY	100 /* ms */
#define DEFAULT_PAYLOAD 	-1 /* <0 - auto, 0 - 3dsx(not supported) , >0 - arm9Payload */
#define DEFAULT_OFFSET 		0x12000
#define DEFAULT_SPLASH 		0 /* 0 - disabled, 1 - splash screen, 2 - entry info, 3 - both */ 
#define DEFAULT_SPLASH_IMAGE {0} 
#define DEFAULT_SCREEN 		1
#define DEFAULT_BRIGHTNESS		0xFF
#define DEFAULT_SOFTBOOT_SPLASH 0
