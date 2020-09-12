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

#include <SD.h>

#include "CartHelpers.h"

class ACartridge {
   public:
    ACartridge(const char* romFile);
    ACartridge(const uint8_t* data);
    // Abstract readByte. It should be defined in every MBC
    virtual uint8_t readByte(uint16_t addr) = 0;
    // Abstract writeByte. It should be defined in every MBC
    virtual void writeByte(uint16_t addr, uint8_t data) = 0;
    virtual ~ACartridge();
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
    char name[16];
};
