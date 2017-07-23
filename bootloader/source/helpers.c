#include "helpers.h"
#include "log.h"
#include "constants.h"
#include "screen.h"
#include "draw.h"
#include "hid.h"
#include "jsmn.h"
#include "aes.h"
#include "convert.h"

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

/* experimental area */

#define ITCM 0x01FF8000
#define CTR_UNITINFO 0x10010010
#define TWL_UNITINFO 0x10010014

/* not yet working */
void copyDeviceID(u32 deviceID){
	if(deviceID!=0){
		*(u32*)(ITCM+0x3804)=deviceID;
		debug("wrote deviceID %x\n result: %x", deviceID, *(u32*)(ITCM+0x3804));
	}
}

/* not yet working */
void loadCTCert(char* path){
	FIL CTCertFile;
	UINT br =0;

	if(!strlen(path)){
		return;
	}

	if(f_open(&CTCertFile, path, FA_READ | FA_OPEN_EXISTING) != FR_OK){
		info("unable to open CTCert file %s", path);
		return;
	}
	f_read(&CTCertFile, (void*)(ITCM+0x3818), 0x68, &br);
	f_close(&CTCertFile);
	debug("wrote CTCert %s, br: %i",path, br);

}

/* not yet working */
void setUnitInfo(){
	*(u32*)(CTR_UNITINFO)=1;
	debug("Unitinfo: %x",*(u32*)(CTR_UNITINFO));
}

/** reads a json containing keys in the following format
* {
*	[keyslotType][keyslot]: [key as hexstring]
* }
*
*/
void setupKeys(char* path)
{
	jsmn_parser p;
	jsmntok_t t[64];
	FIL keyFile;
	char jsonString[8096]={0};
	char key[16]={0};
	u32 i;
	u32 x;
	UINT br;

	if(!strlen(path)){
		return;
	}
    if(f_open(&keyFile, path, FA_READ | FA_OPEN_EXISTING) != FR_OK) {
       debug("Unable to open keys file");
       return;
    }


	f_read(&keyFile, (void*)jsonString, 0x1024, &br);
	f_close(&keyFile);

	jsmn_init(&p);

	u32  results = jsmn_parse(&p, jsonString, strlen(jsonString), t, sizeof(t)/sizeof(t[0]));
	if (results < 0) {
		debug("Failed to parse JSON: %d", results);
		return;
	}

	if (results < 1 || t[0].type != JSMN_OBJECT) {
		debug("Object expected");
		return;
	}

	for (i = 1; i < results; i+=2) {
		char keyType = *(jsonString+t[i].start);
		char* pEnd;
		int keyslot=strtol((char*)jsonString+t[i].start+1, &pEnd,16);
		if(keyslot<1)
		{
			continue;
		}

		for(x=0;x<16;x++)
		{
			char tmp[3]={0};
			sprintf((char*)&tmp, "%.2s", (char*)jsonString+t[i+1].start+x*2);
			key[x]=htoi(tmp);
		}

		switch(keyType)
		{
			case 'x':
			case 'X':
				setup_aeskeyX(keyslot,key);
				break;
			case 'y':
			case 'Y':
				setup_aeskeyY(keyslot,key);
				break;

			case 'n':
			case 'N':
				setup_aeskey(keyslot,key);
				break;
		}
	}
}
