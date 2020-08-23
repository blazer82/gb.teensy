#pragma once

#include <sys/_stdint.h>
#include "Cartridge.h"
#include <stdlib.h>
#include <Arduino.h>
#include <SD.h>
#include <SPI.h>

class NoMBC : public Cartridge{
    public:
        NoMBC(const char *romFile);
        uint8_t readByte(uint16_t addr);
        void writeByte(uint16_t addr, uint8_t data);
    private:
        uint8_t* rom;
        uint8_t* ram;
};

