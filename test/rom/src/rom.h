#pragma once

#include <stdint.h>

class ROM {
   public:
    static const uint8_t *getRom(int index) { return cpu_instrs; }
    static const uint8_t cpu_instrs[0x10000];
};
