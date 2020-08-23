#include "NoMBC.h"

NoMBC::NoMBC(const char *romFile) : Cartridge(romFile){
    // Allocate space for the ROM
    rom = (uint8_t*)malloc(0x7FFF * sizeof(uint8_t));

    // Allocate space for the RAM, if any
    if (ramSize != 0x0){
        ram = (uint8_t*)malloc(ramSize * sizeof(uint8_t));
    }
}

uint8_t NoMBC::readByte(uint16_t addr){
    if(addr >= CART_RAM){
        if(ramSize != 0){
            return ram[addr - CART_RAM];
        }
        else{
            // TODO: Assume undefined RAM reads return 0xFF. Look this up
            return 0xFF;
        }
    }
    else{
        return rom[addr];
    }
}

void NoMBC::writeByte(uint16_t addr, uint8_t data){
    // Handle writes to cartridge RAM
    if(addr >= CART_RAM){
        // Make sure the RAM exists before we write to it
        if(ramSize != 0){
            ram[(addr & (ramSize - 1)) - CART_RAM] = data;
            return;
        }
        else{
            return;
        }
    }
    else{
        return;
    }
}