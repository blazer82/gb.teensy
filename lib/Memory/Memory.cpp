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

#include "Memory.h"

#include <Arduino.h>
#include <string.h>

#define MAX(a, b) (((a) > (b)) ? (a) : (b))

uint8_t Memory::vram[0x2000] = {0};
uint8_t vram[0x2000] = {0};
uint8_t Memory::wram[0x2000] = {0};
uint8_t Memory::oam[0xA0] = {0};
uint8_t Memory::ioreg[0x80] = {0};
uint8_t Memory::hram[0x7F] = {0};
uint8_t Memory::iereg = 0;

void Memory::writeByteInternal(const uint16_t location, const uint8_t data, const bool internal) {
    uint16_t d;
    switch (location) {
        // Handle write to Joypad registers at 0xFF00
        // Register resides in I/O region
        case MEM_JOYPAD:
            if (internal) {
                ioreg[MEM_JOYPAD - MEM_IO_REGS] = data;
            } else {
                ioreg[MEM_JOYPAD - MEM_IO_REGS] = (ioreg[MEM_JOYPAD - MEM_IO_REGS] & 0xCF) | (data & 0x30);
            }
            break;

        // Handle writes to LCD Status register
        // Resides in I/O region
        case MEM_LCD_STATUS:
            if (internal) {
                ioreg[MEM_LCD_STATUS - MEM_IO_REGS] = data;
            } else {
                ioreg[MEM_LCD_STATUS - MEM_IO_REGS] = (ioreg[MEM_LCD_STATUS - MEM_IO_REGS] & 0x07) | (data | 0xF8);
            }
            break;

        // Handle writes to DMA transfer register
        case MEM_DMA:
            d = 0x0;
            // DMA transfers occur from ROM/RAM to OAM in chunks of 0xA0 bytes
            // The address of ROM/RAM to transfer to OAM is the data * 0x100
            for (uint16_t s = data * 0x100; s < data * 0x100 + 0xA0; s++) {
                oam[d] = readByte(s);
                d++;
            }
            break;

        // Handle writes to the Divider register
        // Resides in I/O region
        case MEM_DIVIDER:
            if (internal) {
                ioreg[MEM_DIVIDER - MEM_IO_REGS] = data;
            } else {
                // Writes to the divider just clear it
                ioreg[MEM_DIVIDER - MEM_IO_REGS] = 0x00;
            }
            break;

        default:
            // Handle writes to the IE register
            if(location >= MEM_INT_EN_REG){
                iereg = data;
            }
            // Handle writes to High RAM
            else if(location >= MEM_HIGH_RAM){
                hram[location - MEM_HIGH_RAM] = data;
            }
            // Handle writes to IO registers
            else if(location >= MEM_IO_REGS){
                ioreg[location - MEM_IO_REGS] = data;
            }
            // Handle writes to unusable memory
            else if(location >= MEM_UNUSABLE){
                return;
            }
            // Handle writes to OAM
            else if(location >= MEM_SPRITE_ATTR_TABLE){
                oam[location - MEM_SPRITE_ATTR_TABLE] = data;
            }
            // Handle writes to echo memory
            else if(location >= MEM_RAM_ECHO){
                // Just write to the beginning of internal RAM
                wram[location - MEM_RAM_ECHO] = data;
            }
            // Handle writes to internal Work RAM
            else if(location >= MEM_RAM_INTERNAL){
                wram[location - MEM_RAM_INTERNAL] = data;
            }
            // Handle writes to external cartridge RAM
            if(location >= MEM_RAM_EXTERNAL){
                Cartridge::writeByte(location, data);
            }
            // Handle writes to VRAM
            else if (location >= MEM_VRAM_TILES) {
                vram[location - MEM_VRAM_TILES] = data;
            }
            // Handle writes to cart ROM
            // These are usually mapped to MBC control registers in the cart
            else if(location >= MEM_ROM){
                Cartridge::writeByte(location, data);
            }
            else {
                // Illegal operation
                Serial.println("Illegal write operation on memory!");
                Serial.printf("\tAttempted write of 0x%x to 0x%x\n", data, location);
            }
            break;
    }
}

void Memory::writeByte(const uint16_t location, const uint8_t data) { writeByteInternal(location, data, false); }

uint8_t Memory::readByte(const uint16_t location) {
        // Handle reads of the IE register
    if(location >= MEM_INT_EN_REG){
        return iereg;
    }
    // Handle reads from High RAM
    else if(location >= MEM_HIGH_RAM){
        return hram[location - MEM_HIGH_RAM];
    }
    // Handle reads from IO registers
    else if(location >= MEM_IO_REGS){
        return ioreg[location - MEM_IO_REGS];
    }
    // Handle reads from unusable memory
    // TODO: Assume reads here return 0xFF. Look this up
    else if(location >= MEM_UNUSABLE){
        return 0xFF;
    }
    // Handle reads from OAM
    else if(location >= MEM_SPRITE_ATTR_TABLE){
        return oam[location - MEM_SPRITE_ATTR_TABLE] ;
    }
    // Handle reads from echo memory
    else if(location >= MEM_RAM_ECHO){
        // Just read from the beginning of internal RAM
        return wram[location - MEM_RAM_ECHO] ;
    }
    // Handle reads from internal Work RAM
    else if(location >= MEM_RAM_INTERNAL){
        return wram[location - MEM_RAM_INTERNAL];
    }
    // Handle reads from external cartridge RAM
    if(location >= MEM_RAM_EXTERNAL){
        return Cartridge::readByte(location);
    }
    // Handle reads from VRAM
    else if (location >= MEM_VRAM_TILES) {
        return vram[location - MEM_VRAM_TILES];
    }
    // Handle reads from cart ROM
    else if(location >= MEM_ROM){
        return Cartridge::readByte(location);
    }
    else {
        // Illegal operation
        Serial.println("Illegal write operation on memory!");
        Serial.printf("\tAttempted read from 0x%x\n", location);
    }
}

void Memory::interrupt(uint8_t flag) { writeByte(MEM_IRQ_FLAG, readByte(MEM_IRQ_FLAG) | flag); }

void Memory::initMemory() {
    // Reset memory to zero
    // memset(memory, 0, 0xFFFF - 0x8000 + 1);

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