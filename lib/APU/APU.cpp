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

#include <Arduino.h>

#include "Memory.h"

IntervalTimer APU::squareTimer[];
IntervalTimer APU::effectTimer;
IntervalTimer APU::noiseTimer;

const uint8_t APU::duty[] = {0x01, 0x81, 0x87, 0x7E};

volatile bool APU::channelEnabled[] = {0, 0, 0, 0};
volatile uint16_t APU::currentSquareFrequency[] = {0, 0};
volatile uint16_t APU::currentNoiseFrequency = 0;
volatile uint8_t APU::dutyStep[] = {0, 0};
volatile uint8_t APU::lengthCounter[] = {0, 0, 0, 0};
volatile uint8_t APU::envelopeStep[] = {0, 0, 0, 0};
volatile uint16_t APU::sweepFrequency = 0;
volatile uint8_t APU::sweepStep = 0;
volatile uint8_t APU::effectTimerCounter = 0;
volatile uint16_t APU::noiseRegister = 0xFFFF;

const uint8_t APU::divisor[] = {8, 16, 32, 48, 64, 80, 96, 112};

void APU::begin() {
    pinMode(AUDIO_OUT_SQUARE1, OUTPUT);
    pinMode(AUDIO_OUT_SQUARE2, OUTPUT);
    pinMode(AUDIO_OUT_WAVE, OUTPUT);
    pinMode(AUDIO_OUT_NOISE, OUTPUT);

    // Setup PWM resolution and frequency according to https://www.pjrc.com/teensy/td_pulse.html
    analogWriteResolution(8);
    analogWriteFrequency(AUDIO_OUT_SQUARE1, 9375000);
    analogWriteFrequency(AUDIO_OUT_SQUARE2, 9375000);
    analogWriteFrequency(AUDIO_OUT_NOISE, 9375000);

    APU::squareTimer[0].begin(APU::squareUpdate1, 1000000);
    APU::squareTimer[1].begin(APU::squareUpdate2, 1000000);
    APU::noiseTimer.begin(APU::noiseUpdate, 1000000);
    APU::effectTimer.begin(APU::effectUpdate, 1000000 / 256);
}

void APU::apuStep() {
    const nr52_register_t nr52 = {.value = Memory::readByte(MEM_SOUND_NR52)};

    if (nr52.bits.masterSwitch) {
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
                    APU::squareTimer[i].update(1000000 / 8 / (uint32_t)squareFreq[i]);
                }
            }
        }

        const nr43_register_t nr43 = {.value = Memory::readByte(MEM_SOUND_NR43)};
        const uint16_t noiseFreq = 0x80000 / APU::divisor[nr43.bits.divisor] / (1 << (nr43.bits.shift + 1));

        if (APU::currentNoiseFrequency != noiseFreq) {
            APU::currentNoiseFrequency = noiseFreq;

            if (noiseFreq == 0) {
                APU::noiseTimer.update(1000000);
                analogWrite(AUDIO_OUT_NOISE, 0);
            } else {
                APU::noiseTimer.update(1000000 / noiseFreq);
            }
        }
    } else {
        APU::squareTimer[0].update(1000000);
        APU::squareTimer[1].update(1000000);
        APU::noiseTimer.update(1000000);
        analogWrite(AUDIO_OUT_SQUARE1, 0);
        analogWrite(AUDIO_OUT_SQUARE2, 0);
        analogWrite(AUDIO_OUT_NOISE, 0);
    }
}

void APU::squareUpdate1() {
    const nrx1_register_t nrx1 = {.value = Memory::readByte(MEM_SOUND_NR11)};
    const nrx4_register_t nrx4 = {.value = Memory::readByte(MEM_SOUND_NR14)};

    if (APU::channelEnabled[0] && (!nrx4.bits.lengthEnable || APU::lengthCounter[0] > 0)) {
        const nrx2_register_t envelope = {.value = Memory::readByte(MEM_SOUND_NR12)};
        const nr50_register_t channelControl = {.value = Memory::readByte(MEM_SOUND_NR50)};
        const nr51_register_t terminalControl = {.value = Memory::readByte(MEM_SOUND_NR51)};
        const uint8_t mixerVolume = (channelControl.bits.terminal1Volume * terminalControl.bits.square1Terminal1 +
                                     channelControl.bits.terminal2Volume * terminalControl.bits.square1Terminal2) /
                                    (terminalControl.bits.square1Terminal1 + terminalControl.bits.square1Terminal2);
        analogWrite(AUDIO_OUT_SQUARE1, ((duty[nrx1.bits.duty] >> APU::dutyStep[0]) & 1) * envelope.bits.volume * mixerVolume);
        APU::dutyStep[0]++;
        APU::dutyStep[0] %= 8;
    } else {
        APU::channelEnabled[0] = 0;
        analogWrite(AUDIO_OUT_SQUARE1, 0);
    }
}

