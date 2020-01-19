/**
 * gb.teensy Emulation Software
 * Copyright (C) 2020  Raphael Stäbler
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 **/

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
