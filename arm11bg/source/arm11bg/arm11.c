/*
* Arm11 Background Thread by hartie95/hartmannaf
* screen init code by darksamus, AuroraWright and some others
* got code for disabeling from CakesForeveryWan
*/

#include "constants.h"
#include "common.h"

// Define A11 Commands Struct
ARM11_COMMANDS

volatile u32 *a11_entry = (volatile u32 *)0x1FFFFFF8;
volatile u32 *a11_entry_firm = (volatile u32 *)0x1FFFFFFC;
volatile u32 *loaderIdentifier = (volatile u32 *)ARM11LOADER_IDENTIFIER_ADDRESS;
static volatile a11Commands* arm11commands=(volatile a11Commands*)ARM11COMMAND_ADDRESS;

static inline void changeMode();
static void mainMode();
static inline void drawMode();
static inline void copyMode();
static inline void enable_lcd();
static inline void disable_lcds(); 
static inline void a11setBrightness();

static inline void setCurrentFramebufferAdresses()
{
    arm11commands->fbTopSelectedBuffer=*(vu32*)FB_SELECTED_TOP&0x1;
    arm11commands->fbTopLeft=*(u32*)0x10400468;
    arm11commands->fbTopLeft2=*(u32*)0x1040046C;

    arm11commands->fbTopRigth=*(u32*)0x10400494;
    arm11commands->fbTopRigth2=*(u32*)0x10400498;

    arm11commands->fbBottomSelectedBuffer=*(vu32*)FB_SELECTED_BOT&0x1;
    arm11commands->fbBottom=*(u32*)0x10400568;
    arm11commands->fbBottom2=*(u32*)0x1040056C;

    if(*(vu32*)FB_SELECTED_TOP&0x1)
    {
        TOP_SCREENL_CAKE=*(u32*)0x1040046C;
        TOP_SCREENR_CAKE=*(u32*)0x10400498;
    }
    else
    {
        TOP_SCREENL_CAKE=*(u32*)0x10400468;
        TOP_SCREENR_CAKE=*(u32*)0x10400494;
    }

    if(*(vu32*)FB_SELECTED_BOT&0x1)
        BOT_SCREEN_CAKE=*(u32*)0x1040056C;
    else
        BOT_SCREEN_CAKE=*(u32*)0x10400568;
}

static inline void setDefaultFramebufferAdresses()
{
    arm11commands->fbTopSelectedBuffer=0;
    arm11commands->fbTopLeft=FB_TOP_LEFT;
    arm11commands->fbTopLeft2=FB_TOP_LEFT2;

    arm11commands->fbTopRigth=FB_TOP_RIGHT;
    arm11commands->fbTopRigth2=FB_TOP_RIGHT2;

    arm11commands->fbBottomSelectedBuffer=0;
    arm11commands->fbBottom=FB_BOTTOM;
    arm11commands->fbBottom2=FB_BOTTOM2;
}

void __attribute__((naked)) a11Entry()
{
    __asm__ (
        "CPSID aif\n\t" //Disable interrupts
        "ldr r0,=_stack\n\t"
        "mov sp, r0"
    );

    /* Initialize the arm11 thread commands */
    *a11_entry = 0;
    *a11_entry_firm = 0;

    /* if screen init was already done, get the addresses from memory */
    if (*(u8*)0x10141200 != 0x1 && *loaderIdentifier!=ARM9LOADERHAX_IDENTIFIER || *(u8*)0x10141200&0x1 != 0x1 && *loaderIdentifier==ARM9LOADERHAX_IDENTIFIER)
    {
        setCurrentFramebufferAdresses();

    }
    else
    {
        setDefaultFramebufferAdresses();
    }

    arm11commands->a11ControllValue=0xDEADBEEF;

    mainMode();
     
}

