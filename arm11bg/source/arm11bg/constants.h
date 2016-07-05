/* Address to comunitcate with the arm11 thread */
#define ARM11COMMAND_ADDRESS 0x1FFF3000

#define ARM11_THREAD_VERSION 0

#define ARM11_COMMANDS typedef struct { \
						    vu32 a11ControllValue; \
						    vu32 a11threadRunning; \
						    vu32 version; \
						    vu32 brightness; \
						    vu32 fbTopSelectedBuffer; \
						    vu32 fbTopLeft; \
						    vu32 fbTopLeft2; \
						    vu32 fbTopRigth; \
						    vu32 fbTopRigth2; \
						    vu32 fbBottomSelectedBuffer; \
						    vu32 fbBottom; \
						    vu32 fbBottom2; \
						    vu32 setBrightness; \
						    vu32 enableLCD; \
						    vu32 mode; \
						    vu32 changeMode; \
						} a11Commands;

/* Framebuffer Addresses */
#define FB_TOP_LEFT 0x18000000
#define FB_TOP_LEFT2 0x18300000
#define FB_TOP_RIGHT FB_TOP_LEFT + SCREEN_SIZE
#define FB_TOP_RIGHT2 FB_TOP_LEFT2 + SCREEN_SIZE
#define FB_BOTTOM FB_TOP_RIGHT + SCREEN_SIZE
#define FB_BOTTOM2 FB_TOP_LEFT2 + SCREEN_SIZE

#define FB_SELECTED_TOP 0x10400478
#define FB_SELECTED_BOT 0x10400578

#define TOP_SCREENL_CAKE *(u32*)(0x23FFFE00)
#define TOP_SCREENR_CAKE *(u32*)(0x23FFFE04)
#define BOT_SCREEN_CAKE  *(u32*)(0x23FFFE08)

#define DEFAULT_BRIGHTNESS		0xFF

/* Screen contants */
#define BYTES_PER_PIXEL 3
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 400

#define SCREEN_SIZE (BYTES_PER_PIXEL * SCREEN_WIDTH * SCREEN_HEIGHT)

#define MODE_MAIN 1
#define MODE_DRAW 2

#define ARM11_DONE 0
#define DISABLE_SCREEN 1
#define ENABLE_SCREEN 2
#define ENABLE_SCREEN_3D 3 //notImplemented
