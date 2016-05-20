
/* Arm11 Background Thread */
// This allows payloads to modifie screen states without the need to include the full screen init binary
// screen init code by darksamus, AuroraWright and some others 
// got code for disabeling from CakesForeveryWan


#include "constants.h"
#include "common.h"

volatile u32 *a11_entry = (volatile u32 *)0x1FFFFFF8;
volatile a11Commands* arm11_commands=(volatile a11Commands*)ARM11COMMAND_ADDRESS;

static inline void enable_lcd();
static inline void disable_lcds(); 
static inline void a11setBrightness();

void __attribute__((naked)) a11Entry()
{
    __asm__ (
        "CPSID aif\n\t" //Disable interrupts
        "ldr r0,=_stack\n\t"
        "mov sp, r0"
    );

    /* Initialize the arm11 thread commands */
    *a11_entry = 0;
    a11Commands* arm11commands=(a11Commands*)ARM11COMMAND_ADDRESS;
    arm11commands->version=1;
    arm11commands->setBrightness=-1;
    arm11commands->brightness=DEFAULT_BRIGHTNESS;
    arm11commands->enableLCD=-1;
    arm11commands->fbTopLeft=FB_TOP_LEFT;
    arm11commands->fbTopRigth=FB_TOP_RIGHT;
    arm11commands->fbBottom=FB_BOTTOM;
    arm11commands->a11ControllValue=0xDEADBEEF;

    /* Wait for arm11 entry and commands */
    while (!*a11_entry)
    {
        /* Check if the command buffer got overwritten */
        if(arm11commands->a11ControllValue==0xDEADBEEF)
        {
            /* Signalize the thread is alive */
            arm11commands->a11threadRunning=1;
            if(arm11commands->setBrightness!=-1)
            {
                a11setBrightness();
                arm11commands->setBrightness=-1;
            }
            if(arm11commands->enableLCD!=-1)
            {
                if(arm11commands->enableLCD==0)
                    disable_lcds();
                if(arm11commands->enableLCD==1)
                    enable_lcd();
                arm11commands->enableLCD=-1;
            }
        }
    }

    /* Signalize the bg thread stops and jumps to an a11 entry */
    arm11commands->a11ControllValue=0;
    arm11commands->a11threadRunning=0;
    ((void (*)())*a11_entry)();  
}

static inline void disable_lcds()  
{  
    /* Disable screen, for example to launch firm */
	*(volatile u32 *)0x10202A44 = 0;  
	*(volatile u32 *)0x10202244 = 0;  
	*(volatile u32 *)0x1020200C = 0;  
	*(volatile u32 *)0x10202014 = 0;  
} 