static void mainMode()
{
    arm11commands->setBrightness=ARM11_DONE;
    arm11commands->brightness=DEFAULT_BRIGHTNESS;

    arm11commands->enableLCD=ARM11_DONE;

    arm11commands->changeMode=ARM11_DONE;
    arm11commands->mode=MODE_MAIN;

    /* Wait for arm11 entry and commands */
    while (!*a11_entry&& !*a11_entry_firm)
    {
        /* Check if the command buffer got overwritten */
        if(arm11commands->a11ControllValue==0xDEADBEEF)
        {
            /* Signalize the thread is alive */
            arm11commands->a11threadRunning=1;
            if (*(u8*)0x10141200 != 0x1)
            {
                if((*(vu32*)FB_SELECTED_TOP&0x1)^(arm11commands->fbTopSelectedBuffer&0x1))
                {
                    if(arm11commands->fbTopSelectedBuffer&0x1)
                    {
                        TOP_SCREENL_CAKE=*(u32*)0x1040046C;
                        TOP_SCREENR_CAKE=*(u32*)0x10400498;
                    }
                    else
                    {
                        TOP_SCREENL_CAKE=*(u32*)0x10400468;
                        TOP_SCREENR_CAKE=*(u32*)0x10400494;
                    }
                    
                    *(vu32*)FB_SELECTED_TOP=arm11commands->fbTopSelectedBuffer&0x1;

                }
                if((*(vu32*)FB_SELECTED_BOT&0x1)^(arm11commands->fbBottomSelectedBuffer&0x1))
                {

                    if(arm11commands->fbBottomSelectedBuffer&0x1)
                        BOT_SCREEN_CAKE=*(u32*)0x1040056C;
                    else
                        BOT_SCREEN_CAKE=*(u32*)0x10400568;

                    *(vu32*)FB_SELECTED_BOT=arm11commands->fbBottomSelectedBuffer&0x1;
                }
            }
            if(arm11commands->setBrightness)
            {
                a11setBrightness();
                arm11commands->setBrightness=ARM11_DONE;
            }
            if(arm11commands->enableLCD)
            {
                u32 lcdComand = arm11commands->enableLCD;
                if(lcdComand==DISABLE_SCREEN)
                    disable_lcds();
                if(lcdComand==ENABLE_SCREEN)
                    enable_lcd();

                arm11commands->enableLCD=ARM11_DONE;
            }
            if(arm11commands->changeMode)
            {
                changeMode();
            }
        }
    }

    /* Signalize the bg thread stops and jumps to an a11 entry */
    arm11commands->a11ControllValue=0;
    arm11commands->a11threadRunning=0;
    if(*a11_entry_firm)
        ((void (*)())*a11_entry_firm)();
    else
        ((void (*)())*a11_entry)();
}

static void changeMode()
{
        switch(arm11commands->changeMode)
        {
            case MODE_DRAW:
                    drawMode();
                    break;
            case MODE_COPY:
                    copyMode();
                    break;
            default:
                    a11Entry();
                    break;

        }
}

/* should be faster, because of the usage of u32 instead of u8 on a 32 bit prozessor*/ 
static inline void fastmemcpy(vu32* src, vu32* target, u32 length)
{
    length/=4;
    while(length--)
    {
        *target=*src;
        target++;
        src++;
    }
}

