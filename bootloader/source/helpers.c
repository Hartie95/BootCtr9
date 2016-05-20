#include "helpers.h"
#include "log.h"
#include "constants.h"
#include "screen.h"
#include "draw.h"

/* File Helpers */

char workingDir[64]={0};

bool file_exists(const char* path) { 
    FIL fd;
    if(f_open(&fd, path, FA_READ | FA_OPEN_EXISTING) == FR_OK) { 
       f_close(&fd); 
       return true; 
    } 
 	return false; 
} 

u32 getFileSize(const char *path){
    FIL fp;
    u32 size = 0;

    if(f_open(&fp, path, FA_READ) == FR_OK)
        size = f_size(&fp); 

    f_close(&fp);
    return size;
}


void getWorkingDirectory(char* str)
{
	if(workingDir[0]==0)
	{
		debug("Checking for target workingDir");
		char* supportedFolders[]={SUPPORTEDFOLDERS};
		char targetPath[64]={0};
		u32 numberOfFolders=sizeof(supportedFolders)/sizeof(char*);
		for(int i=0;i<numberOfFolders;i++)
		{
			strcpy(targetPath, supportedFolders[i]);
			strcat(targetPath, INI_FILE);
			if(file_exists(targetPath))
			{
				strcpy(workingDir,supportedFolders[i]);
				break;
			}
		}
		if(workingDir[0]==0)
			strcpy(workingDir,"/");
		debug("Working directory set to: %s",workingDir);
	}
	strcpy(str,workingDir);
}

void checkFolders(const char* fileName, char* str)
{
 	getWorkingDirectory(str);
 	strcat(str,fileName);
	return;
}


/* Screen Helpers */

void setScreenState(bool enableScreen)
{
	if(enableScreen)
		initScreen();
	else
		shutdownScreen();
}

void initScreen()
{
	if(screenInit())
	{
		clearScreens();
		debug("Screens enabled");
	}
}

void shutdownScreen()
{
	debug("shutting down Screens");
	screenShutdown();
}

void setBrightness(u8 brightness)
{
	changeBrightness(brightness);
}


/* Config Helpers */

bool openIniFile(FIL* file)
{
    char filename[64]={0};
    checkFolders(INI_FILE,filename);
	if(file_exists(filename))
    {
    	if (f_open(file, filename, FA_READ | FA_OPEN_EXISTING) == FR_OK)
        	return true;
    }
    return false;
}

int iniparse(ini_handler handler, void* user, FIL* file)
{
    int error;
    f_rewind(file);
    error = ini_parse_stream((ini_reader)f_gets, file, handler, user);
    return error;
}

/* other Helpers */

bool isColdboot()
{
	if(*(u8*)CFG_BOOTENV == COLDBOOT)
		return true;
	return false;
}
