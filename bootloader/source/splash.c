#include "splash.h"
#include "draw.h"
#include "log.h"
#include "helpers.h"
#include "sdmmc.h"
#include "ff.h"
#include "constants.h"
#include "hid.h"

bool drawBootSplash(loaderConfiguration* loaderConfig)
{
    if(!isColdboot()&&loaderConfig->enableSoftbootSplash==0)
        return false;

    if(loaderConfig->bootsplash>0)
    {
        switch(loaderConfig->bootsplash)
        {/* 0 - disabled, 1 - splash screen, 2 - entry info, 3 - both */ 
            case 1:
            case 3:
                if(splash_image(loaderConfig->bootsplash_image)!=0)
                    splash_ascii(NULL);
                break;
            case 4:
                if(splash_anim(loaderConfig->bootsplash_image)!=0)
                    splash_ascii(NULL);
                break;
            case 2:
                splash_ascii(NULL);
                break;
        }
        return true;
    }
    else
        return false;
}

bool drawSplash(configuration* app)
{
    if(!isColdboot()&&app->enableSoftbootSplash==0)
        return false;
    
    if(app->splash>0)
    {
        switch(app->splash)
        {/* 0 - disabled, 1 - splash screen, 2 - entry info, 3 - both */ 
            case 1:
                if(splash_image(app->splash_image)!=0)
                    splash_ascii(NULL);
                break;
            case 2:
                splash_ascii(app->path);
                break;
            case 3:
                if(splash_image(app->splash_image)!=0)
                    splash_ascii(app->path);
                break;
            case 4:
                if(splash_anim(app->splash_image)!=0)
                    splash_ascii(app->path);
                break;
        }
        if(app->splash!=4)
        {
            debug("Showing Splash for %llu ms",app->splashDelay);
            for(volatile u64 i=0;i<0xEFF*app->splashDelay;i++);
        }
        return true;
    }
    else
        return false;
}

int splash_ascii(const char* payloadName)
{
    // print BootCtr9 logo
    // http://patorjk.com/software/taag/#p=display&f=Bigfig&t=BootCtr
    ClearScreen(TOP_SCREENL,0);
    ClearScreen(TOP_SCREENR,0);
    if(payloadName!=NULL)
    {
        if(strlen(payloadName)>1)
        {
            DrawStringF(5,5,ASCII_ART_TEMPLATE_EXTENDET, VERSION_STRING, payloadName);
            return 0;
        }
    }
    DrawStringF(5,5,ASCII_ART_TEMPLATE, VERSION_STRING);
    return 0;
}

int splash_image(char *splash_path)
{
    // load image in memory, doing proper error checking
    FIL splash_file;
    unsigned int br;
    if(strlen(splash_path)==0)
    {
        debug("Splash image not set, use default screen");
        return -1;
    }
    if(f_open(&splash_file, splash_path, FA_READ | FA_OPEN_EXISTING) != FR_OK)
    {
        debug("Couldn't open splash image %s.", splash_path);
        return -1;
    }

    //load splash to templocation in memory to prevent visible drawing
    f_read(&splash_file, (void*)(TMPSPLASHADDRESS), 0x00600000, &br);
    // copy splash image to framebuffers(in case 3d is enabled)
    memcpy((void*)TOP_SCREENL,(void*)TMPSPLASHADDRESS,br);
    memcpy((void*)TOP_SCREENR,(void*)TMPSPLASHADDRESS,br);

    return 0;    
}

void delay(u32 n) {
    u32 i = n;
    while (i--)
        __asm("mov r0, #0"); // Just a little delay
}

