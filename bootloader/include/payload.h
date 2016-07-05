#pragma once

#include "common.h"
#include "config.h"

int checkPayload(configuration* app);
int patchPath(u32 size,const char *path);

int loadPayload(loaderConfiguration* loader, configuration* app, int payloadSource);
void runPayload(configuration* app);
