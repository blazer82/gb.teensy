#pragma once

#include <sys/_stdint.h>

//#define DISPLAY_ENABLED

#ifdef DISPLAY_ENABLED
#include "Display.h"
#endif

class PPU
{
    public:
        PPU();
        void ppuStep();
    protected:
#ifdef DISPLAY_ENABLED
        Display display;
#endif
        
        void getBackgroundForLine(uint8_t y, uint16_t *line, uint8_t originX, uint8_t originY);
        void getSpritesForLine(uint8_t y, uint16_t *line);
        void mapColorsForLine(uint16_t *line);
    private:
};
