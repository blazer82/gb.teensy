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
#include <sys/_stdint.h>

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

uint8_t lookupMbcType(uint8_t code);
uint8_t lookupMbcTypeFromCart(const char* romFile);
uint16_t lookupRamBankSize(uint8_t code);
uint32_t lookupRomSize(uint8_t code);
uint32_t lookupRamSize(uint8_t code);
uint16_t lookupRomBanks(uint8_t code);
uint8_t lookupRamBanks(uint8_t code);
const char* lookupCartType(uint8_t code);
const char* lookupMBCTypeString(uint8_t code);