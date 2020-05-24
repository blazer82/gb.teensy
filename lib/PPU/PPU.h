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
    // PPU();
    static void ppuStep();

protected:
    static uint16_t frames[2][160 * 144];
    static uint64_t ticks;
    static uint8_t originX, originY, lcdc, lcdStatus;

    static void getBackgroundForLine(const uint8_t y, uint16_t *frame, const uint8_t originX, const uint8_t originY);
    static void getSpritesForLine(const uint8_t y, uint16_t *frame);
    static void mapColorsForFrame(uint16_t *frame);

private:
};
