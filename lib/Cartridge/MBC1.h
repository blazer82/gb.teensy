/**
 * gb.teensy Emulation Software
 * Copyright (C) 2020  Raphael Stäbler, Grant Haack
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
#include "ACartridge.h"

// Control Register Addresses
#define MBC1_RAM_ENABLE_REG 0x0000
#define MBC1_PRIMARY_BANK_REG 0x2000
#define MBC1_SECONDARY_BANK_REG 0x4000
#define MBC1_BANKING_MODE_SEL_REG 0x6000

class MBC1 : public ACartridge {
   public:
    MBC1(const char* romFile);
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