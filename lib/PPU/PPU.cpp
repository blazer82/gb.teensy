#include <string.h>
#include <math.h>
#include "PPU.h"
#include "Display.h"
#include "CPU.h"
#include "Memory.h"

#define COLOR1 0x0000
#define COLOR2 0x4BC4
#define COLOR3 0x968B
#define COLOR4 0xFFFF

uint16_t *lines[2];
uint64_t ticks = 0;
uint8_t originX, originY, lcdc, lcdStatus;
uint16_t offsetX, offsetY;

PPU::PPU() {
    //Allocate memory for the pixel buffers
    for (int i = 0; i < 2; i++) {
        lines[i] = (uint16_t*) malloc(320 * sizeof(uint16_t));
        memset(lines[i], 0, 320 * sizeof(uint16_t));
    }

    // Init offsets
    offsetX = (320 - 160) / 2;
    offsetY = (240 - 144) / 2;

    // Start display device
    display = Display();
    display.begin();
    display.setRotation(1);
    display.fillScreen(ILI9341_BLACK);

    CPU::cpuEnabled = 1;
}

void PPU::getBackgroundForLine(uint8_t y, uint16_t *line, uint8_t originX, uint8_t originY) {
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

void PPU::getSpritesForLine(uint8_t y, uint16_t *line) {
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

                    // finish sending line
                    /*if (sendingLine != -1) {
                        send_line_finish(spi);
                    }*/

                    sendingLine = calculatingLine;
                    calculatingLine = (calculatingLine == 0) ? 1 : 0;

                    display.hLine(y + offsetY, offsetX, lines[sendingLine]);

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
