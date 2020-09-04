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
IntervalTimer APU::effectTimer;

const uint8_t APU::duty[] = {0x01, 0x81, 0x87, 0x7E};

volatile bool APU::channelEnabled[] = {0, 0};
volatile uint8_t APU::currentSquareFrequency[] = {0, 0};
volatile uint8_t APU::dutyStep[] = {0, 0};

volatile uint8_t APU::sweepStep = 0;
volatile uint8_t APU::effectTimerCounter = 0;

const uint8_t divisor[] = {8, 16, 32, 48, 64, 80, 96, 112};

void APU::begin() {
    pinMode(AUDIO_OUT_SQUARE1, OUTPUT);
    pinMode(AUDIO_OUT_SQUARE2, OUTPUT);
    pinMode(AUDIO_OUT_WAVE, OUTPUT);

    // Setup PWM resolution and frequency according to https://www.pjrc.com/teensy/td_pulse.html
    analogWriteResolution(8);
    analogWriteFrequency(AUDIO_OUT_SQUARE1, 9375000);
    analogWriteFrequency(AUDIO_OUT_SQUARE2, 9375000);
    analogWriteFrequency(AUDIO_OUT_WAVE, 9375000);

    APU::squareTimer[0].begin(APU::squareUpdate1, 1000000);
    APU::squareTimer[1].begin(APU::squareUpdate2, 1000000);
    APU::effectTimer.begin(APU::effectUpdate, 3906);
}

void APU::apuStep() {
    const nrx4_register_t nr14 = {.value = Memory::readByte(MEM_SOUND_NR14)};
    const nrx4_register_t nr24 = {.value = Memory::readByte(MEM_SOUND_NR24)};
    const uint16_t squareFreq[] = {(uint16_t)(0x20000 / (0x800 - ((nr14.bits.frequency << 8) | Memory::readByte(MEM_SOUND_NR13)))),
                                   (uint16_t)(0x20000 / (0x800 - ((nr24.bits.frequency << 8) | Memory::readByte(MEM_SOUND_NR23))))};

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
    const nrx1_register_t nrx1 = {.value = Memory::readByte(MEM_SOUND_NR11)};
    const nrx4_register_t nrx4 = {.value = Memory::readByte(MEM_SOUND_NR14)};
    const uint8_t length = nrx4.bits.lengthEnable ? 0x40 - nrx1.bits.length : 1;

    if (APU::channelEnabled[0] && length > 0) {
        const nrx2_register_t envelope = {.value = Memory::readByte(MEM_SOUND_NR12)};
        const bool so1 = (Memory::readByte(MEM_SOUND_NR51) & 0x1) != 0;
        const bool so2 = (Memory::readByte(MEM_SOUND_NR51) & 0x10) != 0;
        const uint8_t mixerVolume = ((Memory::readByte(MEM_SOUND_NR50) & 0x7) * so1 + ((Memory::readByte(MEM_SOUND_NR50) >> 4 & 0x7)) * so2) / (so1 + so2);
        analogWrite(AUDIO_OUT_SQUARE1, ((duty[nrx1.bits.duty] >> APU::dutyStep[0]) & 1) * envelope.bits.volume * mixerVolume);
        APU::dutyStep[0]++;
        APU::dutyStep[0] %= 8;
    } else {
        analogWrite(AUDIO_OUT_SQUARE1, 0);
    }
}

void APU::squareUpdate2() {
    const nrx1_register_t nrx1 = {.value = Memory::readByte(MEM_SOUND_NR21)};
    const nrx4_register_t nrx4 = {.value = Memory::readByte(MEM_SOUND_NR24)};
    const uint8_t length = nrx4.bits.lengthEnable ? 0x40 - nrx1.bits.length : 1;

    if (APU::channelEnabled[1] && length > 0) {
        const nrx2_register_t envelope = {.value = Memory::readByte(MEM_SOUND_NR22)};
        const bool so1 = (Memory::readByte(MEM_SOUND_NR51) & 0x2) != 0;
        const bool so2 = (Memory::readByte(MEM_SOUND_NR51) & 0x20) != 0;
        const uint8_t mixerVolume = ((Memory::readByte(MEM_SOUND_NR50) & 0x7) * so1 + ((Memory::readByte(MEM_SOUND_NR50) >> 4 & 0x7)) * so2) / (so1 + so2);
        analogWrite(AUDIO_OUT_SQUARE2, ((duty[nrx1.bits.duty] >> APU::dutyStep[1]) & 1) * envelope.bits.volume * mixerVolume);
        APU::dutyStep[1]++;
        APU::dutyStep[1] %= 8;
    } else {
        analogWrite(AUDIO_OUT_SQUARE2, 0);
    }
}