void APU::squareUpdate2() {
    const nrx1_register_t nrx1 = {.value = Memory::readByte(MEM_SOUND_NR21)};
    const nrx4_register_t nrx4 = {.value = Memory::readByte(MEM_SOUND_NR24)};

    if (APU::channelEnabled[1] && (!nrx4.bits.lengthEnable || APU::lengthCounter[1] > 0)) {
        const nrx2_register_t envelope = {.value = Memory::readByte(MEM_SOUND_NR22)};
        const nr50_register_t channelControl = {.value = Memory::readByte(MEM_SOUND_NR50)};
        const nr51_register_t terminalControl = {.value = Memory::readByte(MEM_SOUND_NR51)};
        const uint8_t mixerVolume = (channelControl.bits.terminal1Volume * terminalControl.bits.square2Terminal1 +
                                     channelControl.bits.terminal2Volume * terminalControl.bits.square2Terminal2) /
                                    (terminalControl.bits.square2Terminal1 + terminalControl.bits.square2Terminal2);
        analogWrite(AUDIO_OUT_SQUARE2, ((duty[nrx1.bits.duty] >> APU::dutyStep[1]) & 1) * envelope.bits.volume * mixerVolume);
        APU::dutyStep[1]++;
        APU::dutyStep[1] %= 8;
    } else {
        APU::channelEnabled[1] = 0;
        analogWrite(AUDIO_OUT_SQUARE2, 0);
    }
}

void APU::noiseUpdate() {
    const nrx4_register_t nrx4 = {.value = Memory::readByte(MEM_SOUND_NR44)};

    if (APU::channelEnabled[3] && (!nrx4.bits.lengthEnable || APU::lengthCounter[3] > 0)) {
        const nr43_register_t nr43 = {.value = Memory::readByte(MEM_SOUND_NR43)};

        const bool xorBit = (APU::noiseRegister >> 1 & 0x1) ^ (APU::noiseRegister & 0x1);
        APU::noiseRegister = (APU::noiseRegister >> 1) | (xorBit << 14);

        if (nr43.bits.width) {
            APU::noiseRegister = (APU::noiseRegister & 0xFFBF) | (xorBit << 6);
        }

        const nrx2_register_t envelope = {.value = Memory::readByte(MEM_SOUND_NR42)};
        const nr50_register_t channelControl = {.value = Memory::readByte(MEM_SOUND_NR50)};
        const nr51_register_t terminalControl = {.value = Memory::readByte(MEM_SOUND_NR51)};
        const uint8_t mixerVolume = (channelControl.bits.terminal1Volume * terminalControl.bits.noiseTerminal1 +
                                     channelControl.bits.terminal2Volume * terminalControl.bits.noiseTerminal2) /
                                    (terminalControl.bits.noiseTerminal1 + terminalControl.bits.noiseTerminal2);

        analogWrite(AUDIO_OUT_NOISE, !(APU::noiseRegister & 1) * envelope.bits.volume * mixerVolume);
    } else {
        APU::channelEnabled[3] = 0;
        analogWrite(AUDIO_OUT_NOISE, 0);
    }
}

