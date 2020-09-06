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
#include "Cartridge.h"
#include "NoMBC.h"
#include "MBC1.h"
#include "MBC2.h"
#include <Arduino.h>

/*      Interrupts      */
// IME: Interrupt Master Enable Flag(W)
// 0 disables all interrupts, 1 enables all interrupts
#define MEM_IRQ_ENABLE          0xFFFF

// IF: Interrupt Flag (R/W)
// Bit 0:   V-Blank IRQ
// Bit 1:   LCD STAT IRQ
// Bit 2:   Timer IRQ
// Bit 3:   Serial IRQ
// Bit 4:   Joypad IRQ
#define MEM_IRQ_FLAG            0xFF0F

/*      Joypad Input    */
// P1/JOYP: Joypad Input (R/W)
// Button/Direction keys are muxed
// All bits are active low
// Bit 0:   Right or A (Read Only)
// Bit 1:   Left  or B (Read Only)
// Bit 2:   Up    or Select (Read Only)
// Bit 3:   Down  or Start (Read Only)
// Bit 4:   Select Direction Keys
// Bit 5:   Select Button Keys
// Bit 6-7: Unused
#define MEM_JOYPAD              0xFF00


/*      LCD Registers    */
// LCDC: LCD Control (R/W)
// Bit 0:   BG Display
// Bit 1:   Sprite Display Enable
// Bit 2:   Sprite Size (0=8x8, 1=8x16)
// Bit 3:   BG Tile Map Display Select 0=0x9800-0x9BFF, 1=0x9C00-0x9FFF)
// Bit 4:   BG and Window Tile Data Select (0=0x8800-0x97FF, 1=0x8000-0x8FFF)
// Bit 5:   Window Display Enable
// Bit 6:   Window Tile Map Display Select (0=0x9800-0x9BFF, 1=0x9C00-0x9FFF)
// Bit 7:   LCD Display Enable
#define MEM_LCDC                0xFF40

// STAT: LCD Status (R/W)
// Bit 0-1: Mode Flag (Read Only)
//      0b00: During H-Blank
//      0b01: During V-Blank
//      0b10: During Searching OAM-RAM
//      Ob11: Transferring data to LCD driver
// Bit 2:   Coincidence Flag (0:LYC<>LY, 1:LYC=LY) (Read Only)
// Bit 3:   Mode 0 H-Blank Interrupt
// Bit 4:   Mode 1 V-Blank Interrupt
// Bit 5:   Mode 2 OAM Interrupt
// Bit 6:   LYV=LY Coincidence Interrupt
// Bit 7:   Unused
#define MEM_LCD_STATUS          0xFF41

// SCY: Scroll Y (R/W)
// Y Position of the BG map to be displayed
#define MEM_LCD_SCROLL_Y        0xFF42

// SXC: Scroll X (R/W)
// X Position of the BG map to be displayed
#define MEM_LCD_SCROLL_X        0xFF43

// LYC: LCD Y Coordinate (R)
// Vertical line currently being transferred to LCD driver (0-153). Writing resets
#define MEM_LCD_Y               0xFF44

// LYC: LY Compare
#define MEM_LCD_YC              0xFF45

// WY: Window Y Position (R/W)
// Y Position of the window area
#define MEM_WY                  0xFF4A

// WX: Window X Position (R/W)
// X Position of the window area
#define MEM_WX                  0xFF4B

/* DMA Registers         */
// DMA: DMA Transfer and Start Address (W)
// Writing starts a DMA transfer
#define MEM_DMA                 0xFF46

/*  Timer Registers     */
// DIV: Divider Register (R/W)
// Incremented at a rate of 16384 Hz. 
// All writes reset to 0x0
#define MEM_DIVIDER             0xFF04

// TIMA: Timer Counter (R/W)
// Incremented by clock frequency specified in TAC
// When it overflows (>0xFF), reset to value in TMA
// and generate an interrupt
#define MEM_TIMA                0xFF05

// TMA: Timer Modulo (R/W)
// Set TIMA to this value when TIMA overflows
#define MEM_TMA                 0xFF06

// TAC: Timer Control (R/W)
// Bit 0-1  INULLnput Clock Select
//  0b00    4096 Hz
//  0b01    262144 Hz
//  0b10    65536 Hz
//  0b11    16384 Hz
// Bit 2:   Timer Stop (0=Stop, 1=Start)
#define MEM_TIMER_CONTROL       0xFF07

