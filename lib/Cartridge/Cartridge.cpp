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
#include <Arduino.h>
#include "Cartridge.h"
#include "CartHelpers.h"
#include "NoMBC.h"
#include "MBC1.h"
#include "MBC2.h"

ACartridge* Cartridge::cart = 0; 

uint8_t Cartridge::begin(const char* romFile){
    uint8_t mbcType = lookupMbcTypeFromCart(romFile);
    if (mbcType == USES_NOMBC){
        cart = new NoMBC(romFile);
    }
    else if(mbcType == USES_MBC1){
        cart = new MBC1(romFile);
    }
    else if(mbcType == USES_MBC2){
        cart = new MBC2(romFile);
    }
    else{
        Serial.printf("MBC type 0x%x is currently not supported\n");
        return 1;
    }
    return 0;
}

void Cartridge::writeByte(const uint16_t addr, const uint8_t data){
    cart->writeByte(addr, data);
}
uint8_t Cartridge::readByte(const uint16_t addr){
    return cart->readByte(addr);
}

void Cartridge::getGameName(char* buf){
    char* name;
    name = cart->getGameName();
    memcpy(buf, name, 16);
    buf[16] = 0; // Null terminate the string
}