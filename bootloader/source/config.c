#include "config.h"

#include "log.h"
#include "helpers.h"

#include "convert.h"

int handler(void *user, const char *section, const char *name, const char *value)
{
    configuration *pconfig = (configuration *) user;
    if (MATCH(pconfig->section, "path")) {
        strcpy (pconfig->path,value);
    } else if (MATCH(pconfig->section, "delay")) {
        pconfig->splashDelay = myAtoi(value);
    } else if (MATCH(pconfig->section, "payload")) {
        pconfig->payload = myAtoi(value);
    } else if (MATCH(pconfig->section, "offset")) {
        pconfig->offset = numberToInt(value);
    } else if (MATCH(pconfig->section, "splash")) { 
        pconfig->splash = myAtoi(value);
    } else if (MATCH(pconfig->section, "splash_image")) { 
        strcpy (pconfig->splash_image,value); 
    } else if (MATCH(pconfig->section, "enableSoftbootSplash")) {
        pconfig->enableSoftbootSplash = numberToInt(value);
    } else if (MATCH(pconfig->section, "screenEnabled")) {
        pconfig->screenEnabled = myAtoi(value);
    } else if (MATCH(pconfig->section, "screenBrightness")) {
        pconfig->screenBrightness = numberToInt(value);
    } else if (MATCH(pconfig->section, "enablePathFix")) {
        pconfig->fixArm9Path = numberToInt(value);
    }else if (MATCH(pconfig->section, "keysPath")) {
        strcpy (pconfig->keysPath,value);
    }

    return 1;
}

int handlerLoaderConfiguration(void *user, const char *section, const char *name, const char *value)
{
    loaderConfiguration *pconfig = (loaderConfiguration *) user;
    if (MATCH(pconfig->section, "key_delay")) {
        pconfig->keyDelay = myAtoi(value);
    } else if (MATCH(pconfig->section, "bootPassword")) {
        strcpy (pconfig->bootPassword,value);
    } else if (MATCH(pconfig->section, "boot_splash")) { 
        pconfig->bootsplash = myAtoi(value);
    } else if (MATCH(pconfig->section, "boot_splash_image")) { 
        strcpy (pconfig->bootsplash_image,value); 
    } else if (MATCH(pconfig->section, "enableSoftbootSplash")) {
        pconfig->enableSoftbootSplash = numberToInt(value);
    } else if (MATCH(pconfig->section, "enableAutosoftboot")) {
        pconfig->enableAutosoftboot = numberToInt(value);
    } else if (MATCH(pconfig->section, "enableArm9CompanionBoot")) {
        pconfig->enableArm9ComapnionBoot = numberToInt(value);
    } else if (MATCH(pconfig->section, "fileLog")) {
        pconfig->fileLog = myAtoi(value);
    } else if (MATCH(pconfig->section, "screenLog")) {
        pconfig->screenLog = myAtoi(value);
    } else if (MATCH(pconfig->section, "screenEnabled")) {
        pconfig->screenEnabled = myAtoi(value);
    } else if (MATCH(pconfig->section, "screenBrightness")) {
        pconfig->screenBrightness = numberToInt(value);
    } else if (MATCH(pconfig->section, "deviceID")) {
        pconfig->deviceID =numberToInt(value);
    } else if (MATCH(pconfig->section, "CTCertPath")) {
        strcpy (pconfig->CTCertPath,value);
    }
    return 1;
}

int readPayloadSection(configuration* app, FIL* configFile)
{
    debug("Reading section %s",app->section);
    int config_err = iniparse(handler, app, configFile);

    switch (config_err) {
        case 0:
            break;
        case -2:
            // should not happen, however better be safe than sorry
            panic("Config file is too big.");
            break;
        default:
            panic("Error found in config file, error code %i",config_err);
            break;
    }
    return config_err;
}
