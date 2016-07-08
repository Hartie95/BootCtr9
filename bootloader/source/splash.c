#include "splash.h"
#include "draw.h"
#include "log.h"
#include "helpers.h"
#include "sdmmc.h"
#include "ff.h"
#include "constants.h"
#include "hid.h"
#include "timer.h"
#include "quicklz.h"
#include "convert.h"

#include "screen.h"
#include "../../arm11bg/source/arm11bg/constants.h"

// Define A11 Commands Struct
ARM11_COMMANDS

bool drawBootSplash(loaderConfiguration* loaderConfig)
{
    if(!isColdboot()&&!loaderConfig->enableSoftbootSplash)
        return false;

    if(loaderConfig->bootsplash)
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

        debug("Splash sucessfully loaded");
        return true;
    }
    else
        return false;
}

bool drawSplash(configuration* app)
{
    if(!isColdboot()&&!app->enableSoftbootSplash)
        return false;
    
    if(app->splash)
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
    if(!strlen(splash_path))
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

int splash_anim(char *splash_path)
{
    u8 frameRate= 0x0F;
    u32 topAnimationFileSize=0;
    u32 frameRateConfigurationFileSize=0;
    char frameRateConfigurationPath[64]={0};

    u8 isCompressionEnabled = false;
    char* currentFrame = (char*) TMPSPLASHADDRESS;
    char* lastFrame = (char*) TMPSPLASHADDRESS2;
    char* tmpbufferCompressed = (char*) TMPSPLASHADDRESS3;

    memset(lastFrame, 0, TOP_FB_SIZE); // Clear all buffers  
    memset(currentFrame, 0, TOP_FB_SIZE);  
    memset(tmpbufferCompressed, 0, TOP_FB_SIZE + 600);  


    if(!strlen(splash_path))
    {
        debug("Splash image not set, using default screen instead");
        return -1;
    }

    topAnimationFileSize=getFileSize(splash_path);
    if(topAnimationFileSize < TOP_FB_SIZE)
    {
        debug("Bootanimation includes less than one Frame\n, using default screen instead");
        return -1;
    }

    FIL FrameRateConfigurationFile;
    FIL topScreenAnimationFile;
    unsigned int br;

    qlz_state_decompress *state_decompress = (qlz_state_decompress*)0x24400000; 
    memset(state_decompress, 0, sizeof(qlz_state_decompress));  
    
    size_t comp_size = 0;  

    // trying to use a plaintext .cfg configuration
    sprintf(frameRateConfigurationPath,"%s.cfg",splash_path);
    frameRateConfigurationFileSize=getFileSize(frameRateConfigurationPath);
    if(frameRateConfigurationFileSize)
    {
        f_open(&FrameRateConfigurationFile, frameRateConfigurationPath, FA_READ);
        char framerateString[3]={0};
        char animationCompressionString[4]={0};
        f_read(&FrameRateConfigurationFile, &framerateString, 2, &br);
        frameRate=myAtoi(framerateString);

        if(frameRateConfigurationFileSize>2)
        {
            f_read(&FrameRateConfigurationFile, &animationCompressionString, 3, &br);

            if(br==3&&!strcmp("lzd", animationCompressionString))
            {
                isCompressionEnabled=true;
            }
        }

        f_close(&FrameRateConfigurationFile);

        if(!frameRate)
            frameRate=0x0F;
    }
    else
    {   
        // Trying to use a binary .cfgb configuration
        sprintf(frameRateConfigurationPath,"%s.cfgb",splash_path);
        frameRateConfigurationFileSize=getFileSize(frameRateConfigurationPath);
        if(frameRateConfigurationFileSize)
        {
            f_open(&FrameRateConfigurationFile, frameRateConfigurationPath, FA_READ);
            f_read(&FrameRateConfigurationFile, &frameRate, 1, &br);

            if(frameRateConfigurationFileSize>1)
                f_read(&FrameRateConfigurationFile, &isCompressionEnabled, 1, &br);

            f_close(&FrameRateConfigurationFile);

            if(!frameRate)
                frameRate=0x0F;
        }
    }
    
    f_open(&topScreenAnimationFile, splash_path, FA_READ);

    ClearScreen(TOP_SCREENL,0);
    ClearScreen(TOP_SCREENR,0);

    //set arm11 thread to draw mode to let it write the readed Framebuffers
    setMode(MODE_DRAW);
    a11Commands* arm11_commands=(a11Commands*)ARM11COMMAND_ADDRESS;

    // calculatet number of Timer cicles per second using the 1024 Prescaler
    // and divide it throug framerate to get the timer Value for the next frame
    u32 nextFrameTimerValue=TIMERFREQUENCY/1024/frameRate;
    vu16* timerValue=timerGetValueAddress(0);

    *timerValue=0;
    //starts timer and let it use the 1024 prescaler and the count up
    //more informations about the timer: https://www.3dbrew.org/wiki/TIMER_Registers#TIMER_CNT
    timerStart(0,PRESCALER_1024);
    u32 frame = 1;
    while(1) 
    {   
        if (GetInput() == (KEY_SELECT|KEY_START))
            break;

        if(isCompressionEnabled)
        {
            if(frame%2)
            {
                currentFrame = (char*) TMPSPLASHADDRESS;
                lastFrame = (char*) TMPSPLASHADDRESS2;
            }
            else
            {
                currentFrame = (char*) TMPSPLASHADDRESS2;
                lastFrame = (char*) TMPSPLASHADDRESS;
            }
            
            if (f_read(&topScreenAnimationFile, (void*)tmpbufferCompressed, 9, &br) != FR_OK)  
                break; 
            if(br!=9)
                break;


            comp_size = qlz_size_compressed(tmpbufferCompressed);  
   
            f_read(&topScreenAnimationFile, (void*)(tmpbufferCompressed + 9), comp_size - 9, &br);  
   
            if (qlz_size_decompressed(tmpbufferCompressed) != TOP_FB_SIZE || br != (comp_size - 9))  
                 break;  
   
            // Decompress the frame  
            qlz_decompress(tmpbufferCompressed, currentFrame, state_decompress);  
   
            // Delta decoding  
            for (u32 i = 0; i < TOP_FB_SIZE; i++)  
                currentFrame[i] += lastFrame[i];    

        }
        else
        {
            //Read to temporary buffer , wait for the next frame and let arm11 write it to the Frame buffer to minimize Tearing
            f_read(&topScreenAnimationFile, (void*)currentFrame, TOP_FB_SIZE, &br);
            if (br < TOP_FB_SIZE) // If it couldn't read the entire frame... 
                break;

        }
        while(*timerValue<nextFrameTimerValue);
        *timerValue=0;
        
        arm11_commands->fbTopLeft = (u32) currentFrame;
        frame++;

    }
    timerStop(0);

    f_close(&topScreenAnimationFile);

    //set arm11 mode back to the main mode
    setMode(MODE_MAIN);

    return 0;
}
