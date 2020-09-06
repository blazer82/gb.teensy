#pragma once

#include <sys/_stdint.h>
#include "ACartridge.h"
#include <stdlib.h>
#include <Arduino.h>

// Control Register Addresses
#define MBC1_RAM_ENABLE_REG          0x0000
#define MBC1_PRIMARY_BANK_REG        0x2000
#define MBC1_SECONDARY_BANK_REG      0x4000
#define MBC1_BANKING_MODE_SEL_REG    0x6000

class MBC1 : public ACartridge{
    public:
        MBC1(const char *romFile);
        ~MBC1();
        uint8_t readByte(uint16_t addr) override;
        void writeByte(uint16_t addr, uint8_t data) override;
    private:
        // Enable/Disable the RAM
        uint8_t ramEnable;
        // The first 5 bits of ROM bank num selection
        uint8_t primaryBankBits;
        // RAM bank selection OR the next 2 bits of ROM bank num selection 
        uint8_t secondaryBankBits;
        // Select simple ROM banking or advanced ROM banking
        uint8_t bankModeSelect;

        // TODO: Allocate these in PSRAM
        // ROM banks
        uint8_t** romBanks;
        // RAM Banks 0x0 - 0x03
        uint8_t** ramBanks;
};

const uint8_t secondRomBankCount[]{
    0, 4, 8, 16, 32, 63, 124
};

const uint8_t firstRomBankCount[]{
    1, 1, 1, 1, 1, 1, 4
};