static inline void drawMode()
{
    arm11commands->fbTopLeft=0;
    arm11commands->fbTopRigth=0;
    arm11commands->fbBottom=0;
    arm11commands->mode=MODE_DRAW;
    arm11commands->changeMode=ARM11_DONE;
    u32 curentTopLeftBuffer=*(u32*)0x10400468;
    u32 curentTopRightBuffer=*(u32*)0x10400494;
    u32 curentBottomBuffer=*(u32*)0x10400568;
    while(!arm11commands->changeMode)
    {
        if(arm11commands->fbTopLeft)
        {
            //Select which buffer should be used for the next picture, based on the current selected Buffer
            if(*(vu32*)FB_SELECTED_TOP)
                curentTopLeftBuffer=*(u32*)0x10400468;
            else
                curentTopLeftBuffer=*(u32*)0x1040046C;

            fastmemcpy((u32*)arm11commands->fbTopLeft,(u32*)curentTopLeftBuffer,SCREEN_SIZE);
            
            //change the selected Framebuffer
            *(vu32*)FB_SELECTED_TOP=(!*(vu32*)FB_SELECTED_TOP)&0x1;

            arm11commands->fbTopLeft=0;
        }
        if(arm11commands->fbTopRigth!=0)
        {
            if(*(vu32*)FB_SELECTED_TOP)
                curentTopRightBuffer=*(u32*)0x10400494;
            else
                curentTopRightBuffer=*(u32*)0x10400498;

            fastmemcpy((u32*)arm11commands->fbTopRigth,(u32*)curentTopRightBuffer,SCREEN_SIZE);
            arm11commands->fbTopRigth=0;
        }
        if(arm11commands->fbBottom)
        {
            //Select which buffer should be used for the next picture, based on the current selected Buffer
            if(*(vu32*)FB_SELECTED_BOT)
                curentBottomBuffer=*(u32*)0x10400568;
            else
                curentBottomBuffer=*(u32*)0x1040056C;

            fastmemcpy((u32*)arm11commands->fbBottom,(u32*)FB_BOTTOM,SCREEN_SIZE);

            //change the selected Framebuffer
            *(vu32*)FB_SELECTED_BOT=(!*(vu32*)FB_SELECTED_BOT)&0x1;
            arm11commands->fbBottom=0;
        }
    }
    *(vu32*)FB_SELECTED_TOP=0;
    *(vu32*)FB_SELECTED_BOT=0;
    //setCurrentFramebufferAdresses();
    changeMode();
}


/* should be faster, because of the usage of u32 instead of u8 on a 32 bit prozessor*/
static inline void fastmemcpyWithStatus(vu8* src, vu8* target, u32 length, vu32* status)
{
    u32 alignment= length%4;
    length/=4;
    while(length--)
    {
        *(vu32*)target=*(vu32*)src;
        target+=sizeof(u32);
        src+=sizeof(u32);
        if(status)
        {
            *status = length*4+alignment;
        }
    }
    while(alignment--){
        *target=*src;
        target++;
        src++;
        if(status)
        {
            *status = alignment;
        }
    }
}

