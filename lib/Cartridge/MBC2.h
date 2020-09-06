#pragma once

#include <sys/_stdint.h>
#include "ACartridge.h"
#include <stdlib.h>
#include <Arduino.h>

// Control Register Addresses
#define MBC2_RAM_ENABLE_REG          0x0000
#define MBC2_RAM_ENABLE_REG_TOP      0x1FFF
#define MBC2_PRIMARY_BANK_REG        0x2000
#define MBC2_PRIMARY_BANK_REG_TOP    0x4000


// MBC2 Has significantly less RAM than can fit in the memory map
#define MBC2_CART_RAM_TOP            0xA200

class MBC2 : public ACartridge{
    public:
        MBC2(const char *romFile);
        ~MBC2();
        uint8_t readByte(uint16_t addr) override;
        void writeByte(uint16_t addr, uint8_t data) override;
    private:
        // Enable/Disable the RAM
        uint8_t ramEnable;
        // Select the ROM bank, 0x0 - 0x0F
        uint8_t romBankSelect;
        // The amount of ROM banks in the cart
        uint8_t romBankCount;

        // TODO: Allocate these in PSRAM
        // ROM banks
        uint8_t** romBanks;
        // The RAM bank
        uint8_t* ramBank;
};