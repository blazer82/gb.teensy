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

const char noMbc[] PROGMEM = "NO MBC";
const char unknown[] PROGMEM  = "UNKNOWN";
const char romOnly[] PROGMEM = "ROM ONLY";
const char mbc1[] PROGMEM = "MBC1";
const char mbc1Ram[] PROGMEM = "MBC1+RAM";
const char mbc1RamBatt[] PROGMEM = "MBC1+RAM+BATTERY";
const char mbc2[] PROGMEM = "MBC2";
const char mbc2Batt[] PROGMEM = "MBC2+BATTERY";
const char romRam[] PROGMEM = "ROM+RAM";
const char romRamBatt[] PROGMEM = "ROM+RAM+BATTERY";
const char mmm01[] PROGMEM = "MMM01";
const char mmm01Ram[] PROGMEM = "MMM01+RAM";
const char mmm01RamBatt[] PROGMEM = "MMM01+RAM+BATTERY";
const char mbc3TimerBatt[] PROGMEM = "MBC3+TIMER+BATTERY";
const char mbc3TimerRamBatt[] PROGMEM = "MBC3+TIMER+RAM+BATTERY";
const char mbc3[] PROGMEM = "MBC3";
const char mbc3Ram[] PROGMEM = "MBC3+RAM";
const char mbc3RamBatt[] PROGMEM = "MBC3+RAM+BATTERY";
const char mbc5[] PROGMEM = "MBC5";
const char mbc5Ram[] PROGMEM = "MBC5+RAM";
const char mbc5RamBatt[] PROGMEM = "MBC5+RAM+BATTERY";
const char mbc5Rumble[] PROGMEM = "MBC5+RUMBLE";
const char mbc5RumbleRam[] PROGMEM = "MBC5+RUMBLE+RAM";
const char mbc5RumbleRamBatt[] PROGMEM = "MBC5+RUMBLE+RAM+BATTERY";
const char mbc6[] PROGMEM = "MBC6";
const char mbc7[] PROGMEM = "MBC7";
const char mbc7SensorRumbleRamBatt[] PROGMEM = "MBC7+SENSOR+RUMBLE+RAM+BATTERY";
const char pocketCamera[] PROGMEM = "POCKET CAMERA";
const char bandaiTama5[] PROGMEM = "BANDAI TAMA5";
const char huc3[] PROGMEM = "HuC3";
const char huc1[] PROGMEM = "HuC1";
const char huc1RamBatt[] PROGMEM = "HuC1+RAM+BATTERY";

uint8_t lookupMbcType(uint8_t code){
    switch(code){
        case 0x00:
            return USES_NOMBC;
        case 0x01:
            return USES_MBC1;
        case 0x2:
            return USES_MBC1;
        case 0x3:
            return USES_MBC1;
        case 0x5:
            return USES_MBC2;
        case 0x6:
            return USES_MBC2;
        case 0x8:
            return USES_NOMBC;
        case 0x9:
            return USES_NOMBC;
        case 0xb:
            return USES_MMM01;
        case 0xc:
            return USES_MMM01;
        case 0xd:
            return USES_MMM01;
        case 0xf:
            return USES_MBC3;
        case 0x10:
            return USES_MBC3;
        case 0x11:
            return USES_MBC3;
        case 0x12:
            return USES_MBC3;
        case 0x13:
            return USES_MBC3;
        case 0x19:
            return USES_MBC5;
        case 0x1a:
            return USES_MBC5;
        case 0x1b:
            return USES_MBC5;
        case 0x1c:
            return USES_MBC5;
        case 0x1d:
            return USES_MBC5;
        case 0x1e:
            return USES_MBC5;
        case 0x20:
            return USES_MBC6;
        case 0x22:
            return USES_MBC7;
        case 0xfc:
            return USES_POCKETCAM;
        case 0xfd:
            return USES_BANDAITAMA;
        case 0xfe:
            return USES_HUC3;
        case 0xff:
            return USES_HUC1;
        default:
            return USES_NOMBC;
    }
}

uint8_t lookupMbcTypeFromCart(const char* romFile){
    Serial.println("Initializing SD card...");
    uint8_t ret;
    // See if the card is present and can be initialized
    if (!SD.begin(BUILTIN_SDCARD)) {
        Serial.println("SD Card failed, or not present");
        return 0xFF;
    }
    File dataFile = SD.open(romFile);
    if (dataFile) {
        // Get the cartridge code
        dataFile.seek(CART_CODE);
        ret = lookupMbcType(dataFile.read());
    }
    else{
        Serial.println("Unable to read cartridge");
        ret = lookupMbcType(0x0);
    }
    dataFile.close();
    return ret;
}

