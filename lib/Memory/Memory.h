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

#define MEM_IRQ_ENABLE    0xFFFF
#define MEM_IRQ_FLAG      0xFF0F
#define MEM_JOYPAD        0xFF00
#define MEM_LCDC          0xFF40
#define MEM_LCD_STATUS    0xFF41
#define MEM_LCD_SCROLL_Y  0xFF42
#define MEM_LCD_SCROLL_X  0xFF43
#define MEM_LCD_Y         0xFF44
#define MEM_DMA           0xFF46

// DIV: Divider Register
// Incremented at a rate of 16384 Hz. 
// All writes reset to 0x0
#define MEM_DIVIDER       0xFF04

// TIMA: Timer Counter
// Incremented by clock frequency specified in TAC
// When it overflows (>0xFF), reset to value in TMA
// and generate an interrupt
#define MEM_TIMA          0xFF05

// TMA: Timer Modulo
// Set TIMA to this value when TIMA overflows
#define MEM_TMA           0xFF06

// TAC: Timer Control
// Bit 0-1  Input Clock Select
//  0b00    4096 Hz
//  0b01    262144 Hz
//  0b10    65536 Hz
//  0b11    16384 Hz
// Bit 2:   Timer Stop (0=Stop, 1=Start)
#define MEM_TIMER_CONTROL 0xFF07

#define MEM_VRAM_MAP1     0x9800
#define MEM_VRAM_MAP2     0x9C00
#define MEM_VRAM_TILES    0x8000
#define MEM_RAM_ECHO      0xE000
#define MEM_RAM_INTERNAL  0xC000
#define MEM_ROM_BANK      0x4000

#define IRQ_VBLANK 0x01
#define IRQ_TIMER  0x04

#define PC_START  0x0100
#define PC_VBLANK 0x0040
#define PC_TIMER  0x0050

class Memory {
   public:
    static void initMemory();

    static void writeByte(const unsigned int location, const uint8_t data);
    static void writeByteInternal(const unsigned int location, const uint8_t data, const bool internal);
    static uint8_t readByte(const unsigned int location);

    static void interrupt(const uint8_t flag);

   protected:
   private:
    static uint8_t memory[32768];
    static bool mbc1_mode;
    static uint8_t romBank;
};
