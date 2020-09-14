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
#include <Timer.h>

#include "APU.h"

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
        // Handle writes to the Divider (DIV) register
        case MEM_DIVIDER:
            Timer::writeDiv(data);
            break;

        // Handle writes to the TIMA register
        case MEM_TIMA:
            Timer::writeTima(data);
            break;
        
        // Handle writes to the TMA Register
        case MEM_TMA:
            Timer::writeTma(data);
            break;

        // Handle writes to the Timer Control (TAC) Register
        case MEM_TIMER_CONTROL:
            Timer::writeTac(data);
            break;

        // Handle writes to the Interrupt Flag (IF) register
        case MEM_IRQ_FLAG:
            // Check for timer interrupt requests
            if (data & IRQ_TIMER){
                // Send them to the timer
                Timer::setInt();
            }
            // Don't store the Timer interrupt flag here. It's
            // handled by Timer
            ioreg[MEM_IRQ_FLAG - MEM_IO_REGS] = (data & ~IRQ_TIMER);

        // Sound length counter
        // Resides in I/O region
        case MEM_SOUND_NR11:
            ioreg[MEM_SOUND_NR11 - MEM_IO_REGS] = data;
            if (!internal) {
                APU::loadLength1();
            }
            break;
        case MEM_SOUND_NR21:
            ioreg[MEM_SOUND_NR21 - MEM_IO_REGS] = data;
            if (!internal) {
                APU::loadLength2();
            }
            break;

        // Sound channel enable
        // Resides in I/O region
        case MEM_SOUND_NR14:
            ioreg[MEM_SOUND_NR14 - MEM_IO_REGS] = data;
            if (!internal) {
                if (data >> 7) {
                    APU::triggerSquare1();
                }
            }
            break;
        case MEM_SOUND_NR24:
            ioreg[MEM_SOUND_NR24 - MEM_IO_REGS] = data;
            if (!internal) {
                if (data >> 7) {
                    APU::triggerSquare2();
                }
            }
            break;
        case MEM_SOUND_NR44:
            ioreg[MEM_SOUND_NR44 - MEM_IO_REGS] = data;
            if (!internal) {
                if (data >> 7) {
                    APU::triggerNoise();
                }
            }
            break;

        default:
            // Handle writes to the IE register
            if (location >= MEM_INT_EN_REG) {
                iereg = data;
            }
            // Handle writes to High RAM
            else if (location >= MEM_HIGH_RAM) {
                hram[location - MEM_HIGH_RAM] = data;
            }
            // Handle writes to IO registers
            else if (location >= MEM_IO_REGS) {
                ioreg[location - MEM_IO_REGS] = data;
            }
            // Handle writes to unusable memory
            else if (location >= MEM_UNUSABLE) {
                return;
            }
            // Handle writes to OAM
            else if (location >= MEM_SPRITE_ATTR_TABLE) {
                oam[location - MEM_SPRITE_ATTR_TABLE] = data;
            }
            // Handle writes to echo memory
            else if (location >= MEM_RAM_ECHO) {
                // Just write to the beginning of internal RAM
                wram[location - MEM_RAM_ECHO] = data;
            }
            // Handle writes to internal Work RAM
            else if (location >= MEM_RAM_INTERNAL) {
                wram[location - MEM_RAM_INTERNAL] = data;
            }
            // Handle writes to external cartridge RAM
            if (location >= MEM_RAM_EXTERNAL) {
                Cartridge::writeByte(location, data);
            }
            // Handle writes to VRAM
            else if (location >= MEM_VRAM_TILES) {
                vram[location - MEM_VRAM_TILES] = data;
            }
            // Handle writes to cart ROM
            // These are usually mapped to MBC control registers in the cart
            else if (location >= MEM_ROM) {
                Cartridge::writeByte(location, data);
            } else {
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
    if (location >= MEM_INT_EN_REG) {
        return iereg;
    }
    // Handle reads from High RAM
    else if (location >= MEM_HIGH_RAM) {
        return hram[location - MEM_HIGH_RAM];
    }
    // Handle reads from IO registers
    else if (location >= MEM_IO_REGS) {
        // Handle reads to IF register
        if(location == MEM_IRQ_FLAG){
            // Get the IRQ bit for the Timer from Timer
            return (ioreg[MEM_IRQ_FLAG - MEM_IO_REGS] & ~IRQ_TIMER) | Timer::checkInt() << 2;
        }
        // Handle reads to the DIV register
        else if(location == MEM_DIVIDER){
            return Timer::readDiv();
        }
        // Handle reads to TIMA register
        else if(location == MEM_TIMA){
            return Timer::readTima();
        }
        // Handle reads to TMA register
        else if(location == MEM_TMA){
            return Timer::readTma();
        }
        // Handle reads to TAC register
        else if(location == MEM_TIMER_CONTROL){
            return Timer::readTac();
        }
        // Handle all other IO reg locations
        else{
            return ioreg[location - MEM_IO_REGS];
        }
    }
    // Handle reads from unusable memory
    // TODO: Assume reads here return 0xFF. Look this up
    else if (location >= MEM_UNUSABLE) {
        return 0xFF;
    }
    // Handle reads from OAM
    else if (location >= MEM_SPRITE_ATTR_TABLE) {
        return oam[location - MEM_SPRITE_ATTR_TABLE];
    }
    // Handle reads from echo memory
    else if (location >= MEM_RAM_ECHO) {
        // Just read from the beginning of internal RAM
        return wram[location - MEM_RAM_ECHO];
    }
    // Handle reads from internal Work RAM
    else if (location >= MEM_RAM_INTERNAL) {
        return wram[location - MEM_RAM_INTERNAL];
    }
    // Handle reads from external cartridge RAM
    if (location >= MEM_RAM_EXTERNAL) {
        return Cartridge::readByte(location);
    }
    // Handle reads from VRAM
    else if (location >= MEM_VRAM_TILES) {
        return vram[location - MEM_VRAM_TILES];
    }
    // Handle reads from cart ROM
    else if (location >= MEM_ROM) {
        return Cartridge::readByte(location);
    } else {
        // Illegal operation
        Serial.println("Illegal write operation on memory!");
        Serial.printf("\tAttempted read from 0x%x\n", location);
    }
}

