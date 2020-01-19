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

#include <string.h>
#include <Arduino.h>
#include "Memory.h"
#include "Cartridge.h"

#define MAX(a, b) (((a) > (b)) ? (a) : (b))

bool mbc1_mode = 0;
uint8_t romBank = 0;

// TODO: Maybe switch to compile-time allocation
uint8_t *Memory::memory = (uint8_t*) malloc((0xFFFF - 0x8000 + 1) * sizeof(uint8_t));

void Memory::writeByteInternal(unsigned int location, uint8_t data, bool internal)
{
    uint16_t d;

    switch (location)
    {
    // Joypad
    case MEM_JOYPAD:
        if (internal) {
            memory[location - 0x8000] = data;
        }
        else {
            memory[location - 0x8000] = (memory[location - 0x8000] & 0xCF) | (data & 0x30);
        }
        break;

    // LCD Status
    case MEM_LCD_STATUS:
        if (internal) {
            memory[location - 0x8000] = data;
        }
        else {
            memory[location - 0x8000] = (memory[location - 0x8000] & 0x07) | (data | 0xF8);
        }
        break;

    // DMA transfer
    case MEM_DMA:
        d = 0xFE00;
        for (uint16_t s = data * 0x100; s < data * 0x100 + 0xA0; s++) {
            memory[d - 0x8000] = readByte(s);
            d++;
        }
        break;

    // Divider
    case MEM_DIVIDER:
        if (internal) {
            memory[location - 0x8000] = data;
        }
        else {
            memory[location - 0x8000] = 0x00;
        }
        break;
    
    default:
        // TODO: RAM Echo (seems to break things tho)
        // TODO: Switchable RAM banks
        // TODO: Add support for ROM switch other than MBC1
        if (location >= 0x6000 && location < MEM_VRAM_TILES) {
            mbc1_mode = data & 1;
            Serial.printf("MBC1 mode set to %#x\n", mbc1_mode);
        }
        else if (location >= 0x2000 && location < MEM_ROM_BANK) {
            romBank = MAX(data & 0x1F, 1) - 1;
            Serial.printf("ROM bank set to %i\n", romBank);
        }
        else if (mbc1_mode == 0 && location >= MEM_ROM_BANK && location < 0x6000) {
            Serial.printf("Attempted to set ROM addressing to %#x TODO: implement!\n", data & 0x3);
        }
        else if (location >= MEM_VRAM_TILES) {
            memory[location - 0x8000] = data;
        }
        else {
            // Illegal operation
        }
        
        break;
    }
}

void Memory::writeByte(unsigned int location, uint8_t data) {
    writeByteInternal(location, data, false);
}

uint8_t Memory::readByte(unsigned int location)
{
    if (location < MEM_VRAM_TILES) {
        if (location >= MEM_ROM_BANK) {
            return Cartridge::cartridge[location + (0x4000 * romBank)];
        }
        else {
            return Cartridge::cartridge[location];
        }
    }
    else {
        return memory[location - 0x8000];
    }
}

void Memory::interrupt(uint8_t flag) {
    writeByte(MEM_IRQ_FLAG, readByte(MEM_IRQ_FLAG) | flag);
}

void Memory::initMemory() {
    // Reset memory to zero
    memset(memory, 0, 0xFFFF - 0x8000 + 1);

    // Init joypad flags
    writeByteInternal(MEM_JOYPAD, 0x2F, true);

    // Init memory according to original GB
    writeByteInternal(0xFF10, 0x80, true);
    writeByteInternal(0xFF11, 0xBF, true);
    writeByteInternal(0xFF12, 0xF3, true);
    writeByteInternal(0xFF14, 0xBF, true);
    writeByteInternal(0xFF16, 0x3F, true);
    writeByteInternal(0xFF19, 0xBF, true);
    writeByteInternal(0xFF1A, 0x7F, true);
    writeByteInternal(0xFF1B, 0xFF, true);
    writeByteInternal(0xFF1C, 0x9F, true);
    writeByteInternal(0xFF1E, 0xBF, true);
    writeByteInternal(0xFF20, 0xFF, true);
    writeByteInternal(0xFF23, 0xBF, true);
    writeByteInternal(0xFF24, 0x77, true);
    writeByteInternal(0xFF25, 0xF3, true);
    writeByteInternal(0xFF26, 0xF1, true);
    writeByteInternal(MEM_LCDC, 0x91, true);
    writeByteInternal(0xFF47, 0xFC, true);
    writeByteInternal(0xFF48, 0xFF, true);
    writeByteInternal(0xFF49, 0xFF, true);
}