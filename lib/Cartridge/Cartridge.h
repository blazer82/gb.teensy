#pragma once

#include <sys/_stdint.h>

#define GAME_TETRIS 1
// #define GAME_SUPER_MARIO 1

class Cartridge {
   public:
#ifdef GAME_TETRIS
    static const uint8_t cartridge[0x8000];
#endif
#ifdef GAME_SUPER_MARIO
    static const uint8_t cartridge[0x10000];
#endif
};