/*      Memory Regions      */
// Cartridge ROM
// Not banked
// 0x0000 - 0x3FFF
#define MEM_ROM                 0x0000

// Banked Cartridge ROM
// Banks can be changed
// 0x4000 - 0x7FFF
#define MEM_ROM_BANK            0x4000

// VRAM
// 0x8000 - 0x9FFF
#define MEM_VRAM                0x8000

// Background Tile Map Numbers
// Contains the numbers of the tiles to be displayed
#define MEM_VRAM_TILES          0x8000

// Backgound Tile Map 1
// 0x9800 - 0x9BFF
#define MEM_VRAM_MAP1           0x9800

// Backgound Tile Map 2
// 0x9800 - 0x9FFF
#define MEM_VRAM_MAP2           0x9C00

// External RAM
// 0xA000 - 0xBFFF
// Optional, sometimes in cartridge. Banked
#define MEM_RAM_EXTERNAL        0xA000

// Work RAM
// Two banks, 0xC000 - 0xCFFF and 0xD000 - 0xDFFF
#define MEM_RAM_INTERNAL        0xC000

// Echo RAM
// 0xE000 - 0xFDFF
// An echo of WRAM. Anything done here will be done in 
// WRAM at -0x2000 bytes 
#define MEM_RAM_ECHO            0xE000

// Sprite Attribute Table (OAM)
#define MEM_SPRITE_ATTR_TABLE   0xFE00

#define MEM_UNUSABLE            0xFEA0

// I/O Registers
#define MEM_IO_REGS             0xFF00

// High RAM 
#define MEM_HIGH_RAM            0xFF80

// Interrupts Enable Register (IE)
#define MEM_INT_EN_REG          0xFFFF

/*      IRQ Bits        */
#define IRQ_VBLANK              0x0001
#define IRQ_LCD_STAT            0x0002
#define IRQ_TIMER               0x0004
#define IRQ_SERIAL              0x00008
#define IRQ_JOYPAD              0x0010

/*      Jump Vectors    */
#define PC_START                0x0100
#define PC_VBLANK               0x0040
#define PC_LCD_STAT             0x0048
#define PC_TIMER                0x0050
#define PC_SERIAL               0x0058
#define PC_JOYPAD               0x0060

#define MEM_SOUND_NR10 0xFF10
#define MEM_SOUND_NR11 0xFF11
#define MEM_SOUND_NR12 0xFF12
#define MEM_SOUND_NR13 0xFF13
#define MEM_SOUND_NR14 0xFF14
#define MEM_SOUND_NR21 0xFF16
#define MEM_SOUND_NR22 0xFF17
#define MEM_SOUND_NR23 0xFF18
#define MEM_SOUND_NR24 0xFF19
#define MEM_SOUND_NR30 0xFF1A
#define MEM_SOUND_NR31 0xFF1B
#define MEM_SOUND_NR32 0xFF1C
#define MEM_SOUND_NR33 0xFF1D
#define MEM_SOUND_NR34 0xFF1E
#define MEM_SOUND_NR41 0xFF20
#define MEM_SOUND_NR42 0xFF21
#define MEM_SOUND_NR43 0xFF22
#define MEM_SOUND_NR44 0xFF23
#define MEM_SOUND_NR50 0xFF24
#define MEM_SOUND_NR51 0xFF25
#define MEM_SOUND_NR52 0xFF26

class Memory {
   public:
    void initMemory(const char* romName);

    void writeByte(uint16_t location, uint8_t data);
    void writeByteInternal(uint16_t location, uint8_t data, bool internal);
    uint8_t readByte(uint16_t location);

    void interrupt(const uint8_t flag);

    static void getTitle(char* title);

   protected:
   private:
    // Video RAM
    // Addr: MEM_VRAM
    uint8_t vram[0x2000];
    // Work RAM (both banks)
    // Addr: MEM_RAM_INTERNAL
    uint8_t wram[0x2000];
    // Sprite Attribute Table (OAM)
    // Addr: MEM_SPRITE_ATTR_TABLE
    uint8_t oam[0xA0];
    // I/O Registers
    // Addr: MEM_IO_REGS
    uint8_t ioreg[0x80];
    // High RAM
    // Addr: MEM_HIGH_RAM
    uint8_t hram[0x7F];
    // Interrupt Enable Register (IE)
    // Addr: MEM_INT_EN_REG
    uint8_t iereg;
    // Game Cart
    Cartridge* cart;
};
