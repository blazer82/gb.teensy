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

#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "PPU.h"
#include "CPU.h"
#include "Memory.h"
#include "FT81x.h"

#define COLOR1 0x0000
#define COLOR2 0x4BC4
#define COLOR3 0x968B
#define COLOR4 0xFFFF

uint16_t PPU::lines[2][160] = {{0}, {0}};
uint64_t PPU::ticks = 0;
uint8_t PPU::originX = 0, PPU::originY = 0, PPU::lcdc = 0, PPU::lcdStatus = 0;

void PPU::getBackgroundForLine(const uint8_t y, uint16_t *line, const uint8_t originX, const uint8_t originY) {
    memset(line, 0x33, sizeof(uint16_t) * 160);
    uint8_t tileIndex, tileLineU, tileLineL;

    uint8_t tilePosY = floor(y / 8) * 8;
    uint8_t tileLineY = y - tilePosY;

    for (uint8_t i = 0; i < 20; i++) {
        tileIndex = Memory::readByte(MEM_VRAM_MAP1 + i + 32 * (tilePosY / 8));
        tileLineL = Memory::readByte(MEM_VRAM_TILES + tileIndex * 16 + tileLineY * 2);
        tileLineU = Memory::readByte(MEM_VRAM_TILES + tileIndex * 16 + tileLineY * 2 + 1);

        for (int8_t c = 0; c < 8; c++) {
            line[i * 8 + c] = (((tileLineU >> (7 - c)) << 1) & 0x2) | ((tileLineL >> (7 - c)) & 0x1);
        }
    }
}

void PPU::getSpritesForLine(const uint8_t y, uint16_t *line) {
    uint8_t spritePosX, spritePosY;
    uint8_t tileIndex, attributes, tileLineU, tileLineL, pixel;
    int16_t spriteLineY, x;

    for (uint16_t i = 0xFE00; i < 0xFEA0; i += 4) {
        spritePosY = Memory::readByte(i) - 16;
        spriteLineY = y - spritePosY;
        if (spriteLineY >= 0 && spriteLineY < 8) {
            spritePosX = Memory::readByte(i + 1) - 8;
            tileIndex = Memory::readByte(i + 2);
            attributes = Memory::readByte(i + 3);

            tileLineL = Memory::readByte(MEM_VRAM_TILES + tileIndex * 16 + spriteLineY * 2);
            tileLineU = Memory::readByte(MEM_VRAM_TILES + tileIndex * 16 + spriteLineY * 2 + 1);

            for (int8_t c = 0; c < 8; c++) {
                x = spritePosX + c;
                if (x >= 0) {
                    if ((attributes & 0x80) == 0 || line[x] == 0) {
                        pixel = (((tileLineU >> (7 - c)) << 1) & 0x2) | ((tileLineL >> (7 - c)) & 0x1);
                        if (pixel != 0) line[x] = pixel;
                    }
                }
            }
        }
    }
}

void PPU::mapColorsForLine(uint16_t *line) {
    for (uint8_t i = 0; i < 160; i++) {
        line[i] = (line[i] == 0x3) ? COLOR1 : ((line[i] == 0x2) ? COLOR2 : ((line[i] == 0x1) ? COLOR3 : COLOR4));
    }
}

void PPU::ppuStep() {
    uint8_t y = Memory::readByte(MEM_LCD_Y) % 152;
    int sendingLine = -1;
    int calculatingLine = 0;

    for (; ticks < CPU::totalCycles; ticks++) {
        if (ticks % 116 == 0) {
            lcdc = Memory::readByte(MEM_LCDC);
            lcdStatus = Memory::readByte(MEM_LCD_STATUS);

            if ((lcdc & 0x80) == 0x80) {
                y = (y + 1) % 152;

                Memory::writeByte(MEM_LCD_Y, y);
                originY = Memory::readByte(MEM_LCD_SCROLL_Y);
                originX = Memory::readByte(MEM_LCD_SCROLL_X);

                if (y < 144) {
                    // calculate line
                    if ((lcdc & 0x01) == 0x01) {
                        getBackgroundForLine(y, lines[calculatingLine], originX, originY);
                    }

                    if ((lcdc & 0x02) == 0x02) {
                        getSpritesForLine(y, lines[calculatingLine]);
                    }

                    mapColorsForLine(lines[calculatingLine]);

                    sendingLine = calculatingLine;
                    calculatingLine = (calculatingLine == 0) ? 1 : 0;

                    FT81x::writeGRAM(y * 320, 320, (uint8_t *) lines[sendingLine]);

                    Memory::writeByteInternal(MEM_LCD_STATUS, (lcdStatus & 0xFC) | 0x00, true);
                }
                else if (y == 144) {
                    Memory::writeByteInternal(MEM_LCD_STATUS, (lcdStatus & 0xFC) | 0x01, true);
                    Memory::interrupt(IRQ_VBLANK);
                }
            }
            else {
                Memory::writeByteInternal(MEM_LCD_STATUS, (lcdStatus & 0xFC) | 0x01, true);
            }
        }
    }
}