int splash_anim(char *splash_path)
{
    u8 rate = 0x0F; // Default, overridden by config (15 in decimal, for those of you who can't hex)
    u32 topAnimSize = 0, configSize = 0; // Define filesizes, 0 by default

    if(strlen(splash_path)==0)
    {
        debug("Splash image not set, use default screen");
        return -1;
    }

    char config_fname[64];
    sprintf(config_fname,"%s.cfg",splash_path);
    //char *bottom_fname = "/anim/0/bottom_anim"; //NoBottomSplash atm in bootctr9

    
    configSize      = getFileSize(config_fname); // Get config file size (mostly to check whether it exists, because rn it only reads 1 byte)
    topAnimSize     = getFileSize(splash_path);    // Get top screen animation size
    //bottomAnimSize  = fileSize(bottom_fname); // NoBottomSplash atm in bootctr9 - Get bottom screen animation size()

    if (configSize) {
        FIL config_fil;
        unsigned int br;
        f_open(&config_fil, config_fname, FA_READ);
        f_read(&config_fil, &rate, 1, &br);
        f_close(&config_fil);
    }

    animationLoop(splash_path, topAnimSize, rate); // Main animation loop, with filenames and amount of frames
    return 0;
}

void animationLoop(char *top_anim,u32 topAnimSize, u8 frameRate) 
{
    // Only reason I have the framerate as an extra parameter is due to portability
    // Provided fatfs and framebuffers are available, this is pretty portable fwiw
    // Except the delay part ofc

    u32 topFrames=topAnimSize / TOP_FB_SIZE;

    FIL bgr_anim_bot, bgr_anim_top;
    unsigned int put_bot, put_top; // Because FatFS

    ClearScreen(TOP_SCREENL,0);
    ClearScreen(TOP_SCREENR,0);

    if (topFrames > 0) { // If top animation exists
        f_open(&bgr_anim_top, top_anim, FA_READ);
        put_top = 0;
    }

    //NoBottomSplash atm in bootctr9
    /*if (bottomFrames > 0) { // If bottom animation exists
        f_open(&bgr_anim_bot, bottom_anim, FA_READ);
        put_bot = 0;
    }*/

    //u32 maxFrames = max(topFrames, bottomFrames); // Get the maximum amount of frames between the two animations
    u32 maxFrames = topFrames;
    u32 delay_ = 0; // Set delay to 0, for those using high framerate animations

    if (frameRate <= 24 && frameRate > 0) // If animation is (relatively) low fps...
        delay_ = (6990480 / frameRate); // Need to take more accurate measurements, but this will do, it's quite a magic number

    u32 delay__ = (delay_ / 2); // FIXME - THIS IS NOT OKAY. Hey, it's just a bad approximation, M'kay?
    //debug("maxframes: %u\n topFrames: %u\n TOP_FB_SIZE: %u",maxFrames,topFrames,TOP_FB_SIZE);

    for (u32 curframe = 0; curframe < maxFrames; curframe++) 
    {   
        if (GetInput() == (KEY_SELECT|KEY_START)) // End the animation if the 'SELECT' key is pressed
            curframe = maxFrames;

        if (curframe <= topFrames) { // If top animation hasn't ended yet
            f_read(&bgr_anim_top, (void*)TOP_SCREENL, TOP_FB_SIZE, &put_top); // Write to the framebuffer directly

           /* if (curframe <= bottomFrames)
                delay(delay__); // Half the delay
            else*/
                delay(delay_); // Whole delay
        }

        //NoBottomSplash atm in bootctr9
        /*if (curframe < bottomFrames) { // If bottom animation hasn't ended yet
            f_read(&bgr_anim_bot, framebuffers->bottom, BOTTOM_FB_SZ, &put_bot); // Write to the framebuffer directly

            if (curframe <= topFrames) // check whether the top animation is playing
                delay(delay__); // Half the delay
            else
                delay(delay_); // Whole delay
        }*/

        // THIS HAS BEEN PARTIALLY CALIBRATED
    }

    /*if (bottomFrames) // If bottom animation's FIL was opened, close
        f_close(&bgr_anim_bot);*/

    if (topFrames) // If top animation's FIL was opened, close
        f_close(&bgr_anim_top);
    
    return;
}
