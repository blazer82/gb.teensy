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
IntervalTimer APU::lengthTimer;
IntervalTimer APU::envelopeTimer;

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
    APU::lengthTimer.begin(APU::lengthUpdate, 3096);
    APU::envelopeTimer.begin(APU::envelopeUpdate, 15630);
}

void APU::apuStep() {
    const uint16_t square1Freq = ((Memory::readByte(MEM_SOUND_NR14) && 0x3) << 8) | Memory::readByte(MEM_SOUND_NR13);
    const uint16_t square2Freq = ((Memory::readByte(MEM_SOUND_NR24) && 0x3) << 8) | Memory::readByte(MEM_SOUND_NR23);

    if (currentSquare1Freq != square1Freq) {
        currentSquare1Freq = square1Freq;

        if (square1Freq == 0) {
            APU::apuTimer1.update(1000000);
            analogWrite(AUDIO_OUT1, 0);
        } else {
            APU::apuTimer1.update(1UL * 125000 / (uint32_t)square1Freq);
        }
    }

    if (currentSquare2Freq != square2Freq) {
        currentSquare2Freq = square2Freq;

        if (square2Freq == 0) {
            APU::apuTimer1.update(1000000);
            analogWrite(AUDIO_OUT2, 0);
        } else {
            APU::apuTimer2.update(1UL * 125000 / (uint32_t)square2Freq);
        }
    }
}

void APU::timer1Step() {
    const bool lengthEnable = (Memory::readByte(MEM_SOUND_NR14) & 0x40) != 0;
    const uint8_t length = lengthEnable ? Memory::readByte(MEM_SOUND_NR11) & 0x3F : 1;

    if (length > 0) {
        const uint8_t dutyIndex = Memory::readByte(MEM_SOUND_NR11) >> 6;
        const uint8_t envelopeVolume = Memory::readByte(MEM_SOUND_NR12) >> 4;
        const uint8_t mixerVolume = (Memory::readByte(MEM_SOUND_NR50) & 0x7) * (Memory::readByte(MEM_SOUND_NR51) & 0x1) +
                                    (Memory::readByte(MEM_SOUND_NR50) >> 4) * (Memory::readByte(MEM_SOUND_NR51) & 0x10);
        analogWrite(AUDIO_OUT1, ((duty[dutyIndex] >> i1) & 1) * 2 * envelopeVolume * mixerVolume);
        i1++;
        i1 %= 8;
    } else {
        analogWrite(AUDIO_OUT1, 0);
    }
}

void APU::timer2Step() {
    const bool lengthEnable = (Memory::readByte(MEM_SOUND_NR24) & 0x40) != 0;
    const uint8_t length = lengthEnable ? Memory::readByte(MEM_SOUND_NR21) & 0x3F : 1;

    if (length > 0) {
        const uint8_t dutyIndex = Memory::readByte(MEM_SOUND_NR21) >> 6;
        const uint8_t envelopeVolume = Memory::readByte(MEM_SOUND_NR22) >> 4;
        const uint8_t mixerVolume = (Memory::readByte(MEM_SOUND_NR50) & 0x7) * (Memory::readByte(MEM_SOUND_NR51) & 0x2) +
                                    (Memory::readByte(MEM_SOUND_NR50) >> 4) * (Memory::readByte(MEM_SOUND_NR51) & 0x20);
        analogWrite(AUDIO_OUT2, ((duty[dutyIndex] >> i2) & 1) * 2 * envelopeVolume * mixerVolume);
        i2++;
        i2 %= 8;
    } else {
        analogWrite(AUDIO_OUT2, 0);
    }
}

void APU::lengthUpdate() {
    const bool lengthEnable1 = (Memory::readByte(MEM_SOUND_NR14) & 0x40) != 0;
    const bool lengthEnable2 = (Memory::readByte(MEM_SOUND_NR24) & 0x40) != 0;

    if (lengthEnable1) {
        const uint8_t length = Memory::readByte(MEM_SOUND_NR11) & 0x3F;
        if (length > 0) {
            Memory::writeByteInternal(MEM_SOUND_NR11, (Memory::readByte(MEM_SOUND_NR11) & 0xC0) | (length - 1), true);
        }
    }

    if (lengthEnable2) {
        const uint8_t length = Memory::readByte(MEM_SOUND_NR21) & 0x3F;
        if (length > 0) {
            Memory::writeByteInternal(MEM_SOUND_NR21, (Memory::readByte(MEM_SOUND_NR21) & 0xC0) | (length - 1), true);
        }
    }
}

void APU::envelopeUpdate() {
    const uint8_t envelope1 = Memory::readByte(MEM_SOUND_NR12);
    const uint8_t volume1 = envelope1 >> 4;
    const uint8_t increase1 = (envelope1 & 0x8) != 0;
    const uint8_t number1 = envelope1 & 0x7;

    const uint8_t envelope2 = Memory::readByte(MEM_SOUND_NR22);
    const uint8_t volume2 = envelope2 >> 4;
    const uint8_t increase2 = (envelope2 & 0x8) != 0;
    const uint8_t number2 = envelope2 & 0x7;

    if (number1 > 0) {
        if (increase1 && volume1 < 0xF) {
            Memory::writeByteInternal(MEM_SOUND_NR12, ((volume1 + 1) << 4) | (increase1 << 3) | (number1 - 1), true);
        } else if (!increase1 && volume1 > 0) {
            Memory::writeByteInternal(MEM_SOUND_NR12, ((volume1 - 1) << 4) | (increase1 << 3) | (number1 - 1), true);
        }
    }

    if (number2 > 0) {
        if (increase2 && volume2 < 0xF) {
            Memory::writeByteInternal(MEM_SOUND_NR22, ((volume2 + 1) << 4) | (increase2 << 3) | (number2 - 1), true);
        } else if (!increase2 && volume2 > 0) {
            Memory::writeByteInternal(MEM_SOUND_NR22, ((volume2 - 1) << 4) | (increase2 << 3) | (number2 - 1), true);
        }
    }
}
