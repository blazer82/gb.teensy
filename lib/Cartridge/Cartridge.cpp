#include "Cartridge.h"

Cartridge::Cartridge(const char* romFile){
    Serial.println("Initializing SD card...");
    // See if the card is present and can be initialized
    if (!SD.begin(BUILTIN_SDCARD)) {
        Serial.println("SD Card failed, or not present");
        return;
    }

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
        // Get the amount of RAM in the cart
        ramCode = dataFile.read();
        ramBankCount = lookupRamBanks(ramCode);
        ramBankSize = lookupRamBankSize(ramCode);
        ramSize = lookupRamSize(ramCode);
        // Get the name of the cart
        dataFile.seek(CART_NAME);
        for(uint8_t i = 0; i < 16; i++){
            name[i] = dataFile.read();
        }
        name[16] = 0;
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

uint8_t Cartridge::getCartCode(){
    return cartCode;
}

uint8_t Cartridge::getRomCode(){
    return romCode;
}

uint8_t Cartridge::getRamCode(){
    return ramCode;
}

char* Cartridge::getGameName(){
    return name;
}