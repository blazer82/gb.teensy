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

IntervalTimer APU::squareTimer[];
IntervalTimer APU::sweepTimer;
IntervalTimer APU::lengthTimer;
IntervalTimer APU::envelopeTimer;

const uint8_t APU::duty[] = {0x01, 0x81, 0x87, 0x7E};

volatile uint8_t APU::currentSquareFrequency[] = {0, 0};
volatile uint8_t APU::dutyStep[] = {0, 0};

volatile uint8_t APU::sweepStep = 0;

void APU::begin() {
    pinMode(AUDIO_OUT_SQUARE1, OUTPUT);
    pinMode(AUDIO_OUT_SQUARE2, OUTPUT);

    // Setup PWM resolution and frequency according to https://www.pjrc.com/teensy/td_pulse.html
    analogWriteResolution(8);
    analogWriteFrequency(AUDIO_OUT_SQUARE1, 9375000);
    analogWriteFrequency(AUDIO_OUT_SQUARE2, 9375000);

    APU::squareTimer[0].begin(APU::squareUpdate1, 1000000);
    APU::squareTimer[1].begin(APU::squareUpdate2, 1000000);
    APU::sweepTimer.begin(APU::sweepUpdate, 7813);
    APU::lengthTimer.begin(APU::lengthUpdate, 3906);
    APU::envelopeTimer.begin(APU::envelopeUpdate, 15625);
}

void APU::apuStep() {
    const uint16_t squareFreq[] = {(uint16_t)(0x20000 / (0x800 - (((Memory::readByte(MEM_SOUND_NR14) & 0x7) << 8) | Memory::readByte(MEM_SOUND_NR13)))),
                                   (uint16_t)(0x20000 / (0x800 - (((Memory::readByte(MEM_SOUND_NR24) & 0x7) << 8) | Memory::readByte(MEM_SOUND_NR23))))};

    for (uint8_t i = 0; i < 2; i++) {
        if (APU::currentSquareFrequency[i] != squareFreq[i]) {
            APU::currentSquareFrequency[i] = squareFreq[i];

            if (squareFreq[i] == 0) {
                APU::squareTimer[i].update(1000000);
                analogWrite(i == 0 ? AUDIO_OUT_SQUARE1 : AUDIO_OUT_SQUARE2, 0);
            } else {
                APU::squareTimer[i].update(125000 / (uint32_t)squareFreq[i]);
            }
        }
    }
}

void APU::squareUpdate1() {
    const bool lengthEnable = (Memory::readByte(MEM_SOUND_NR14) & 0x40) != 0;
    const uint8_t length = lengthEnable ? 0x40 - (Memory::readByte(MEM_SOUND_NR11) & 0x3F) : 1;

    if (length > 0) {
        const uint8_t dutyIndex = Memory::readByte(MEM_SOUND_NR11) >> 6;
        const uint8_t envelopeVolume = Memory::readByte(MEM_SOUND_NR12) >> 4;
        const bool so1 = (Memory::readByte(MEM_SOUND_NR51) & 0x1) != 0;
        const bool so2 = (Memory::readByte(MEM_SOUND_NR51) & 0x10) != 0;
        const uint8_t mixerVolume = ((Memory::readByte(MEM_SOUND_NR50) & 0x7) * so1 + ((Memory::readByte(MEM_SOUND_NR50) >> 4 & 0x7)) * so2) / (so1 + so2);
        analogWrite(AUDIO_OUT_SQUARE1, ((duty[dutyIndex] >> APU::dutyStep[0]) & 1) * envelopeVolume * mixerVolume);
        APU::dutyStep[0]++;
        APU::dutyStep[0] %= 8;
    } else {
        analogWrite(AUDIO_OUT_SQUARE1, 0);
    }
}

void APU::squareUpdate2() {
    const bool lengthEnable = (Memory::readByte(MEM_SOUND_NR24) & 0x40) != 0;
    const uint8_t length = lengthEnable ? 0x40 - (Memory::readByte(MEM_SOUND_NR21) & 0x3F) : 1;

    if (length > 0) {
        const uint8_t dutyIndex = Memory::readByte(MEM_SOUND_NR21) >> 6;
        const uint8_t envelopeVolume = Memory::readByte(MEM_SOUND_NR22) >> 4;
        const bool so1 = (Memory::readByte(MEM_SOUND_NR51) & 0x2) != 0;
        const bool so2 = (Memory::readByte(MEM_SOUND_NR51) & 0x20) != 0;
        const uint8_t mixerVolume = ((Memory::readByte(MEM_SOUND_NR50) & 0x7) * so1 + ((Memory::readByte(MEM_SOUND_NR50) >> 4 & 0x7)) * so2) / (so1 + so2);
        analogWrite(AUDIO_OUT_SQUARE2, ((duty[dutyIndex] >> APU::dutyStep[1]) & 1) * envelopeVolume * mixerVolume);
        APU::dutyStep[1]++;
        APU::dutyStep[1] %= 8;
    } else {
        analogWrite(AUDIO_OUT_SQUARE2, 0);
    }
}

void APU::sweepUpdate() {
    const uint8_t sweep = Memory::readByte(MEM_SOUND_NR10);
    const uint8_t sweepTime = (sweep >> 4) & 0x7;
    const bool increase = (sweep & 0x8) == 0;
    const uint8_t sweepNumber = sweep & 0x7;

    if (sweepTime != 0) {
        APU::sweepStep++;

        if ((APU::sweepStep % sweepTime) == 0) {
            const uint16_t frequency = ((Memory::readByte(MEM_SOUND_NR14) & 0x7) << 8) | Memory::readByte(MEM_SOUND_NR13);
            const uint16_t newFrequency = increase ? (frequency << sweepNumber) : (frequency >> sweepNumber);
            if (newFrequency == 0 || newFrequency > 0x7FF) {
                Memory::writeByteInternal(MEM_SOUND_NR13, 0, true);
                Memory::writeByteInternal(MEM_SOUND_NR14, Memory::readByte(MEM_SOUND_NR14) & 0xFC, true);
            } else {
                Memory::writeByteInternal(MEM_SOUND_NR13, newFrequency & 0xF, true);
                Memory::writeByteInternal(MEM_SOUND_NR14, ((newFrequency >> 8) & 0x3) | (Memory::readByte(MEM_SOUND_NR14) & 0xFC), true);
            }
        }
    }
}

void APU::lengthUpdate() {
    const bool lengthEnable1 = (Memory::readByte(MEM_SOUND_NR14) & 0x40) != 0;
    const bool lengthEnable2 = (Memory::readByte(MEM_SOUND_NR24) & 0x40) != 0;

    if (lengthEnable1) {
        const uint8_t length = 0x40 - (Memory::readByte(MEM_SOUND_NR11) & 0x3F);
        if (length > 0) {
            Memory::writeByteInternal(MEM_SOUND_NR11, (Memory::readByte(MEM_SOUND_NR11) & 0xC0) | (0x40 - (length - 1)), true);
        }
    }

    if (lengthEnable2) {
        const uint8_t length = 0x40 - (Memory::readByte(MEM_SOUND_NR21) & 0x3F);
        if (length > 0) {
            Memory::writeByteInternal(MEM_SOUND_NR21, (Memory::readByte(MEM_SOUND_NR21) & 0xC0) | (0x40 - (length - 1)), true);
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
