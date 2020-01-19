/**
 * This file is part of the gb.teensy emulator
 * <https://github.com/blazer82/gb.teensy>.
 * 
 * It is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * It is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 **/

#pragma once

#include <sys/_stdint.h>

#define DISPLAY_ENABLED

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
