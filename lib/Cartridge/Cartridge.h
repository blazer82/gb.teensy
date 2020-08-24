#pragma once

#include <Arduino.h>

#define GAME_TETRIS 1
// #define GAME_SUPER_MARIO 1
// #define TEST_CPU_INSTR 1
// #define TEST_SINGLE 1
// #define TEST_INSTR_TIMING 1

class Cartridge {
   public:
#if defined(GAME_TETRIS) || defined(TEST_INSTR_TIMING) || defined(TEST_SINGLE)
    static const uint8_t cartridge[0x8000] PROGMEM;
#endif
#if defined(GAME_SUPER_MARIO) || defined(TEST_CPU_INSTR)
    static const uint8_t cartridge[0x10000] PROGMEM;
#endif
};
