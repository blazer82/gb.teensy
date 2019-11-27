#pragma once

#include <sys/_stdint.h>

#define MEM_IRQ_ENABLE 0xFFFF
#define MEM_IRQ_FLAG 0xFF0F
#define MEM_JOYPAD 0xFF00
#define MEM_LCDC 0xFF40
#define MEM_LCD_STATUS 0xFF41
#define MEM_LCD_SCROLL_Y 0xFF42
#define MEM_LCD_SCROLL_X 0xFF43
#define MEM_LCD_Y 0xFF44
#define MEM_DMA 0xFF46
#define MEM_DIVIDER 0xFF04
#define MEM_TIMA 0xFF05
#define MEM_TMA 0xFF06
#define MEM_TIMER_CONTROL 0xFF07
#define MEM_VRAM_MAP1 0x9800
#define MEM_VRAM_MAP2 0x9C00
#define MEM_VRAM_TILES 0x8000
#define MEM_RAM_ECHO 0xE000
#define MEM_RAM_INTERNAL 0xC000
#define MEM_ROM_BANK 0x4000
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

#define IRQ_VBLANK 0x01
#define IRQ_TIMER 0x04

#define PC_START 0x0100
#define PC_VBLANK 0x0040
#define PC_TIMER 0x0050

class Memory
{
    public:
        static uint8_t *memory;

        static void initMemory();

        static void writeByte(unsigned int location, uint8_t data);
        static void writeByteInternal(unsigned int location, uint8_t data, bool internal);
        static uint8_t readByte(unsigned int location);

        static void interrupt(uint8_t flag);
    protected:
    private:
};