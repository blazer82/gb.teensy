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

#include <Arduino.h>

#include "ACartridge.h"

// Control Register Addresses
#define MBC2_RAM_ENABLE_REG       0x0000
#define MBC2_RAM_ENABLE_REG_TOP   0x1FFF
#define MBC2_PRIMARY_BANK_REG     0x2000
#define MBC2_PRIMARY_BANK_REG_TOP 0x4000
// MBC2 Has significantly less RAM than can fit in the memory map
#define MBC2_CART_RAM_TOP 0xA200

class MBC2 : public ACartridge {
   public:
    MBC2(const char* romFile);
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