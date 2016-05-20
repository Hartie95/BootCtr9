#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

#include "font.h"
#include "draw.h"
#include "constants.h"

int current_y = 0;

void ClearScreen(unsigned char *screen, int color)
{
    int i;
    unsigned char *screenPos = screen;
    for (i = 0; i < (SCREEN_HEIGHT * SCREEN_WIDTH); i++) {
        *(screenPos++) = color >> 16;  // B
        *(screenPos++) = color >> 8;   // G
        *(screenPos++) = color & 0xFF; // R
    }
}

void clearScreens()
{
    ClearScreen(TOP_SCREENL,0);
    ClearScreen(TOP_SCREENR,0);
    ClearScreen(BOT_SCREEN,0);
}

void DrawCharacter(unsigned char *screen, int character, int x, int y, int color, int bgcolor)
{
    int yy, xx;
    for (yy = 0; yy < 8; yy++) {
        int xDisplacement = (x * BYTES_PER_PIXEL * SCREEN_WIDTH);
        int yDisplacement = ((SCREEN_WIDTH - (y + yy) - 1) * BYTES_PER_PIXEL);
        unsigned char *screenPos = screen + xDisplacement + yDisplacement;

        unsigned char charPos = font[character * 8 + yy];
        for (xx = 7; xx >= 0; xx--) {
            if ((charPos >> xx) & 1) {
                *(screenPos + 0) = color >> 16;  // B
                *(screenPos + 1) = color >> 8;   // G
                *(screenPos + 2) = color & 0xFF; // R
            } else {
                *(screenPos + 0) = bgcolor >> 16;  // B
                *(screenPos + 1) = bgcolor >> 8;   // G
                *(screenPos + 2) = bgcolor & 0xFF; // R
            }
            screenPos += BYTES_PER_PIXEL * SCREEN_WIDTH;
        }
    }
}

u32 DrawString(unsigned char *screen, const char *str, int x, int y, int color, int bgcolor)
{
    int i,position=0;
    for (i = 0; i < strlen(str); i++)
    {
        if(str[i]=='\n')
        {    
            y+=10;
            position=0;
        }
        else
        {
            DrawCharacter(screen, str[i], x + position, y, color, bgcolor);
            position+=8;
        }
    }
    return y;
}

void DrawStringF(int x, int y, const char *format, ...)
{
    char str[256];
    va_list va;

    va_start(va, format);
    vsnprintf(str, 256, format, va);
    va_end(va);

    DrawString(TOP_SCREENL, str, x, y, RGB(255, 255, 255), RGB(0, 0, 0));
    DrawString(TOP_SCREENR, str, x, y, RGB(255, 255, 255), RGB(0, 0, 0));
}

void drawDebug(const char *format, ...)
{
    char str[256];
    va_list va;

    va_start(va, format);
    vsnprintf(str, 256, format, va);
    va_end(va);

    current_y = DrawString(DEBUGSCREEN, str, 10, current_y, RGB(255, 255, 255), RGB(0, 0, 0));

    current_y += 10;
}
