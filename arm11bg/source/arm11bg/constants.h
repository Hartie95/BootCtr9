/* Address to comunitcate with the arm11 thread */
#define ARM11COMMAND_ADDRESS 0x1FFF3000

/* Framebuffer Addresses */
#define FB_TOP_LEFT 0x18300000
#define FB_TOP_RIGHT 0x18300000
#define FB_BOTTOM 0x18346500
//#define FB_TOP_RIGHT 0x1838CA00 //for 3d?

#define DEFAULT_BRIGHTNESS		0xFF

/* Screen contants */
#define BYTES_PER_PIXEL 3
#define SCREEN_WIDTH 240
#define SCREEN_HEIGHT 400

#define SCREEN_SIZE (BYTES_PER_PIXEL * SCREEN_WIDTH * SCREEN_HEIGHT)
