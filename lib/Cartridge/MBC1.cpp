#include "MBC1.h"

MBC1::MBC1(const char *romFile) : Cartridge(romFile){
    // TODO: Open a ROM file from filesystem and parse it for these values
    // Right now just simulate the max values
    ramBankCount = 4;  // Simulate 32KB of RAM
    ramBankSize = 0x2000;
    
    romBankCount = 128;  // Simulate 2MB of ROM

    // Initialize the control registers
    ramEnable = 0x0;
    primaryBankBits = 0x1; // Defaults to bank 1 on PoR
    secondaryBankBits = 0x0;
    bankModeSelect = 0x0;

    // Allocate memory for ROM banks
    romBanks = (uint8_t**)malloc(romBankCount * sizeof(uint8_t *));
    for (uint8_t i = 0; i < romBankCount; i++){
        romBanks[i] = (uint8_t*)malloc(ROM_BANK_SIZE * sizeof(uint8_t));
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
            // If this is a large RAM cart, then use secondary bank bits as 
            // the RAM bank
            if(ramBankCount == 4){
                // Return data
                return ramBanks[secondaryBankBits][addr];
            }
            // If this is not a large RAM cart, then the secondary bank bits are
            // not used for RAM bank switching
            else{
                // Mask the address with the size of the RAM bank to 
                // prevent out of bounds reads. Some single bank MBC1 
                // carts only have 2K of RAM per bank. Large RAM carts 
                // are all 8K per bank
                addr = addr & (ramBankSize - 1);
                // Read data from the first and only bank
                return ramBanks[0][addr];
            }
        }
        else{
            // Assume that invalid reads return 0xFF. TODO Look this up.
            return 0xFF;
        }

    }
    // Handle reads from banked cartridge ROM
    else if(addr >= CART_ROM_BANKED_BOT){
        // If this is a small ROM cart, then we don't need to do any fancy legwork
        // with the secondary bank bits
        if(romBankCount <= 32){
            // Mask bank number with 1 - romBankCount to prevent out of bounds reads
            return romBanks[primaryBankBits & (romBankCount - 1)][addr];
        }
        // Large ROM carts use the secondary bank bits
        else{
            // Calculate the selected ROM bank using the bank bits
            uint8_t romBank = ((secondaryBankBits << 5) | primaryBankBits);
            // TODO: I don't know how the real MBC1 would handle attempts to read out
            // of bounds data. Masking doesn't really work for 72, 80, and 96 bank carts.
            // Right now I am subtracting to prevent out of bounds reads but I don't
            // know if this is accurate.
            if(romBank > romBankCount){
                romBank = romBank - romBankCount;
            }
            return romBanks[romBank][addr];

        }
    }
    // Handle reads from ROM bank zero
    // I know this is not called banked ROM, but technically it can be banked
    else if(addr >= CART_ROM_ZERO_BOT){
        // If this is a small ROM cart, then this region is not banked
        if(romBankCount <= 32){
            // Read back from bank 0
            return romBanks[0][addr];
        }
        // If this is a large ROM cart, then this region can be banked.
        // The secondary bank bits are shifted left five places to get the bank
        else{
            if (romBankCount < 128){
                // Make sure we don't read back from a bank that doesn't exist
                // If we're not using the upper bank bit, mask it out
                return romBanks[(secondaryBankBits & 0x1) << 5][addr];
            }
            else{
                // Otherwise, use it
                return romBanks[(secondaryBankBits) << 5][addr];
            }
            
        }
    }
    // MISRA
    else{
        Serial.printf("Attempted to read from invalid address 0x%x!\n\n", addr);
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
        // Writes of 0x0 default to 0x1
        if(data == 0x0){
            data = 0x1;
        }
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