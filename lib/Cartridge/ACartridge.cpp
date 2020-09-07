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
#include <stdlib.h>
#include <Arduino.h>
#include <SD.h>
#include <SPI.h>
#include "ACartridge.h"

ACartridge::ACartridge(const char* romFile){
    Serial.println("Initializing SD card...");
    // See if the card is present and can be initialized
    if (!SD.begin(BUILTIN_SDCARD)) {
        Serial.println("SD Card failed, or not present");
        return;
    }
    Serial.println("SD Card Initialized!");
    

    dataFile = SD.open(romFile);
    if (dataFile) {
        // Get the cartridge code
        dataFile.seek(CART_CODE);
        cartCode = dataFile.read();
        cartType = lookupCartType(cartCode);
        // Get the amount of ROM in the cart
        romCode = dataFile.read();
        romBankCount = lookupRomBanks(romCode);
        romSize = lookupRomSize(romSize);
        // Get the amount of RAM in the car

        ramCode = dataFile.read();
        ramBankCount = lookupRamBanks(ramCode);
        ramBankSize = lookupRamBankSize(ramCode);
        ramSize = lookupRamSize(ramCode);
        // Get the name of the cart
        dataFile.seek(CART_NAME);
        for(uint8_t i = 0; i < 16; i++){
            name[i] = dataFile.read();
        }
        Serial.println("Cartridge Info:");
        Serial.println("----------");
        Serial.printf("Game Name: %s\n", name);
        Serial.printf("\tCart Code: 0x%x\n", cartCode);
        Serial.printf("\tCart Type: %s\n", cartType);
        Serial.printf("\tMemory Bank Controller: %s\n", lookupMBCTypeString(cartCode));
        Serial.println("ROM Info:");
        Serial.printf("\tROM Code: 0x%x\n", romCode);
        Serial.printf("\tROM Banks: %i\n", romBankCount);
        Serial.printf("\tROM Size: 0x%x\n", romSize);
        Serial.println("RAM Info:");
        Serial.printf("\tRAM Code: 0x%x\n", ramCode);
        Serial.printf("\tRAM Banks: %i\n", ramBankCount);
        Serial.printf("\tRAM Size: 0x%x\n", ramSize);
        Serial.printf("\tRAM Bank Size: 0x%x\n", ramBankSize);
    }
    else{
        Serial.println("Unable to read cartridge");
    }

}

ACartridge::~ACartridge(){
    Serial.println("Deleting Cartridge");
}

uint8_t ACartridge::getCartCode(){
    return cartCode;
}

uint8_t ACartridge::getRomCode(){
    return romCode;
}

uint8_t ACartridge::getRamCode(){
    return ramCode;
}

char* ACartridge::getGameName(){
    return name;
}