static inline void enable_lcd()
{
    /* Cleare the frame buffers to be completely sure to not display anything unwanted */
    for(int i = 0; i < (SCREEN_SIZE); i++)
    {
        *((unsigned int*)FB_TOP_LEFT + i) = 0;
        *((unsigned int*)FB_TOP_RIGHT + i) = 0;
        *((unsigned int*)FB_BOTTOM + i) = 0;
    }

    /* Starts Screen initialisation */
    *(volatile u32*)0x10141200 = 0x1007F;
    *(volatile u32*)0x10202014 = 0x00000001;
    *(volatile u32*)0x1020200C &= 0xFFFEFFFE;

    /* set brightnes from config field */
    *(volatile u32*)0x10202240 = ((a11Commands*)ARM11COMMAND_ADDRESS)->brightness;
    *(volatile u32*)0x10202A40 = ((a11Commands*)ARM11COMMAND_ADDRESS)->brightness;
    *(volatile u32*)0x10202244 = 0x1023E;
    *(volatile u32*)0x10202A44 = 0x1023E;

    /* Top screen */
    *(volatile u32*)0x10400400 = 0x000001c2;
    *(volatile u32*)0x10400404 = 0x000000d1;
    *(volatile u32*)0x10400408 = 0x000001c1;
    *(volatile u32*)0x1040040c = 0x000001c1;
    *(volatile u32*)0x10400410 = 0x00000000;
    *(volatile u32*)0x10400414 = 0x000000cf;
    *(volatile u32*)0x10400418 = 0x000000d1;
    *(volatile u32*)0x1040041c = 0x01c501c1;
    *(volatile u32*)0x10400420 = 0x00010000;
    *(volatile u32*)0x10400424 = 0x0000019d;
    *(volatile u32*)0x10400428 = 0x00000002;
    *(volatile u32*)0x1040042c = 0x00000192;
    *(volatile u32*)0x10400430 = 0x00000192;
    *(volatile u32*)0x10400434 = 0x00000192;
    *(volatile u32*)0x10400438 = 0x00000001;
    *(volatile u32*)0x1040043c = 0x00000002;
    *(volatile u32*)0x10400440 = 0x01960192;
    *(volatile u32*)0x10400444 = 0x00000000;
    *(volatile u32*)0x10400448 = 0x00000000;
    *(volatile u32*)0x1040045C = 0x00f00190;
    *(volatile u32*)0x10400460 = 0x01c100d1;
    *(volatile u32*)0x10400464 = 0x01920002;
    *(volatile u32*)0x10400468 = 0x18300000;
    *(volatile u32*)0x10400470 = 0x80341;
    *(volatile u32*)0x10400474 = 0x00010501;
    *(volatile u32*)0x10400478 = 0;
    *(volatile u32*)0x10400490 = 0x000002D0;
    *(volatile u32*)0x1040049C = 0x00000000;

    // Disco register
    for(volatile u32 i = 0; i < 256; i++)
        *(volatile u32*)0x10400484 = 0x10101 * i;

    /* Bottom screen */
    *(volatile u32*)0x10400500 = 0x000001c2;
    *(volatile u32*)0x10400504 = 0x000000d1;
    *(volatile u32*)0x10400508 = 0x000001c1;
    *(volatile u32*)0x1040050c = 0x000001c1;
    *(volatile u32*)0x10400510 = 0x000000cd;
    *(volatile u32*)0x10400514 = 0x000000cf;
    *(volatile u32*)0x10400518 = 0x000000d1;
    *(volatile u32*)0x1040051c = 0x01c501c1;
    *(volatile u32*)0x10400520 = 0x00010000;
    *(volatile u32*)0x10400524 = 0x0000019d;
    *(volatile u32*)0x10400528 = 0x00000052;
    *(volatile u32*)0x1040052c = 0x00000192;
    *(volatile u32*)0x10400530 = 0x00000192;
    *(volatile u32*)0x10400534 = 0x0000004f;
    *(volatile u32*)0x10400538 = 0x00000050;
    *(volatile u32*)0x1040053c = 0x00000052;
    *(volatile u32*)0x10400540 = 0x01980194;
    *(volatile u32*)0x10400544 = 0x00000000;
    *(volatile u32*)0x10400548 = 0x00000011;
    *(volatile u32*)0x1040055C = 0x00f00140;
    *(volatile u32*)0x10400560 = 0x01c100d1;
    *(volatile u32*)0x10400564 = 0x01920052;
    *(volatile u32*)0x10400568 = 0x18300000 + 0x46500;
    *(volatile u32*)0x10400570 = 0x80301;
    *(volatile u32*)0x10400574 = 0x00010501;
    *(volatile u32*)0x10400578 = 0;
    *(volatile u32*)0x10400590 = 0x000002D0;
    *(volatile u32*)0x1040059C = 0x00000000;

    // Disco register
    for(volatile u32 i = 0; i < 256; i++)
        *(volatile u32*)0x10400584 = 0x10101 * i;

    /* Set framebuffers */
    *(volatile u32*)0x10400468 = FB_TOP_LEFT;
    *(volatile u32*)0x1040046c = FB_TOP_LEFT;
    *(volatile u32*)0x10400494 = FB_TOP_RIGHT;
    *(volatile u32*)0x10400498 = FB_TOP_RIGHT;
    *(volatile u32*)0x10400568 = FB_BOTTOM;
    *(volatile u32*)0x1040056c = FB_BOTTOM;
    //(volatile u32*)0x10202000 = 0xffFFFFFF; needed for 3d?
}

static inline void a11setBrightness()
{
    if(*(u8*)0x10141200 != 0x1)
    {
        *((volatile u32*)0x10202240) = ((a11Commands*)ARM11COMMAND_ADDRESS)->brightness;           
        *((volatile u32*)0x10202A40) = ((a11Commands*)ARM11COMMAND_ADDRESS)->brightness;
    }
} 

