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
#include "NoMBC.h"

#include <Arduino.h>
#include <SD.h>
#include <SPI.h>
#include <stdlib.h>

NoMBC::NoMBC(const char *romFile) : ACartridge(romFile) {
    // Allocate space for the ROM, always 2 banks
    rom = (uint8_t *)malloc(ROM_BANK_SIZE * 2 * sizeof(uint8_t));

    // Write the ROM data to memory
    Serial.println("Loading ROM into memory...");
    File dataFile = SD.open(romFile);
    if (dataFile) {
        for (uint16_t i = 0; i < ROM_BANK_SIZE * 2; i++) {
            rom[i] = dataFile.read();
        }
    } else {
        Serial.printf("Could not open rom file %s\n", romFile);
    }
    Serial.println();
    Serial.println("ROM Loaded!");
    // Allocate space for the RAM, if any
    if (ramSize != 0x0) {
        Serial.println("Initializing RAM...");
        ram = (uint8_t *)malloc(ramSize * sizeof(uint8_t));
        memset(ram, 0x0, ramSize);
        Serial.println("RAM Initialized!");
    }
}

NoMBC::~NoMBC() { Serial.println("Deleting NoMBC"); }

uint8_t NoMBC::readByte(uint16_t addr) {
    if (addr >= CART_RAM) {
        if (ramSize != 0) {
            return ram[addr - CART_RAM];
        } else {
            // TODO: Assume undefined RAM reads return 0xFF. Look this up
            return 0xFF;
        }
    } else {
        return rom[addr];
    }
}

void NoMBC::writeByte(uint16_t addr, uint8_t data) {
    // Handle writes to cartridge RAM
    if (addr >= CART_RAM) {
        // Make sure the RAM exists before we write to it
        if (ramSize != 0) {
            ram[(addr & (ramSize - 1)) - CART_RAM] = data;
            return;
        } else {
            return;
        }
    } else {
        return;
    }
}