void APU::effectUpdate() {
    noInterrupts();
    APU::effectTimerCounter++;

    // Length update
    {
        const nrx4_register_t nrx4 = {.value = Memory::readByte(MEM_SOUND_NR14)};
        if (nrx4.bits.lengthEnable && APU::lengthCounter[0] != 0) {
            APU::lengthCounter[0]--;
        }
    }

    {
        const nrx4_register_t nrx4 = {.value = Memory::readByte(MEM_SOUND_NR24)};
        if (nrx4.bits.lengthEnable && APU::lengthCounter[1] != 0) {
            APU::lengthCounter[1]--;
        }
    }

    {
        const nrx4_register_t nrx4 = {.value = Memory::readByte(MEM_SOUND_NR44)};
        if (nrx4.bits.lengthEnable && APU::lengthCounter[3] != 0) {
            APU::lengthCounter[3]--;
        }
    }

    if ((APU::effectTimerCounter % 2) == 0) {
        // Sweep update
        const nr10_register_t nr10 = {.value = Memory::readByte(MEM_SOUND_NR10)};

        if (nr10.bits.time != 0 && nr10.bits.shift != 0) {
            APU::sweepStep++;

            if ((APU::sweepStep % nr10.bits.time) == 0) {
                const uint16_t newFrequency = nr10.bits.direction ? (APU::sweepFrequency << nr10.bits.shift) : (APU::sweepFrequency >> nr10.bits.shift);
                if (newFrequency > 0 && newFrequency < 0x7FF) {
                    APU::sweepFrequency = newFrequency;
                    Memory::writeByteInternal(MEM_SOUND_NR13, newFrequency & 0xF, true);
                    Memory::writeByteInternal(MEM_SOUND_NR14, ((newFrequency >> 8) & 0x3) | (Memory::readByte(MEM_SOUND_NR14) & 0xFC), true);
                }
            }
        }
    }

    if ((APU::effectTimerCounter % 4) == 0) {
        // Envelope update
        {
            APU::envelopeStep[0]++;
            const nrx2_register_t envelope = {.value = Memory::readByte(MEM_SOUND_NR12)};

            if (APU::envelopeStep[0] % envelope.bits.period == 0) {
                if (envelope.bits.direction && envelope.bits.volume < 0xF) {
                    const nrx2_register_t newEnvelope = {
                        .bits = {.period = envelope.bits.period, .direction = envelope.bits.direction, .volume = envelope.bits.volume + 1U}};
                    Memory::writeByteInternal(MEM_SOUND_NR12, newEnvelope.value, true);
                } else if (!envelope.bits.direction && envelope.bits.volume > 0) {
                    const nrx2_register_t newEnvelope = {
                        .bits = {.period = envelope.bits.period, .direction = envelope.bits.direction, .volume = envelope.bits.volume - 1U}};
                    Memory::writeByteInternal(MEM_SOUND_NR12, newEnvelope.value, true);
                }
            }
        }
        {
            APU::envelopeStep[1]++;
            const nrx2_register_t envelope = {.value = Memory::readByte(MEM_SOUND_NR22)};

            if (APU::envelopeStep[1] % envelope.bits.period == 0) {
                if (envelope.bits.direction && envelope.bits.volume < 0xF) {
                    const nrx2_register_t newEnvelope = {
                        .bits = {.period = envelope.bits.period, .direction = envelope.bits.direction, .volume = envelope.bits.volume + 1U}};
                    Memory::writeByteInternal(MEM_SOUND_NR22, newEnvelope.value, true);
                } else if (!envelope.bits.direction && envelope.bits.volume > 0) {
                    const nrx2_register_t newEnvelope = {
                        .bits = {.period = envelope.bits.period, .direction = envelope.bits.direction, .volume = envelope.bits.volume - 1U}};
                    Memory::writeByteInternal(MEM_SOUND_NR22, newEnvelope.value, true);
                }
            }
        }
        {
            APU::envelopeStep[3]++;
            const nrx2_register_t envelope = {.value = Memory::readByte(MEM_SOUND_NR42)};

            if (APU::envelopeStep[3] % envelope.bits.period == 0) {
                if (envelope.bits.direction && envelope.bits.volume < 0xF) {
                    const nrx2_register_t newEnvelope = {
                        .bits = {.period = envelope.bits.period, .direction = envelope.bits.direction, .volume = envelope.bits.volume + 1U}};
                    Memory::writeByteInternal(MEM_SOUND_NR42, newEnvelope.value, true);
                } else if (!envelope.bits.direction && envelope.bits.volume > 0) {
                    const nrx2_register_t newEnvelope = {
                        .bits = {.period = envelope.bits.period, .direction = envelope.bits.direction, .volume = envelope.bits.volume - 1U}};
                    Memory::writeByteInternal(MEM_SOUND_NR42, newEnvelope.value, true);
                }
            }
        }
    }
    interrupts();
}

void APU::triggerSquare1() {
    const nrx4_register_t nr14 = {.value = Memory::readByte(MEM_SOUND_NR14)};

    APU::loadLength1();
    APU::envelopeStep[0] = 0;
    APU::channelEnabled[0] = 1;
    APU::sweepStep = 0;
    APU::sweepFrequency = (uint16_t)(0x20000 / (0x800 - ((nr14.bits.frequency << 8) | Memory::readByte(MEM_SOUND_NR13))));
}

void APU::triggerSquare2() {
    APU::loadLength2();
    APU::envelopeStep[1] = 0;
    APU::channelEnabled[1] = 1;
}

void APU::triggerNoise() {
    APU::loadLength4();
    APU::envelopeStep[3] = 0;
    APU::channelEnabled[3] = 1;
    APU::noiseRegister = 0xFFFF;
}

void APU::loadLength1() {
    const nrx1_register_t nrx1 = {.value = Memory::readByte(MEM_SOUND_NR11)};
    APU::lengthCounter[0] = 0x40 - nrx1.bits.length;
}

void APU::loadLength2() {
    const nrx1_register_t nrx1 = {.value = Memory::readByte(MEM_SOUND_NR21)};
    APU::lengthCounter[1] = 0x40 - nrx1.bits.length;
}

void APU::loadLength4() {
    const nrx1_register_t nrx1 = {.value = Memory::readByte(MEM_SOUND_NR41)};
    APU::lengthCounter[3] = 0x40 - nrx1.bits.length;
}
