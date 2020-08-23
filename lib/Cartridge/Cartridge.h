#pragma once

#include <sys/_stdint.h>

// Cartridge metadata
#define RAM_CODE            0x149
#define ROM_CODE            0x148
#define CART_CODE           0x147
#define ROM_BANK_SIZE       0x3FFF

#define CART_ROM_ZERO       0X0000 // Technically, this can also be banked
#define CART_ROM_BANKED     0x4000
#define CART_RAM            0xA000


class Cartridge {
    public:
        Cartridge(const char* romFile);
        virtual uint8_t readByte(uint16_t addr);
        virtual void writeByte(uint16_t addr, uint8_t data);
        // This is here so everything will build until I can remove legacy
        // cartridge code
        static uint8_t cartridge[0x8000] ;
};


/**
 * MBC Notes
 * 
 * No MBC
 * 32KByte ROM only. Two 12KByte banks
 * 0x0000 - 0x3FFF  Bank0
 * 0x4000 - 0x7FFF  Bank1
 * 0xA000 - 0xBFFF  Optional 8KByte RAM
 * 
 * MBC1
 * 0x0000 - 0x3FFF  ROM Bank 0x00/0x20/0x40/0x60
 * 0x4000 - 0x7FFF  ROM Bank 0x01 - 0x7F
 * 0xA000 - 0xBFFF  Space for 8KByte External RAM. Can be 2KByte, 8KByte, or 32KByte (four 8KByte banks)
 * Control Registers
 * 0x0000 - 0x1FFF  RAM Enable (write anywhere in this range)
 *                      0x00 Disable RAM
 *                      0x0A Enable RAM 
 *                      Practically any value with 0x0A in lower 4 bits enables RAM
 *                      Any other value disables RAM
 * 0x2000 - 0x3FFF  ROM Bank Number (5 Bits)
 *                      Selects ROM bank. If bank number is set too high, value
 *                      is masked. Writing 0x0 writes a 0x01. 
 * 0x4000 - 0x5FFF  RAM Bank Number OR Upper bits of ROM Bank Number (2 bits)
 *                      If used for upper bits of ROM bank, (only effective on 1MB or larger ROM carts)
 *                          Bank = (Upper 2 Bits << 5) + ROM Bank
 *                      If upper bits used for RAM bank, (only effective on 32KB or larger RAM carts)
 *                          Selects RAM bank 0x00 - 0x03
 * 0x6000 - 0x7FFF  Banking Mode Select (1 bit)
 *                      Selects banking mode. Controls the 2 bit banking register behavior
 *                      0x00 Simple Banking Mode
 *                          Only effects 0x4000 - 0x7FFF banking area. RAM banking disabled
 *                      0x01 RAM Banking Mode / Advanced Banking Mode
 *                          If RAM > 8KB:
 *                              Enables RAM banking, switches RAM bank area to bank selected by 2 bit banking register
 *                          If ROM > 1MB:
 *                              Previously unbankable 0x0000 - 0x3000 is now bankable. Switches between bank 0x00, 0x10, 0x20, 0x30
 * Deal with Multi Game Compilation Carts later, can't be bothered
 * 
 * MBC2
 * 0x0000 - 0x3FFF  ROM Bank 0
 * 0x4000 - 0x7FFF  ROM Bank 0x01 - 0xF
 * 0xA000 - 0xA1FF  512x4bits RAM, built into MBC2 chip. 
 *                  Only lower 4 bits of the bytes in this area are used
 * Control Registers
 * 0x0000 - 0x1FFF RAM Enable
 *      LSb of upper address byte must be 0 to enable/disable RAM. 
 *          Ex: 0x0000 - 0x00FF, 0x0200 - 0x2FF, 0x0400 - 0x4FFF ... 0x1E00 - 0x1EFF
 * 0x2000 - 0x3FFF ROM Bank Number
 *      0xXXXXBBBB: X is don't care, B is bank select bit. Will select a ROM bank at 0x4000 - 0x7FFF
 *      LSb of upper address byte must be 1 to select ROM bank.
 *          Ex: 0x2100 - 0x21FF, 0x2300 - 0x23FF ... 0x3F00 -0x3FFF
 * */