void APU::effectUpdate() {
    APU::effectTimerCounter++;

    // Length update
    if (APU::channelEnabled[0]) {
        const nrx4_register_t nrx4 = {.value = Memory::readByte(MEM_SOUND_NR14)};
        if (nrx4.bits.lengthEnable) {
            const uint8_t length = 0x40 - (Memory::readByte(MEM_SOUND_NR11) & 0x3F);
            if (length > 0) {
                Memory::writeByteInternal(MEM_SOUND_NR11, (Memory::readByte(MEM_SOUND_NR11) & 0xC0) | (0x40 - (length - 1)), true);
            } else {
                APU::channelEnabled[0] = 0;
            }
        }
    }

    if (APU::channelEnabled[1]) {
        const nrx4_register_t nrx4 = {.value = Memory::readByte(MEM_SOUND_NR24)};
        if (nrx4.bits.lengthEnable) {
            const uint8_t length = 0x40 - (Memory::readByte(MEM_SOUND_NR21) & 0x3F);
            if (length > 0) {
                Memory::writeByteInternal(MEM_SOUND_NR21, (Memory::readByte(MEM_SOUND_NR21) & 0xC0) | (0x40 - (length - 1)), true);
            } else {
                APU::channelEnabled[1] = 0;
            }
        }
    }

    if ((APU::effectTimerCounter % 2) == 0) {
        // Sweep update
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

    if ((APU::effectTimerCounter % 4) == 0) {
        // Envelope update
        const nrx2_register_t envelope1 = {.value = Memory::readByte(MEM_SOUND_NR12)};
        const nrx2_register_t envelope2 = {.value = Memory::readByte(MEM_SOUND_NR22)};

        if (envelope1.bits.number > 0) {
            if (envelope1.bits.direction && envelope1.bits.volume < 0xF) {
                const nrx2_register_t newEnvelope = {
                    .bits = {.number = envelope1.bits.number - 1U, .direction = envelope1.bits.direction, .volume = envelope1.bits.volume + 1U}};
                Memory::writeByteInternal(MEM_SOUND_NR12, newEnvelope.value, true);
            } else if (!envelope1.bits.direction && envelope1.bits.volume > 0) {
                const nrx2_register_t newEnvelope = {.bits = {envelope1.bits.number - 1U, envelope1.bits.direction, envelope1.bits.volume - 1U}};
                Memory::writeByteInternal(MEM_SOUND_NR12, newEnvelope.value, true);
            }
        }

        if (envelope2.bits.number > 0) {
            if (envelope2.bits.direction && envelope2.bits.volume < 0xF) {
                const nrx2_register_t newEnvelope = {
                    .bits = {.number = envelope2.bits.number - 1U, .direction = envelope2.bits.direction, .volume = envelope2.bits.volume + 1U}};
                Memory::writeByteInternal(MEM_SOUND_NR22, newEnvelope.value, true);
            } else if (!envelope2.bits.direction && envelope2.bits.volume > 0) {
                const nrx2_register_t newEnvelope = {.bits = {envelope2.bits.number - 1U, envelope2.bits.direction, envelope2.bits.volume - 1U}};
                Memory::writeByteInternal(MEM_SOUND_NR22, newEnvelope.value, true);
            }
        }
    }
}

void APU::triggerSquare1() { APU::channelEnabled[0] = 1; }

void APU::triggerSquare2() { APU::channelEnabled[1] = 1; }
