#pragma once

#include <sys/_stdint.h>
#include "Display.h"

class PPU
{
    public:
        PPU();
        void ppuStep();
    protected:
        Display display;
        
        void getBackgroundForLine(uint8_t y, uint16_t *line, uint8_t originX, uint8_t originY);
        void getSpritesForLine(uint8_t y, uint16_t *line);
        void mapColorsForLine(uint16_t *line);
    private:
};
