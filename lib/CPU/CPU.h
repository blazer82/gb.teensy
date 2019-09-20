#pragma once

#include <sys/_stdint.h>

class CPU
{
    public:
        static volatile bool cpuEnabled;
        static volatile uint64_t totalCycles;

        static void cpuStep();
    protected:
        static uint8_t readOp();
        static uint16_t readNn();
        static void pushStack(uint16_t data);
        static uint16_t popStack();
    private:
};
