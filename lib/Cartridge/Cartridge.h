#pragma once

#include <sys/_stdint.h>
#include <avr/pgmspace.h>
#include <stdlib.h>
#include <Arduino.h>
#include <SD.h>
#include <SPI.h>

// Cartridge metadata
#define RAM_CODE            0x149
#define ROM_CODE            0x148
#define CART_CODE           0x147
#define CART_NAME           0x134

#define ROM_BANK_SIZE       0x4000

// Cartridge Memory Regions
#define CART_ROM_ZERO       0X0000 // Technically, this can also be banked
#define CART_ROM_BANKED     0x4000
#define CART_RAM            0xA000


class Cartridge {
    public:
        Cartridge(const char* romFile);
        virtual uint8_t readByte(uint16_t addr);
        virtual void writeByte(uint16_t addr, uint8_t data);        
        uint8_t getCartCode();
        uint8_t getRomCode();
        uint8_t getRamCode();
        char* getGameName();
    protected:
        // Metadata about the cart
        uint8_t cartCode;
        uint8_t romCode;
        uint8_t ramCode;

        // The cartridge type as a string
        const char* cartType;

        // The total size of all cartridge RAM
        uint32_t ramSize;
        // The size of a single bank of cartridge RAM
        uint16_t ramBankSize;
        // The total size of all cartridge ROM
        uint32_t romSize;

        // The total amount of RAM banks in the cartridge
        uint8_t ramBankCount;
        // The total amount of ROM banks in the cartridge
        uint16_t romBankCount;

        // Human readable name for the ROM
        char name[17];
};

uint16_t lookupRamBankSize(uint8_t code){
    switch(code){
        case 0x0:
            return 0x0;
        case 0x1:
            return 0x800;
        default:
            return 0x2000;
    }
}

uint32_t lookupRomSize(uint8_t code){
    switch(code){
        case 0x0:
            return 0x8000;
        case 0x1:
            return 0x20000;
        case 0x2:
            return 0x40000;
        case 0x3:
            return 0x80000;
        case 0x4:
            return 0x100000;
        case 0x5:
            return 0x200000;
        case 0x6:
            return 0x400000;
        case 0x7:
            return 0x800000;
        case 0x8:
            return 0x1000000;
        case 0x52:
            return 0x240000;
        case 0x53:
            return 0x280000;
        case 0x54:
            return 0x300000;
        default:
            return 0x8000;
    }
}

uint32_t lookupRamSize(uint8_t code){
    switch(code){
        case 0x0:
            return 0;
        case 0x1:
            return 0x800;
        case 0x2:
            return 0x2000;
        case 0x3:
            return 0x8000;
        case 0x4:
            return 0x20000;
        case 0x5:
            return 0x10000;
        default:
            return 0;
    }
}

uint16_t lookupRomBanks(uint8_t code){
    switch(code){
        case 0x0:
            return 1;
        case 0x1:
            return 4;
        case 0x2:
            return 8;
        case 0x3:
            return 16;
        case 0x4:
            return 32;
        case 0x5:
            return 64;
        case 0x6:
            return 128;
        case 0x7:
            return 256;
        case 0x8:
            return 512;
        case 0x52:
            return 72;
        case 0x53:
            return 80;
        case 0x54:
            return 96;
        default:
            return 1;
    }
}

uint8_t lookupRamBanks(uint8_t code){
    switch(code){
        case 0x0:
            return 0;
        case 0x1:
            return 1;
        case 0x2:
            return 1;
        case 0x3:
            return 4;
        case 0x4:
            return 16;
        case 0x5:
            return 8;
        default:
            return 0;
    }
}

const char unknown[] PROGMEM  = "UNKNOWN";
const char romOnly[] PROGMEM = "ROM ONLY";
const char mbc1[] PROGMEM = "MBC1";
const char mbc1Ram[] PROGMEM = "MBC1+RAM";
const char mbc1RamBatt[] PROGMEM = "MBC1+RAM+BATTERY";
const char mbc2[] PROGMEM = "MBC2";
const char mbc2Batt[] PROGMEM = "MBC2+BATTERY";
const char romRam[] PROGMEM = "ROM+RAM";
const char romRamBatt[] PROGMEM = "ROM+RAM+BATTERY";
const char mmm01[] PROGMEM = "MMM01";
const char mmm01Ram[] PROGMEM = "MMM01+RAM";
const char mmm01RamBatt[] PROGMEM = "MMM01+RAM+BATTERY";
const char mbc3TimerBatt[] PROGMEM = "MBC3+TIMER+BATTERY";
const char mbc3TimerRamBatt[] PROGMEM = "MBC3+TIMER+RAM+BATTERY";
const char mbc3[] PROGMEM = "MBC3";
const char mbc3Ram[] PROGMEM = "MBC3+RAM";
const char mbc3RamBatt[] PROGMEM = "MBC3+RAM+BATTERY";
const char mbc5[] PROGMEM = "MBC5";
const char mbc5Ram[] PROGMEM = "MBC5+RAM";
const char mbc5RamBatt[] PROGMEM = "MBC5+RAM+BATTERY";
const char mbc5Rumble[] PROGMEM = "MBC5+RUMBLE";
const char mbc5RumbleRam[] PROGMEM = "MBC5+RUMBLE+RAM";
const char mbc5RumbleRamBatt[] PROGMEM = "MBC5+RUMBLE+RAM+BATTERY";
const char mbc6[] PROGMEM = "MBC6";
const char mbc7SensorRumbleRamBatt[] PROGMEM = "MBC7+SENSOR+RUMBLE+RAM+BATTERY";
const char pocketCamera[] PROGMEM = "POCKET CAMERA";
const char bandaiTama5[] PROGMEM = "BANDAI TAMA5";
const char huc3[] PROGMEM = "HuC3";
const char huc1RamBatt[] PROGMEM = "HuC1+RAM+BATTERY";

const char* lookupCartType(uint8_t code){
    switch(code){
        case 0x00:
            return romOnly;
        case 0x01:
            return mbc1;
        case 0x2:
            return mbc1Ram;
        case 0x3:
            return mbc1RamBatt;
        case 0x5:
            return mbc2;
        case 0x6:
            return mbc2Batt;
        case 0x8:
            return romRam;
        case 0x9:
            return romRamBatt;
        case 0xb:
            return mmm01;
        case 0xc:
            return mmm01Ram;
        case 0xd:
            return mmm01RamBatt;
        case 0xf:
            return mbc3TimerBatt;
        case 0x10:
            return mbc3TimerRamBatt;
        case 0x11:
            return mbc3;
        case 0x12:
            return mbc3Ram;
        case 0x13:
            return mbc3RamBatt;
        case 0x19:
            return mbc5;
        case 0x1a:
            return mbc5Ram;
        case 0x1b:
            return mbc5RamBatt;
        case 0x1c:
            return mbc5Rumble;
        case 0x1d:
            return mbc5RumbleRam;
        case 0x1e:
            return mbc5RumbleRamBatt;
        case 0x20:
            return mbc6;
        case 0x22:
            return mbc7SensorRumbleRamBatt;
        case 0xfc:
            return pocketCamera;
        case 0xfd:
            return bandaiTama5;
        case 0xfe:
            return huc3;
        case 0xff:
            return huc1RamBatt;
        default:
            return unknown;
    }
}


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