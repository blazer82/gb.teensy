#pragma once

#include <sys/_stdint.h>

#define DISPLAY_ENABLED

#ifdef DISPLAY_ENABLED
#include "Display.h"
#endif

class PPU
{
    public:
        static void init();
        static void ppuStep();
    protected:
#ifdef DISPLAY_ENABLED
        static Display display;
#endif
        
        static void getBackgroundForLine(uint8_t y, uint16_t *line, uint8_t originX, uint8_t originY);
        static void getSpritesForLine(uint8_t y, uint16_t *line);
        static void mapColorsForLine(uint16_t *line);
    private:
};
