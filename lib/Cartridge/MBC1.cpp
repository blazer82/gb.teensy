#include "MBC1.h"

MBC1::MBC1(const char *romFile) : Cartridge(romFile){
    // TODO: Open a ROM file from filesystem and parse it for these values
    // Right now just simulate the max values
    ramBankCount = 4;  // Simulate 32KB of RAM
    ramBankSize = 0x2000;
    
    secondaryRomBankCount = 124;  // Simulate 2MB of ROM
    firstRomBankCount = 4;

    // Initialize the control registers
    ramEnable = 0x0;
    primaryBankBits = 0x1;
    secondaryBankBits = 0x0;
    bankModeSelect = 0x0;

    // Allocate memory for the second ROM banks
    secondaryRomBanks = (uint8_t**)malloc(secondaryRomBankCount * sizeof(uint8_t *));
    for (uint8_t i = 0; i < secondaryRomBankCount; i++){
        secondaryRomBanks[i] = (uint8_t*)malloc(ROM_BANK_SIZE * sizeof(uint8_t));
    }

    // Allocate memory for the first ROM banks
    firstRomBanks = (uint8_t**)malloc(firstRomBankCount * sizeof(uint8_t *));
    for (uint8_t i = 0; i < firstRomBankCount; i++){
        firstRomBanks[i] = (uint8_t*)malloc(ROM_BANK_SIZE * sizeof(uint8_t));
    }

    // Allocate memory for the RAM banks
    ramBanks = (uint8_t**)malloc(ramBankCount * sizeof(uint8_t *));
    for (uint8_t i = 0; i < ramBankCount; i++){
        ramBanks[i] = (uint8_t*)malloc(ramBankSize * sizeof(uint8_t));
    }
}

uint8_t MBC1::readByte(uint16_t addr){
    // Handle reads from RAM
    if(addr >= CART_RAM_BOT){
        // Mame sure RAM is enabled and exists
        if(ramEnable && ramBankCount != 0){
            // Mask the address with the size of a RAM bank
            addr = addr & (ramBankSize - 1);
            // If this is a large RAM cart, then use secondary bank bits as 
            // the RAM bank
            if(ramBankCount > 1){
                // Return data
                return ramBanks[secondaryBankBits][addr];
            }
            // If this is not a large RAM cart, then the secondary bank bits are
            // not used for RAM bank switching
            else{
                // Read data from the first and only bank
                return ramBanks[0][addr];
            }
        }
        else{
            // Assume that invalid writes return 0xFF. Look this up.
            return 0xFF;
        }

    }
    // Handle reads from banked cartridge ROM
    else if(addr >= CART_ROM_BANKED_BOT){

    }
}

void MBC1::writeByte(uint16_t addr, uint8_t data){
    // Handle writes to RAM
    if(addr >= CART_RAM_BOT){
        // Make sure RAM is enabled and it exists
        if(ramEnable && ramBankCount != 0){
            // Mask the address with the size of a RAM bank
            addr = addr & (ramBankSize - 1);
            // If this is a large RAM cart, then use secondary bank bits as 
            // the RAM bank
            if(ramBankCount > 1){
                // Write the data
                ramBanks[secondaryBankBits][addr] = data;
                return;
            }
            // If this is not a large RAM cart, then the secondary bank bits are
            // not used for RAM bank switching
            else{
                // Write the data to the first and only bank
                ramBanks[0][addr] = data;
                return;
            }
        }
        else{
            return;
        }
    }
    // Handle writes to control registers
    // Manipulate the bank mode select register
    else if(addr >= BANKING_MODE_SEL_BOT){
        bankModeSelect = data & 0x1;
        return;
    }
    // Manipulate the secondary bank bits control register
    else if(addr >= SECONDARY_BANK_BOT){
        secondaryBankBits = data & 0x3;
        return;
    }
    // Manipulate primary bank bits control register
    else if(addr >= PRIMARY_BANK_BOT){
        primaryBankBits = data & 0x1F;
        return;
    }
    // Manipulate RAM enable control register
    if(addr >= RAM_ENABLE_BOT){
        // If 0xA is in the lower 4 bits, enable RAM
        if ((data & 0xF) == 0xA){
            ramEnable = 1;
        }
        else{
            ramEnable = 0;
        }
        return;
    }
    // MISRA
    else{
        Serial.printf("Attempted to write to invalid address 0x%x!\n\n", addr);
    }
}