uint16_t lookupRamBankSize(uint8_t code){
    switch(code){
        case 0x0:
            return 0x0;
        case 0x1:
            return 0x800;
        default:
            return 0x2000;
    }
}

uint32_t lookupRomSize(uint8_t code){
    switch(code){
        case 0x0:
            return 0x8000;
        case 0x1:
            return 0x20000;
        case 0x2:
            return 0x40000;
        case 0x3:
            return 0x80000;
        case 0x4:
            return 0x100000;
        case 0x5:
            return 0x200000;
        case 0x6:
            return 0x400000;
        case 0x7:
            return 0x800000;
        case 0x8:
            return 0x1000000;
        case 0x52:
            return 0x240000;
        case 0x53:
            return 0x280000;
        case 0x54:
            return 0x300000;
        default:
            return 0x8000;
    }
}

uint32_t lookupRamSize(uint8_t code){
    switch(code){
        case 0x0:
            return 0;
        case 0x1:
            return 0x800;
        case 0x2:
            return 0x2000;
        case 0x3:
            return 0x8000;
        case 0x4:
            return 0x20000;
        case 0x5:
            return 0x10000;
        default:
            return 0;
    }
}

uint16_t lookupRomBanks(uint8_t code){
    switch(code){
        case 0x0:
            return 1;
        case 0x1:
            return 4;
        case 0x2:
            return 8;
        case 0x3:
            return 16;
        case 0x4:
            return 32;
        case 0x5:
            return 64;
        case 0x6:
            return 128;
        case 0x7:
            return 256;
        case 0x8:
            return 512;
        case 0x52:
            return 72;
        case 0x53:
            return 80;
        case 0x54:
            return 96;
        default:
            return 1;
    }
}

uint8_t lookupRamBanks(uint8_t code){
    switch(code){
        case 0x0:
            return 0;
        case 0x1:
            return 1;
        case 0x2:
            return 1;
        case 0x3:
            return 4;
        case 0x4:
            return 16;
        case 0x5:
            return 8;
        default:
            return 0;
    }
}

const char* lookupCartType(uint8_t code){
    switch(code){
        case 0x00:
            return romOnly;
        case 0x01:
            return mbc1;
        case 0x2:
            return mbc1Ram;
        case 0x3:
            return mbc1RamBatt;
        case 0x5:
            return mbc2;
        case 0x6:
            return mbc2Batt;
        case 0x12:
            return mbc3Ram;
        case 0x13:
            return mbc3RamBatt;
        case 0x19:
            return mbc5;
        case 0x1a:
            return mbc5Ram;
        case 0x1b:
            return mbc5RamBatt;
        case 0x1c:
            return mbc5Rumble;
        case 0x1d:
            return mbc5RumbleRam;
        case 0x1e:
            return mbc5RumbleRamBatt;
        case 0x20:
            return mbc6;
        case 0x22:
            return mbc7SensorRumbleRamBatt;
        case 0xfc:
            return pocketCamera;
        case 0xfd:
            return bandaiTama5;
        case 0xfe:
            return huc3;
        case 0xff:
            return huc1RamBatt;
        default:
            return unknown;
    }
}

const char* lookupMBCTypeString(uint8_t code){
    switch(code){
        case 0x00:
            return noMbc;
        case 0x01:
            return mbc1;
        case 0x2:
            return mbc1;
        case 0x3:
            return mbc1;
        case 0x5:
            return mbc2;
        case 0x6:
            return mbc2;
        case 0x8:
            return noMbc;
        case 0x9:
            return noMbc;
        case 0xb:
            return mmm01;
        case 0xc:
            return mmm01;
        case 0xd:
            return mmm01;
        case 0xf:
            return mbc3;
        case 0x10:
            return mbc3;
        case 0x11:
            return mbc3;
        case 0x12:
            return mbc3;
        case 0x13:
            return mbc3;
        case 0x19:
            return mbc5;
        case 0x1a:
            return mbc5;
        case 0x1b:
            return mbc5;
        case 0x1c:
            return mbc5;
        case 0x1d:
            return mbc5;
        case 0x1e:
            return mbc5;
        case 0x20:
            return mbc6;
        case 0x22:
            return mbc7;
        case 0xfc:
            return pocketCamera;
        case 0xfd:
            return bandaiTama5;
        case 0xfe:
            return huc3;
        case 0xff:
            return huc1;
        default:
            return unknown;
    }
}
