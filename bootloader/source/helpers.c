#include "helpers.h"
#include "log.h"
#include "constants.h"
#include "screen.h"
#include "draw.h"
#include "hid.h"

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
	if(!workingDir[0])
	{
		debug("Checking for target workingDir");
		char* supportedFolders[]={SUPPORTED_FOLDERS};
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

bool checkPassword(char* bootPassword)
{
	int passwordSize=strlen(bootPassword);
	
	//Check if a password is set
	if(!passwordSize)
		return true;

	while(GetInput()!=0);
	

	info("Please insert your password to load this payload");
	char currentKey[10]={0};
	char currentCharacter=0;
	unsigned int currentPosition=0;
	unsigned int currentKeyPosition=0;
	do
	{
		currentCharacter=bootPassword[currentPosition];
		if(currentCharacter!=' '&&currentCharacter)
		{
			currentKey[currentKeyPosition]=currentCharacter;
			currentKeyPosition++;
		}
		else
		{
			currentKey[currentPosition]='\0';
			u32 key = InputWait();
			while(GetInput()!=0);

		    // using X-macros to generate each switch-case rules
		    // https://en.wikibooks.org/wiki/C_Programming/Preprocessor#X-Macros
		    #define KEY(k) \
		    if(!strcmp(currentKey, "KEY_"#k)) \
		    { \
		        if( key & KEY_##k) \
		        {	\
		        	info("please press the next key"); \
		        } \
		        else \
		        { \
		        	info("You pressed the wrong key %s , will load [DEFAULT] instead.","KEY_"#k); \
		        	return false; \
		        } \
		    } \
		    else
		    #include "keys.def"
		    {
		    	debug("%s is not a valid key",currentKey);
		    	return false;	
		    }

			currentKeyPosition=0;
		}
		currentPosition++;
	}while(currentCharacter);
	return true;
}

