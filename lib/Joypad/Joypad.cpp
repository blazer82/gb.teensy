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
    pinMode(JOYPAD_START, INPUT_PULLUP);
    pinMode(JOYPAD_SELECT, INPUT_PULLUP);
    pinMode(JOYPAD_LEFT, INPUT_PULLUP);
    pinMode(JOYPAD_RIGHT, INPUT_PULLUP);
    pinMode(JOYPAD_UP, INPUT_PULLUP);
    pinMode(JOYPAD_DOWN, INPUT_PULLUP);
    pinMode(JOYPAD_B, INPUT_PULLUP);
    pinMode(JOYPAD_A, INPUT_PULLUP);
}

void Joypad::joypadStep() {
    joypad_register_t joypad = {.value = Memory::readByte(MEM_JOYPAD)};

    if (joypad.direction.selectDirection == 0) {
        joypad.direction.left = digitalReadFast(JOYPAD_LEFT);
        joypad.direction.right = digitalReadFast(JOYPAD_RIGHT);
        joypad.direction.up = digitalReadFast(JOYPAD_UP);
        joypad.direction.down = digitalReadFast(JOYPAD_DOWN);
        Memory::writeByteInternal(MEM_JOYPAD, joypad.value, true);
    }

    if (joypad.button.selectButton == 0) {
        joypad.button.start = digitalReadFast(JOYPAD_START);
        joypad.button.select = digitalReadFast(JOYPAD_SELECT);
        joypad.button.a = digitalReadFast(JOYPAD_A);
        joypad.button.b = digitalReadFast(JOYPAD_B);
        Memory::writeByteInternal(MEM_JOYPAD, joypad.value, true);
    }
}