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

const uint8_t duty[] = {0x01, 0x81, 0x87, 0x7E};
volatile uint8_t i1 = 0;
volatile uint8_t i2 = 0;

volatile uint16_t currentSquare1Freq = 0;
volatile uint16_t currentSquare2Freq = 0;

void APU::begin() {
    pinMode(AUDIO_OUT1, OUTPUT);
    pinMode(AUDIO_OUT2, OUTPUT);

    // Setup PWM resolution and frequency according to https://www.pjrc.com/teensy/td_pulse.html
    analogWriteResolution(4);
    analogWriteFrequency(AUDIO_OUT1, 9375000);
    analogWriteFrequency(AUDIO_OUT2, 9375000);

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
    const uint8_t dutyIndex = Memory::readByte(MEM_SOUND_NR11) >> 6;
    analogWrite(AUDIO_OUT1, ((duty[dutyIndex] >> i1) & 1) * 0xF);
    i1++;
    i1 %= 8;
}

void APU::timer2Step() {
    const uint8_t dutyIndex = Memory::readByte(MEM_SOUND_NR11) >> 6;
    analogWrite(AUDIO_OUT2, ((duty[dutyIndex] >> i2) & 1) * 0xF);
    i2++;
    i2 %= 8;
}
