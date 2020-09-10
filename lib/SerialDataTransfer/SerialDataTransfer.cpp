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

#include "SerialDataTransfer.h"

#include "Memory.h"

void SerialDataTransfer::serialStep() {
    const uint8_t sc = Memory::readByte(MEM_SERIAL_SC);
    if ((sc & 0x81) == 0x81) {
        Serial.print((char)Memory::readByte(MEM_SERIAL_SB));
        Memory::writeByteInternal(MEM_SERIAL_SC, sc & 0x7F, true);
    }
}
