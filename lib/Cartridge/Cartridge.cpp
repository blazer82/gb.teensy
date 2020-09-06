#include "Cartridge.h"
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