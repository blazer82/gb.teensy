/**
 * gb.teensy Emulation Software
 * Copyright (C) 2020  Raphael Stäbler, Grant Haack
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
#include "ACartridge.h"

class NoMBC : public ACartridge {
   public:
    NoMBC(const char* romFile);
    ~NoMBC();
    uint8_t readByte(uint16_t addr) override;
    void writeByte(uint16_t addr, uint8_t data) override;

   private:
    uint8_t* rom;
    uint8_t* ram;
};
