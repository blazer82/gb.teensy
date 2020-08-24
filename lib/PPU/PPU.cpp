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

#include "PPU.h"

#include <math.h>
#include <stdlib.h>
#include <string.h>

#include "CPU.h"
#include "Memory.h"

#define COLOR1 0x0000
#define COLOR2 0x4BC4
#define COLOR3 0x968B
#define COLOR4 0xFFFF

uint16_t PPU::frames[2][160 * 144] = {{0}, {0}};
uint64_t PPU::ticks = 0;
uint8_t PPU::originX = 0, PPU::originY = 0, PPU::lcdc = 0, PPU::lcdStatus = 0;
Memory* PPU::mem = 0;
void PPU::setMemoryHandle(Memory* memHandle){
    PPU::mem = memHandle;
}

void PPU::getBackgroundForLine(const uint8_t y, uint16_t *frame, const uint8_t originX, const uint8_t originY) {
    memset(frame + y * 160, 0x33, sizeof(uint16_t) * 160);
    uint8_t tileIndex, tileLineU, tileLineL;

    uint8_t tilePosY = floor(y / 8) * 8;
    uint8_t tileLineY = y - tilePosY;

    for (uint8_t i = 0; i < 20; i++) {
        tileIndex = PPU::mem->readByte(MEM_VRAM_MAP1 + i + 32 * (tilePosY / 8));
        tileLineL = PPU::mem->readByte(MEM_VRAM_TILES + tileIndex * 16 + tileLineY * 2);
        tileLineU = PPU::mem->readByte(MEM_VRAM_TILES + tileIndex * 16 + tileLineY * 2 + 1);

        for (int8_t c = 0; c < 8; c++) {
            frame[y * 160 + i * 8 + c] = (((tileLineU >> (7 - c)) << 1) & 0x2) | ((tileLineL >> (7 - c)) & 0x1);
        }
    }
}

void PPU::getSpritesForLine(const uint8_t y, uint16_t *frame) {
    uint8_t spritePosX, spritePosY;
    uint8_t tileIndex, attributes, tileLineU, tileLineL, pixel;
    int16_t spriteLineY, x;

    for (uint16_t i = 0xFE00; i < 0xFEA0; i += 4) {
        spritePosY = PPU::mem->readByte(i) - 16;
        spriteLineY = y - spritePosY;
        if (spriteLineY >= 0 && spriteLineY < 8) {
            spritePosX = PPU::mem->readByte(i + 1) - 8;
            tileIndex = PPU::mem->readByte(i + 2);
            attributes = PPU::mem->readByte(i + 3);

            tileLineL = PPU::mem->readByte(MEM_VRAM_TILES + tileIndex * 16 + spriteLineY * 2);
            tileLineU = PPU::mem->readByte(MEM_VRAM_TILES + tileIndex * 16 + spriteLineY * 2 + 1);

            for (int8_t c = 0; c < 8; c++) {
                x = spritePosX + c;
                if (x >= 0) {
                    if ((attributes & 0x80) == 0 || frame[y * 160 + x] == 0) {
                        pixel = (((tileLineU >> (7 - c)) << 1) & 0x2) | ((tileLineL >> (7 - c)) & 0x1);
                        if (pixel != 0) frame[y * 160 + x] = pixel;
                    }
                }
            }
        }
    }
}

void PPU::mapColorsForFrame(uint16_t *frame) {
    for (uint16_t i = 0; i < 160 * 144; i++) {
        frame[i] = (frame[i] == 0x3) ? COLOR1 : ((frame[i] == 0x2) ? COLOR2 : ((frame[i] == 0x1) ? COLOR3 : COLOR4));
    }
}

void PPU::ppuStep(FT81x &ft81x) {
    uint8_t y = PPU::mem->readByte(MEM_LCD_Y) % 152;
    static uint8_t sendingFrame = 1;
    static uint8_t calculatingFrame = 0;

    for (; ticks < CPU::totalCycles; ticks++) {
        if (ticks % 116 == 0) {
            lcdc = PPU::mem->readByte(MEM_LCDC);
            lcdStatus = PPU::mem->readByte(MEM_LCD_STATUS);

            if ((lcdc & 0x80) == 0x80) {
                y = (y + 1) % 152;

                PPU::mem->writeByte(MEM_LCD_Y, y);
                originY = PPU::mem->readByte(MEM_LCD_SCROLL_Y);
                originX = PPU::mem->readByte(MEM_LCD_SCROLL_X);

                if (y < 144) {
                    // calculate line
                    if ((lcdc & 0x01) == 0x01) {
                        getBackgroundForLine(y, frames[calculatingFrame], originX, originY);
                    }

                    if ((lcdc & 0x02) == 0x02) {
                        getSpritesForLine(y, frames[calculatingFrame]);
                    }

                    PPU::mem->writeByteInternal(MEM_LCD_STATUS, (lcdStatus & 0xFC) | 0x00, true);
                } else if (y == 144) {
                    PPU::mem->writeByteInternal(MEM_LCD_STATUS, (lcdStatus & 0xFC) | 0x01, true);
                    PPU::mem->interrupt(IRQ_VBLANK);

                    mapColorsForFrame(frames[calculatingFrame]);

                    sendingFrame = calculatingFrame;
                    calculatingFrame = !calculatingFrame;

                    ft81x.writeGRAM(0, 2 * 160 * 144, (uint8_t *)frames[sendingFrame]);
                }
            } else {
                PPU::mem->writeByteInternal(MEM_LCD_STATUS, (lcdStatus & 0xFC) | 0x01, true);
            }
        }
    }
}
