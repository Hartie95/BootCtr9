#include "screen.h"
#include "i2c.h"
#include "constants.h"

#include "../../arm11bg/source/arm11bg/constants.h"

#define A11_STUB_LOC     0x1FFF4C80
extern u8 arm11stub_bin[];
extern u32 arm11stub_bin_size;

// Define A11 Commands Struct
ARM11_COMMANDS

//got code for disabeling from CakesForeveryWan
static volatile u32 *a11_entry = (volatile u32 *)0x1FFFFFF8;
static volatile u32 *a11_entry_firm = (volatile u32 *)0x1FFFFFFC;

bool forceScreenInit=false;

static a11Commands* arm11_commands=(a11Commands*)ARM11COMMAND_ADDRESS;

// arm11 temp function, run while changing the arm11 binary 
void __attribute__((naked)) arm11tmp()
{
    *a11_entry = 0;  // Don't wait for us  
    while (!*a11_entry);
    ((void (*)())*a11_entry)();  
}

void setMode(u32 mode)
{
    if(arm11_commands->mode!=mode)
    {
        arm11_commands->changeMode=mode;
        while(arm11_commands->changeMode);
    }

}

void changeBrightness(u32 _brightness)
{
    arm11_commands->brightness=_brightness;
    arm11_commands->setBrightness=1;
    while(arm11_commands->setBrightness);
}

extern u8 *top_screen, *top_screen2, *bottom_screen;
void selectedFramebuffers(u8 topBuffer, u8 bottomBuffer)
{
    arm11_commands->fbTopSelectedBuffer;
    arm11_commands->fbBottomSelectedBuffer;
    for(volatile unsigned int i = 0; i < 0xF; i++);
    
    //these are inside of arm9 only memory, so the arm9 payload needs to update this
    *(volatile u32*)0x80FFFD8 = arm11_commands->fbTopSelectedBuffer;    // framebuffer select top
    *(volatile u32*)0x80FFFDC = arm11_commands->fbBottomSelectedBuffer;    // framebuffer select bottom
    top_screen = (u8*)(*(u32*)0x23FFFE00);
    top_screen2 = (u8*)(*(u32*)0x23FFFE04); 
    bottom_screen = (u8*)(*(u32*)0x23FFFE08);
}

bool screenInit()
{
    bool initFramebuffers, initScreen;
    initFramebuffers=initScreen=false;
    //Check if it's a no-screen-init A9LH boot via PDN_GPU_CNT  
    if (*(u8*)0x10141200 == 0x1)
    {
        initFramebuffers=initScreen=true;
    } 
    else if(forceScreenInit)
    {
        initFramebuffers=true;
    }

    if(initScreen){
        arm11_commands->enableLCD=ENABLE_SCREEN;
        while(arm11_commands->enableLCD);
        i2cWriteRegister(3, 0x22, 0x2A); // 0x2A -> boot into firm with no backlight
    }
    if(initFramebuffers){

        /* Initialize framebuffer using addresses from the arm11 thread */
        
        *(volatile u32*)0x80FFFC0 = arm11_commands->fbTopLeft;    // framebuffer 1 top left
        *(volatile u32*)0x80FFFC4 = arm11_commands->fbTopLeft2;    // framebuffer 2 top left
        *(volatile u32*)0x80FFFC8 = arm11_commands->fbTopRigth;    // framebuffer 1 top right
        *(volatile u32*)0x80FFFCC = arm11_commands->fbTopRigth2;    // framebuffer 2 top right
        *(volatile u32*)0x80FFFD0 = arm11_commands->fbBottom;    // framebuffer 1 bottom
        *(volatile u32*)0x80FFFD4 = arm11_commands->fbBottom2;    // framebuffer 2 bottom
        *(volatile u32*)0x80FFFD8 = arm11_commands->fbTopSelectedBuffer;    // framebuffer select top
        *(volatile u32*)0x80FFFDC = arm11_commands->fbBottomSelectedBuffer;    // framebuffer select bottom

        u32 topLeft=arm11_commands->fbTopLeft;
        u32 topRigth=arm11_commands->fbTopRigth;
        u32 bottom=arm11_commands->fbBottom;

        //cakehax  
        *(volatile u32*)0x23FFFE00 = topLeft;
        *(volatile u32*)0x23FFFE04 = topRigth;
        *(volatile u32*)0x23FFFE08 = bottom;

        top_screen = (u8*)topLeft;
        top_screen2 = (u8*)topRigth; 
        bottom_screen = (u8*)bottom; 
    }
        
    return initScreen;
}

void screenShutdown()
{
    if(*(u8*)0x10141200 != 0x1)
    {
        arm11_commands->enableLCD=DISABLE_SCREEN;
        while(arm11_commands->enableLCD);
    }
}

void loadStub()
{
    *a11_entry=(u32)arm11tmp;
    *a11_entry_firm=(u32)arm11tmp;
    while(*a11_entry);
}

void a11copy(u32 source, u32 target, u32 size)
{
    setMode(MODE_COPY);
    arm11_commands->fbTopLeft=source;
    arm11_commands->fbTopRigth=target;
    arm11_commands->fbBottom=size; 
    while(arm11_commands->fbBottom)
    {
        if(size!=arm11_commands->fbBottom)
        {
            size=arm11_commands->fbBottom;
        }
    }
    size=arm11_commands->fbBottom;
    setMode(MODE_MAIN);
}
