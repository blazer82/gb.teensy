
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

#include <Arduino.h>

#define JOYPAD_START  16
#define JOYPAD_SELECT 17
#define JOYPAD_LEFT   18
#define JOYPAD_RIGHT  19
#define JOYPAD_UP     20
#define JOYPAD_DOWN   21
#define JOYPAD_B      22
#define JOYPAD_A      23

class Joypad {
   public:
    static void begin();
    static void joypadStep();

   protected:
    static uint8_t previousValue;

    typedef union {
        struct {
            unsigned right : 1;
            unsigned left : 1;
            unsigned up : 1;
            unsigned down : 1;
            unsigned selectDirection : 1;
            unsigned selectButton : 1;
            unsigned : 2;
        } direction;
        struct {
            unsigned a : 1;
            unsigned b : 1;
            unsigned select : 1;
            unsigned start : 1;
            unsigned selectDirection : 1;
            unsigned selectButton : 1;
            unsigned : 2;
        } button;
        uint8_t value;
    } joypad_register_t;

   private:
};