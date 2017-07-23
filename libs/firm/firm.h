#pragma once

#include "common.h"

typedef struct __attribute__((packed))
{
    u32 offset;
    u8 *address;
    u32 size;
    u32 procType;
    u8 hash[0x20];
} FirmSection;

typedef struct __attribute__((packed))
{
    char magic[4];
    u32 reserved1;
    u8 *arm11Entry;
    u8 *arm9Entry;
    u8 reserved2[0x30];
    FirmSection section[4];
} Firm;