void Memory::interrupt(uint8_t flag) { writeByte(MEM_IRQ_FLAG, readByte(MEM_IRQ_FLAG) | flag); }

void Memory::initMemory() {
    // Initialize the memory like the original 
    writeByteInternal(MEM_JOYPAD, 0xCF, true);          // FF00
    writeByteInternal(MEM_SERIAL_SB, 0x00, true);       // FF01
    writeByteInternal(MEM_SERIAL_SC, 0x7E, true);       // FF02
    writeByteInternal(0xFF03, 0xFF, true);              // FF03
    // 0xFF04 is handled by the Timer                   // FF04
    writeByteInternal(MEM_TIMA, 0x00, true);            // FF05
    writeByteInternal(MEM_TMA, 0x00, true);             // FF06
    writeByteInternal(MEM_TIMER_CONTROL, 0xF8, true);   // FF07
    for(uint8_t i = 0; i < 7; i++){
        writeByteInternal(0xFF08 + i, 0xFF, true);      // FF08 - FF0E
    }
    writeByteInternal(MEM_IRQ_FLAG, 0xE1, true);        // FF0F
    // Init memory according to original GB
    writeByteInternal(MEM_SOUND_NR10, 0x80, true);      // FF10
    writeByteInternal(MEM_SOUND_NR11, 0xBF, true);      // FF11
    writeByteInternal(MEM_SOUND_NR12, 0xF3, true);      // FF12
    writeByteInternal(MEM_SOUND_NR13, 0xFF, true);      // FF13
    writeByteInternal(MEM_SOUND_NR14, 0xBF, true);      // FF14
    writeByteInternal(0xFF15, 0xFF, true);              // FF15
    writeByteInternal(MEM_SOUND_NR21, 0x3F, true);      // FF16
    writeByteInternal(MEM_SOUND_NR22, 0x00, true);      // FF17
    writeByteInternal(MEM_SOUND_NR23, 0xFF, true);      // FF18
    writeByteInternal(MEM_SOUND_NR24, 0xBF, true);      // FF19
    writeByteInternal(MEM_SOUND_NR30, 0x7F, true);      // FF1A
    writeByteInternal(MEM_SOUND_NR31, 0xFF, true);      // FF1B
    writeByteInternal(MEM_SOUND_NR32, 0x9F, true);      // FF1C
    writeByteInternal(MEM_SOUND_NR33, 0xFF, true);      // FF1D
    writeByteInternal(MEM_SOUND_NR34, 0xBF, true);      // FF1E
    writeByteInternal(0xFF1F, 0xFF, true);              // FF1F
    writeByteInternal(MEM_SOUND_NR41, 0xFF, true);      // FF20
    writeByteInternal(MEM_SOUND_NR42, 0x00, true);      // FF21
    writeByteInternal(MEM_SOUND_NR43, 0x00, true);      // FF22
    writeByteInternal(MEM_SOUND_NR44, 0xBF, true);      // FF23
    writeByteInternal(MEM_SOUND_NR50, 0x77, true);      // FF24
    writeByteInternal(MEM_SOUND_NR51, 0xF3, true);      // FF25
    writeByteInternal(MEM_SOUND_NR52, 0xF1, true);      // FF26
    for(uint8_t i = 0; i < 25; i++){
        writeByteInternal(0xFF27 + i, 0xFF, true);      // FF27 - FF3F
    }
    writeByteInternal(MEM_LCDC, 0x91, true);            // FF40
    writeByteInternal(MEM_LCD_STATUS, 0x80, true);      // FF41
    writeByteInternal(MEM_LCD_SCROLL_Y, 0x00, true);    // FF42
    writeByteInternal(MEM_LCD_SCROLL_X, 0x00, true);    // FF43
    writeByteInternal(MEM_LCD_Y, 0x97, true);           // FF44
    writeByteInternal(MEM_LCD_YC, 0x00, true);          // FF45
    writeByteInternal(0xFF46, 0xFF, true);              // FF46
    writeByteInternal(0xFF47, 0xFC, true);              // FF47
    writeByteInternal(0xFF48, 0xFF, true);              // FF48
    writeByteInternal(0xFF49, 0xFF, true);              // FF49
    writeByteInternal(0xFF4A, 0x00, true);              // FF4A
    writeByteInternal(0xFF4B, 0x00, true);              // FF4B
    for(uint8_t i = 0; i < 52; i++){
        writeByteInternal(0xFF4C + i, 0xFF, true);      // FF4C - FF7F
    }

}