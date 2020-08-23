#pragma once

#include <sys/_stdint.h>
#include "Cartridge.h"
#include <stdlib.h>
#include <Arduino.h>

// Control Register Addresses
#define RAM_ENABLE_REG          0x0000
#define PRIMARY_BANK_REG        0x2000
#define SECONDARY_BANK_REG      0x4000
#define BANKING_MODE_SEL_REG    0x6000

class MBC1 : public Cartridge{
    public:
        MBC1(const char *romFile);
        uint8_t readByte(uint16_t addr);
        void writeByte(uint16_t addr, uint8_t data);
    private:
        // Enable/Disable the RAM
        uint8_t ramEnable;
        // The first 5 bits of ROM bank num selection
        uint8_t primaryBankBits;
        // RAM bank selection OR the next 2 bits of ROM bank num selection 
        uint8_t secondaryBankBits;
        // Select simple ROM banking or advanced ROM banking
        uint8_t bankModeSelect;

        // ROM banks
        uint8_t** romBanks;
        // RAM Banks 0x0 - 0x03
        uint8_t** ramBanks;

        // The total amount of ROM banks
        uint8_t romBankCount;
        // The total amount of RAM banks
        uint8_t ramBankCount;
        // The total size of each individual RAM bank (not all are 8K)
        uint16_t ramBankSize;
};

const uint8_t secondRomBankCount[]{
    0, 4, 8, 16, 32, 63, 124
};

const uint8_t firstRomBankCount[]{
    1, 1, 1, 1, 1, 1, 4
};