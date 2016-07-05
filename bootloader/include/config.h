#pragma once

#include "common.h"

#include "ini.h"

#include "ff.h"

typedef struct {
    char* section;
    unsigned long long keyDelay;
    int bootsplash; 
    char bootsplash_image[64];
    bool enableSoftbootSplash;
    bool enableAutosoftboot;
    bool enableArm9ComapnionBoot;
    char bootPassword[100];
    bool fileLog;
    bool screenLog;
    unsigned int screenEnabled;
    unsigned int screenBrightness;
} loaderConfiguration;

typedef struct {
    char* section;
    char path[64];
    unsigned long long splashDelay;
    unsigned long payload;
    unsigned long offset; 
	int splash; 
	char splash_image[64];
    bool enableSoftbootSplash;
    unsigned int screenEnabled;
    unsigned int screenBrightness;
    unsigned int fixArm9Path;
} configuration;



#define MATCH(s, n) strcmp(section, s) == 0 && strcmp(name, n) == 0
int handler(void *user, const char *section, const char *name, const char *value);
int handlerLoaderConfiguration(void *user, const char *section, const char *name, const char *value);
int readPayloadSection(configuration* app, FIL* configFile);