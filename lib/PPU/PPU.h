/**
 * gb.teensy Emulation Software
 * Copyright (C) 2020  Raphael Stäbler
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 **/

#pragma once

#include <sys/_stdint.h>

class PPU
{
    public:
        PPU();
        void ppuStep();
    protected:
        void getBackgroundForLine(const uint8_t y, uint16_t *line, const uint8_t originX, const uint8_t originY);
        void getSpritesForLine(const uint8_t y, uint16_t *line);
        void mapColorsForLine(uint16_t *line);
    private:
};
