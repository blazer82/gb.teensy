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
Memory *PPU::mem = 0;

void PPU::getBackgroundForLine(const uint8_t y, uint16_t *frame, const uint8_t originX, const uint8_t originY) {
    memset(frame + y * 160, 0x33, sizeof(uint16_t) * 160);
    uint8_t tileIndex, tileLineU, tileLineL;

    uint8_t tilePosY = floor(y / 8) * 8;
    uint8_t tileLineY = y - tilePosY;

    for (uint8_t i = 0; i < 20; i++) {
        tileIndex = Memory::readByte(MEM_VRAM_MAP1 + i + 32 * (tilePosY / 8));
        tileLineL = Memory::readByte(MEM_VRAM_TILES + tileIndex * 16 + tileLineY * 2);
        tileLineU = Memory::readByte(MEM_VRAM_TILES + tileIndex * 16 + tileLineY * 2 + 1);

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
    uint8_t y = Memory::readByte(MEM_LCD_Y) % 152;
    static uint8_t sendingFrame = 1;
    static uint8_t calculatingFrame = 0;

    while (ticks < CPU::totalCycles) {
        ticks++;
        const uint8_t cycleTicks = ticks % 114;

        switch (cycleTicks) {
            case 0:  // reading from OAM memory
                // TODO: Disable access to OAM during this time
                lcdStatus = Memory::readByte(MEM_LCD_STATUS);
                // Set LCDC to Mode 2: Searching OAM
                Memory::writeByteInternal(MEM_LCD_STATUS, (lcdStatus & 0xFC) | 0x02, true);
                // Trigger an OAM interrupt through LCD STAT if enabled
                if ((lcdStatus & 0x20) == 0x20) {
                    Memory::interrupt(IRQ_LCD_STAT);
                }
                break;

            case 20:  // reading from both OAM and VRAM
                // TODO: Disable access to all video memory during this time
                lcdStatus = Memory::readByte(MEM_LCD_STATUS);
                Memory::writeByteInternal(MEM_LCD_STATUS, (lcdStatus & 0xFC) | 0x03, true); // Shouldn't this be 0x03? If things break, return 0x03 to 0x02
                // Check if we the current line is the same as what's in LY Compare (LYC)
                if (y == Memory::readByte(MEM_LCD_YC)) {
                    // Set coincidence flag
                    Memory::writeByteInternal(MEM_LCD_STATUS, (lcdStatus & 0xFB) | 0x04, true);
                    // Trigger coincidence interrupt through LCD STAT if enabled
                    if ((lcdStatus & 0x40) == 0x40) {
                        Memory::interrupt(IRQ_LCD_STAT);
                    }
                } else {
                    // Otherwise, clear the coincidence flag
                    Memory::writeByteInternal(MEM_LCD_STATUS, (lcdStatus & 0xFB) | 0x00, true);
                }
                break;

            case 43:  // H-Blank and V-Blank period
                lcdc = Memory::readByte(MEM_LCDC);
                lcdStatus = Memory::readByte(MEM_LCD_STATUS);
                // Check if LCD is enabled
                if ((lcdc & 0x80) == 0x80) {
                    y = (y + 1) % 152;
                    // Update the current LCD Y coordinate
                    Memory::writeByte(MEM_LCD_Y, y);
                    // Get the X and Y posision of the background map
                    originY = Memory::readByte(MEM_LCD_SCROLL_Y);
                    originX = Memory::readByte(MEM_LCD_SCROLL_X);
                    // Make sure we're in the visible portion of the screen
                    if (y < 144) {
                        // calculate line
                        // Check if background is enabled
                        if ((lcdc & 0x01) == 0x01) {
                            // Get the background
                            getBackgroundForLine(y, frames[calculatingFrame], originX, originY);
                        }
                        // Check if sprites are enabled
                        if ((lcdc & 0x02) == 0x02) {
                            // Get the sprite
                            getSpritesForLine(y, frames[calculatingFrame]);
                        }
                        
                        Memory::writeByteInternal(MEM_LCD_STATUS, (lcdStatus & 0xFC) | 0x00, true);
                        if ((lcdStatus & 0x08) == 0x08) {
                            Memory::interrupt(IRQ_LCD_STAT);
                        }
                    } else if (y == 144) {
                        Memory::writeByteInternal(MEM_LCD_STATUS, (lcdStatus & 0xFC) | 0x01, true);
                        Memory::interrupt(IRQ_VBLANK);

                        mapColorsForFrame(frames[calculatingFrame]);

                        sendingFrame = calculatingFrame;
                        calculatingFrame = !calculatingFrame;

                        ft81x.writeGRAM(0, 2 * 160 * 144, (uint8_t *)frames[sendingFrame]);
                    }
                } else {
                    Memory::writeByteInternal(MEM_LCD_STATUS, (lcdStatus & 0xFC) | 0x01, true);
                }
                break;

            default:
                break;
        }
    }
}
