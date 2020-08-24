#pragma once

#include <sys/_stdint.h>
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

// Defines I made to differentiate memory bank controllers
#define USES_NOMBC          0x0
#define USES_MBC1           0x1
#define USES_MBC2           0x2
#define USES_MMM01          0x3
#define USES_MBC3           0x4
#define USES_MBC5           0x5
#define USES_MBC6           0x6
#define USES_MBC7           0x7
#define USES_POCKETCAM      0x8
#define USES_BANDAITAMA     0x9
#define USES_HUC3           0xA
#define USES_HUC1           0xB

class Cartridge {
    public:
        Cartridge(const char* romFile);
        uint8_t readByte(uint16_t addr);
        void writeByte(uint16_t addr, uint8_t data);
        uint8_t getCartCode();
        uint8_t getRomCode();
        uint8_t getRamCode();
        char* getGameName();
    protected:
        // Metadata about the cart
        uint8_t cartCode;
        uint8_t romCode;
        uint8_t ramCode;

        // The file object used to read the ROM file off the SD card
        File dataFile;

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

uint8_t lookupMbcType(uint8_t code);
uint8_t lookupMbcTypeFromCart(const char* romFile);
uint16_t lookupRamBankSize(uint8_t code);
uint32_t lookupRomSize(uint8_t code);
uint32_t lookupRamSize(uint8_t code);
uint16_t lookupRomBanks(uint8_t code);
uint8_t lookupRamBanks(uint8_t code);
const char* lookupCartType(uint8_t code);
const char* lookupMBCTypeString(uint8_t code);