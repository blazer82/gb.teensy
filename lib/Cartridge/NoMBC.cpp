#include "NoMBC.h"

NoMBC::NoMBC(const char *romFile) : Cartridge(romFile){
    // Allocate space for the ROM, always 2 banks
    rom = (uint8_t*)malloc(ROM_BANK_SIZE * 2 * sizeof(uint8_t));

    // Write the ROM data to memory
    Serial.printf("Loading %s into memory...\n", romFile);
    File dataFile = SD.open(romFile);
    if (dataFile) {
        for(uint16_t i = 0; i < ROM_BANK_SIZE * 2; i++){
            rom[i] = dataFile.read();
            // if((i % 32) == 0){
            //     Serial.printf("\n0x%04x: ", i);
            // }
            // Serial.printf("0x%02x ", rom[i]);
        }
    }
    else{
        Serial.printf("Could not open rom file %s\n", romFile);
    }
    Serial.println();
    Serial.println("ROM Loaded!");
    // Allocate space for the RAM, if any
    if (ramSize != 0x0){
        Serial.println("Initializing RAM...");
        ram = (uint8_t*)malloc(ramSize * sizeof(uint8_t));
        memset(ram, 0x0, ramSize);
        Serial.println("RAM Initialized!");
    }
}

NoMBC::~NoMBC(){
    Serial.println("Deleting NoMBC");
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