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

#include "Joypad.h"

#include "Memory.h"

void Joypad::begin() {
    pinMode(JOYPAD_START, INPUT);
    pinMode(JOYPAD_SELECT, INPUT);
    pinMode(JOYPAD_LEFT, INPUT);
    pinMode(JOYPAD_RIGHT, INPUT);
    pinMode(JOYPAD_UP, INPUT);
    pinMode(JOYPAD_DOWN, INPUT);
    pinMode(JOYPAD_B, INPUT);
    pinMode(JOYPAD_A, INPUT);
}

void Joypad::joypadStep() {
    uint8_t joypad = Memory::readByte(MEM_JOYPAD);

    if ((joypad & 0x10) == 0) {
        bool left = digitalReadFast(JOYPAD_LEFT);
        bool right = digitalReadFast(JOYPAD_RIGHT);
        bool up = digitalReadFast(JOYPAD_UP);
        bool down = digitalReadFast(JOYPAD_DOWN);
        joypad = (joypad & 0xF0) | (down << 3) | (up << 2) | (left << 1) | right;
        Memory::writeByteInternal(MEM_JOYPAD, joypad, true);
    }

    if ((joypad & 0x20) == 0) {
        bool start = digitalReadFast(JOYPAD_START);
        bool select = digitalReadFast(JOYPAD_SELECT);
        bool a = digitalReadFast(JOYPAD_A);
        bool b = digitalReadFast(JOYPAD_B);
        joypad = (joypad & 0xF0) | (start << 3) | (select << 2) | (b << 1) | a;
        Memory::writeByteInternal(MEM_JOYPAD, joypad, true);
    }
}