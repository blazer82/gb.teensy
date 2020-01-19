/**
 * This file is part of the gb.teensy emulator
 * <https://github.com/blazer82/gb.teensy>.
 * 
 * This is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 **/

#pragma once

#include <SPI.h>
#include <ILI9341_t3.h>

class Display : public ILI9341_t3
{
    public:
        Display(uint8_t _CS = 14, uint8_t _DC = 5, uint8_t _RST = 255, uint8_t _MOSI=11, uint8_t _SCLK=13, uint8_t _MISO=12) : ILI9341_t3(_CS, _DC, _RST, _MOSI, _SCLK, _MISO) {};
        void hLine(uint16_t y, uint16_t x, uint16_t *lineData);
    protected:
    private:
};