static inline void copyMode()
{
    arm11commands->fbTopLeft=0;
    arm11commands->fbTopRigth=0;
    arm11commands->fbBottom=0;/*TODO start signal?*/
    arm11commands->mode=MODE_COPY;
    arm11commands->changeMode=ARM11_DONE;
    while(!arm11commands->changeMode)
    {
        if(arm11commands->fbTopLeft&&arm11commands->fbTopRigth&&arm11commands->fbBottom>0)
        {
            fastmemcpyWithStatus((vu8*)arm11commands->fbTopLeft,(vu8*)arm11commands->fbTopRigth,arm11commands->fbBottom,&arm11commands->fbBottom);
            arm11commands->fbTopLeft=0;
            arm11commands->fbTopRigth=0;
        }
    }
    //setCurrentFramebufferAdresses();
    changeMode();
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
    int i=SCREEN_SIZE;
    while(i--)
    {
        *((vu32*)FB_TOP_LEFT + i) = 0;
        *((vu32*)FB_TOP_RIGHT + i) = 0;
        *((vu32*)FB_BOTTOM + i) = 0;
        *((vu32*)FB_TOP_LEFT2 + i) = 0;
        *((vu32*)FB_TOP_RIGHT2 + i) = 0;
        *((vu32*)FB_BOTTOM2 + i) = 0;
    }

    
    arm11commands->fbTopSelectedBuffer=0;
    arm11commands->fbTopLeft=FB_TOP_LEFT;
    arm11commands->fbTopLeft2=FB_TOP_LEFT2;

    arm11commands->fbTopRigth=FB_TOP_RIGHT;
    arm11commands->fbTopRigth2=FB_TOP_RIGHT2;

    arm11commands->fbBottomSelectedBuffer=0;
    arm11commands->fbBottom=FB_BOTTOM;
    arm11commands->fbBottom2=FB_BOTTOM2;

    /* Starts Screen initialisation */
    *(volatile u32*)0x10141200 = 0x1007F;       // PDN_GPU_CNT 
    *(volatile u32*)0x10202014 = 0x00000001;    // LCD register?
    *(volatile u32*)0x1020200C &= 0xFFFEFFFE;   // LCD register?

    /* LCD register */
    /* Top screen */
    *(volatile u32*)0x10202240 = arm11commands->brightness;
    *(volatile u32*)0x10202244 = 0x1023E;
    /* Bottom screen */
    *(volatile u32*)0x10202A40 = arm11commands->brightness;
    *(volatile u32*)0x10202A44 = 0x1023E;

    /* GPU registers */
    /* Top screen framebuffer setup */
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
    *(volatile u32*)0x1040045C = 0x00f00190;    // Framebuffer width(16 bit) & height (16bit)
    *(volatile u32*)0x10400460 = 0x01c100d1;
    *(volatile u32*)0x10400464 = 0x01920002;
    *(volatile u32*)0x10400468 = FB_TOP_LEFT;   // Framebuffer top left first address
    *(volatile u32*)0x1040046C = FB_TOP_LEFT2;  // Framebuffer top left second address
    *(volatile u32*)0x10400470 = 0x80341;       // Framebuffer format 
    *(volatile u32*)0x10400474 = 0x00010501;
    *(volatile u32*)0x10400478 = 0;             // Framebuffer select 
    *(volatile u32*)0x10400490 = 0x000002D0;    // Framebuffer stride 
    *(volatile u32*)0x10400494 = FB_TOP_RIGHT;  // Framebuffer top right first address
    *(volatile u32*)0x10400498 = FB_TOP_RIGHT2; // Framebuffer top right second address
    *(volatile u32*)0x1040049C = 0x00000000;

    // Disco register
    for(volatile u32 i = 0; i < 256; i++)
        *(volatile u32*)0x10400484 = 0x10101 * i;

    /* Bottom screen framebuffer setup */
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
    *(volatile u32*)0x1040055C = 0x00f00140;    // Framebuffer width(16 bit) & height (16bit)
    *(volatile u32*)0x10400560 = 0x01c100d1;    // Framebuffer bottom first address
    *(volatile u32*)0x10400564 = 0x01920052;
    *(volatile u32*)0x10400568 = FB_BOTTOM;
    *(volatile u32*)0x1040056C = FB_BOTTOM2;    // Framebuffer bottom second address
    *(volatile u32*)0x10400570 = 0x80301;       // Framebuffer format 
    *(volatile u32*)0x10400574 = 0x00010501;
    *(volatile u32*)0x10400578 = 0;             // Framebuffer select
    *(volatile u32*)0x10400590 = 0x000002D0;    
    *(volatile u32*)0x1040059C = 0x00000000;

    // Disco register
    for(volatile u32 i = 0; i < 256; i++)
        *(volatile u32*)0x10400584 = 0x10101 * i;

    /* Set framebuffers */
    *(volatile u32*)0x10400468 = FB_TOP_LEFT;
    *(volatile u32*)0x1040046c = FB_TOP_LEFT2;
    *(volatile u32*)0x10400494 = FB_TOP_RIGHT;
    *(volatile u32*)0x10400498 = FB_TOP_RIGHT2;
    *(volatile u32*)0x10400568 = FB_BOTTOM;
    *(volatile u32*)0x1040056c = FB_BOTTOM2;
    /* LCD enable parallax*/
    //*(volatile u32*)0x10202000 = 0xAAAAAAAA; //needed for 3d?

    
}

static inline void a11setBrightness()
{
    if(*(u8*)0x10141200 != 0x1)
    {
        *((volatile u32*)0x10202240) = arm11commands->brightness;           
        *((volatile u32*)0x10202A40) = arm11commands->brightness;
    }
} 
