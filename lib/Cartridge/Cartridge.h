#pragma once

#include <Arduino.h>
#include "ACartridge.h"

class Cartridge {
    public:
        static uint8_t begin(const char* romFile);
        static void writeByte(const uint16_t addr, const uint8_t data);
        static uint8_t readByte(const uint16_t addr);
    private:
        static ACartridge* cart;
};