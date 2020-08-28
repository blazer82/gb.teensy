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

#include "APU.h"

#include "Memory.h"

IntervalTimer APU::apuTimer1;
IntervalTimer APU::apuTimer2;

const uint8_t duty50 = 0x87;
volatile uint8_t i1 = 0;
volatile uint8_t i2 = 0;

volatile uint16_t currentSquare1Freq = 0;
volatile uint16_t currentSquare2Freq = 0;

void APU::begin() {
    pinMode(AUDIO_OUT1, OUTPUT);
    pinMode(AUDIO_OUT2, OUTPUT);

    APU::apuTimer1.begin(APU::timer1Step, 1000000);
    APU::apuTimer2.begin(APU::timer2Step, 1000000);
}

void APU::apuStep() {
    const uint16_t square1Freq = ((Memory::readByte(MEM_SOUND_NR14) && 0x3) << 8) | Memory::readByte(MEM_SOUND_NR13);
    const uint16_t square2Freq = ((Memory::readByte(MEM_SOUND_NR24) && 0x3) << 8) | Memory::readByte(MEM_SOUND_NR23);

    if (currentSquare1Freq != square1Freq) {
        APU::apuTimer1.update(1UL * 125000 / (uint32_t)square1Freq);
    }

    if (currentSquare2Freq != square2Freq) {
        APU::apuTimer2.update(1UL * 125000 / (uint32_t)square2Freq);
    }
}

void APU::timer1Step() {
    digitalWriteFast(AUDIO_OUT1, (duty50 >> i1) & 1);
    i1++;
    i1 %= 8;
}

void APU::timer2Step() {
    digitalWriteFast(AUDIO_OUT2, (duty50 >> i2) & 1);
    i2++;
    i2 %= 8;
}
