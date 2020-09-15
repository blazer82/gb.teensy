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

// Use Teensyduino's IntervalTimer for all sound channels
// Because as of v0.2.1 using Periodic Timers with TeensyTimerTool
// did result in audible glitches when updating the channel's frequency
IntervalTimer APU::frequencyTimer[];

// Use a General Purpose Timer for effects
PeriodicTimer APU::effectTimer(GPT1);

void (*APU::frequencyUpdate[])() = {APU::squareUpdate1, APU::squareUpdate2, APU::waveUpdate};

const uint8_t APU::duty[] = {0x01, 0x81, 0x87, 0x7E};

volatile bool APU::channelEnabled[] = {0, 0, 0, 0};
volatile uint16_t APU::currentFrequency[] = {0, 0, 0, 0};
volatile uint8_t APU::dutyStep[] = {0, 0, 0};
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
    analogWriteFrequency(AUDIO_OUT_WAVE, 9375000);
    analogWriteFrequency(AUDIO_OUT_NOISE, 9375000);

    analogWrite(AUDIO_OUT_SQUARE1, 0);
    analogWrite(AUDIO_OUT_SQUARE2, 0);
    analogWrite(AUDIO_OUT_WAVE, 0);
    analogWrite(AUDIO_OUT_NOISE, 0);

    // Start effect timer
    APU::effectTimer.begin(APU::effectUpdate, 1000000 / 256);

    for (uint8_t i = 0; i < 3; i++) {
        APU::frequencyTimer[i].begin(APU::frequencyUpdate[i], 1000000);
    }
}

void APU::apuStep() {
    const nr52_register_t nr52 = {.value = Memory::readByte(MEM_SOUND_NR52)};

    if (nr52.bits.masterSwitch) {
        const nrx4_register_t nr14 = {.value = Memory::readByte(MEM_SOUND_NR14)};
        const nrx4_register_t nr24 = {.value = Memory::readByte(MEM_SOUND_NR24)};
        const nrx4_register_t nr34 = {.value = Memory::readByte(MEM_SOUND_NR34)};
        const uint16_t frequency[] = {(uint16_t)(0x20000 / (0x800 - ((nr14.bits.frequency << 8) | Memory::readByte(MEM_SOUND_NR13)))),
                                      (uint16_t)(0x20000 / (0x800 - ((nr24.bits.frequency << 8) | Memory::readByte(MEM_SOUND_NR23)))),
                                      (uint16_t)(0x10000 / (0x800 - ((nr34.bits.frequency << 8) | Memory::readByte(MEM_SOUND_NR33))))};

        for (uint8_t i = 0; i < 3; i++) {
            if (APU::currentFrequency[i] != frequency[i]) {
                APU::currentFrequency[i] = frequency[i];

                if (frequency[i] != 0) {
                    APU::frequencyTimer[i].update(1000000 / 8 / (uint32_t)frequency[i]);
                }
            }
        }

        const nr43_register_t nr43 = {.value = Memory::readByte(MEM_SOUND_NR43)};
        const uint16_t noiseFreq = 0x80000 / APU::divisor[nr43.bits.divisor] / (1 << (nr43.bits.shift + 1));

        if (APU::currentFrequency[Channel::noise] != noiseFreq) {
            APU::currentFrequency[Channel::noise] = noiseFreq;

            if (noiseFreq == 0) {
                APU::frequencyTimer[Channel::noise].update(1000000);
                analogWrite(AUDIO_OUT_NOISE, 0);
            } else {
                APU::frequencyTimer[Channel::noise].begin(APU::noiseUpdate, 1000000 / noiseFreq);
            }
        }
    } else {
        APU::frequencyTimer[Channel::square1].update(1000000);
        APU::frequencyTimer[Channel::square2].update(1000000);
        APU::frequencyTimer[Channel::wave].update(1000000);
        APU::frequencyTimer[Channel::noise].update(1000000);
        analogWrite(AUDIO_OUT_SQUARE1, 0);
        analogWrite(AUDIO_OUT_SQUARE2, 0);
        analogWrite(AUDIO_OUT_WAVE, 0);
        analogWrite(AUDIO_OUT_NOISE, 0);
    }
}

void APU::squareUpdate1() {
    const nrx1_register_t nrx1 = {.value = Memory::readByte(MEM_SOUND_NR11)};
    const nrx4_register_t nrx4 = {.value = Memory::readByte(MEM_SOUND_NR14)};

    if (APU::channelEnabled[Channel::square1] && (!nrx4.bits.lengthEnable || APU::lengthCounter[Channel::square1] > 0)) {
        const nrx2_register_t envelope = {.value = Memory::readByte(MEM_SOUND_NR12)};
        const nr50_register_t channelControl = {.value = Memory::readByte(MEM_SOUND_NR50)};
        const nr51_register_t terminalControl = {.value = Memory::readByte(MEM_SOUND_NR51)};
        const uint8_t mixerVolume = (channelControl.bits.terminal1Volume * terminalControl.bits.square1Terminal1 +
                                     channelControl.bits.terminal2Volume * terminalControl.bits.square1Terminal2) /
                                    (terminalControl.bits.square1Terminal1 + terminalControl.bits.square1Terminal2);
        analogWrite(AUDIO_OUT_SQUARE1, ((duty[nrx1.bits.duty] >> APU::dutyStep[Channel::square1]) & 1) * envelope.bits.volume * mixerVolume * 2);
        APU::dutyStep[Channel::square1]++;
        APU::dutyStep[Channel::square1] %= 8;
    } else {
        APU::channelEnabled[Channel::square1] = 0;
        analogWrite(AUDIO_OUT_SQUARE1, 0);
    }
}

void APU::squareUpdate2() {
    const nrx1_register_t nrx1 = {.value = Memory::readByte(MEM_SOUND_NR21)};
    const nrx4_register_t nrx4 = {.value = Memory::readByte(MEM_SOUND_NR24)};

    if (APU::channelEnabled[Channel::square2] && (!nrx4.bits.lengthEnable || APU::lengthCounter[Channel::square2] > 0)) {
        const nrx2_register_t envelope = {.value = Memory::readByte(MEM_SOUND_NR22)};
        const nr50_register_t channelControl = {.value = Memory::readByte(MEM_SOUND_NR50)};
        const nr51_register_t terminalControl = {.value = Memory::readByte(MEM_SOUND_NR51)};
        const uint8_t mixerVolume = (channelControl.bits.terminal1Volume * terminalControl.bits.square2Terminal1 +
                                     channelControl.bits.terminal2Volume * terminalControl.bits.square2Terminal2) /
                                    (terminalControl.bits.square2Terminal1 + terminalControl.bits.square2Terminal2);
        analogWrite(AUDIO_OUT_SQUARE2, ((duty[nrx1.bits.duty] >> APU::dutyStep[Channel::square2]) & 1) * envelope.bits.volume * mixerVolume * 2);
        APU::dutyStep[Channel::square2]++;
        APU::dutyStep[Channel::square2] %= 8;
    } else {
        APU::channelEnabled[Channel::square2] = 0;
        analogWrite(AUDIO_OUT_SQUARE2, 0);
    }
}

void APU::waveUpdate() {
    const bool enabled = Memory::readByte(MEM_SOUND_NR30) >> 7;
    const nrx4_register_t nrx4 = {.value = Memory::readByte(MEM_SOUND_NR34)};

    if (enabled && APU::channelEnabled[Channel::wave] && (!nrx4.bits.lengthEnable || APU::lengthCounter[Channel::wave] > 0)) {
        const uint8_t volumeShift = ((Memory::readByte(MEM_SOUND_NR32) >> 5) & 0x3);
        const uint8_t waveByte = Memory::readByte(MEM_SOUND_WAVE_START + 16 - APU::dutyStep[Channel::wave] / 2);
        const uint8_t waveNibble = (waveByte >> (1 - (APU::dutyStep[Channel::wave] % 2))) & 0xF;

        const nr50_register_t channelControl = {.value = Memory::readByte(MEM_SOUND_NR50)};
        const nr51_register_t terminalControl = {.value = Memory::readByte(MEM_SOUND_NR51)};
        const uint8_t mixerVolume = (channelControl.bits.terminal1Volume * terminalControl.bits.waveTerminal1 +
                                     channelControl.bits.terminal2Volume * terminalControl.bits.waveTerminal2) /
                                    (terminalControl.bits.waveTerminal1 + terminalControl.bits.waveTerminal2);
        analogWrite(AUDIO_OUT_WAVE, (waveNibble >> volumeShift) * mixerVolume);
        APU::dutyStep[Channel::wave]++;
        APU::dutyStep[Channel::wave] %= 32;
    } else {
        APU::channelEnabled[Channel::wave] = 0;
        analogWrite(AUDIO_OUT_WAVE, 0);
    }
}

void APU::noiseUpdate() {
    const nrx4_register_t nrx4 = {.value = Memory::readByte(MEM_SOUND_NR44)};

    if (APU::channelEnabled[Channel::noise] && (!nrx4.bits.lengthEnable || APU::lengthCounter[Channel::noise] > 0)) {
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

        analogWrite(AUDIO_OUT_NOISE, !(APU::noiseRegister & 1) * envelope.bits.volume * mixerVolume * 2);
    } else {
        APU::channelEnabled[Channel::noise] = 0;
        analogWrite(AUDIO_OUT_NOISE, 0);
    }
}

void APU::effectUpdate() {
    noInterrupts();
    APU::effectTimerCounter++;

    // Length update
    {
        const nrx4_register_t nrx4 = {.value = Memory::readByte(MEM_SOUND_NR14)};
        if (nrx4.bits.lengthEnable && APU::lengthCounter[Channel::square1] != 0) {
            APU::lengthCounter[Channel::square1]--;
        }
    }

    {
        const nrx4_register_t nrx4 = {.value = Memory::readByte(MEM_SOUND_NR24)};
        if (nrx4.bits.lengthEnable && APU::lengthCounter[Channel::square2] != 0) {
            APU::lengthCounter[Channel::square2]--;
        }
    }

    {
        const nrx4_register_t nrx4 = {.value = Memory::readByte(MEM_SOUND_NR34)};
        if (nrx4.bits.lengthEnable && APU::lengthCounter[Channel::wave] != 0) {
            APU::lengthCounter[Channel::wave]--;
        }
    }

    {
        const nrx4_register_t nrx4 = {.value = Memory::readByte(MEM_SOUND_NR44)};
        if (nrx4.bits.lengthEnable && APU::lengthCounter[Channel::noise] != 0) {
            APU::lengthCounter[Channel::noise]--;
        }
    }

    if ((APU::effectTimerCounter % 2) == 0) {
        // Sweep update
        const nr10_register_t nr10 = {.value = Memory::readByte(MEM_SOUND_NR10)};

        if (nr10.bits.time != 0 && nr10.bits.shift != 0) {
            APU::sweepStep++;

            if ((APU::sweepStep % nr10.bits.time) == 0) {
                const uint16_t newFrequency = APU::sweepFrequency + (APU::sweepFrequency >> nr10.bits.shift) * (nr10.bits.direction ? -1 : 1);
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
            APU::envelopeStep[Channel::square1]++;
            const nrx2_register_t envelope = {.value = Memory::readByte(MEM_SOUND_NR12)};

            if (APU::envelopeStep[Channel::square1] % envelope.bits.period == 0) {
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
            APU::envelopeStep[Channel::square2]++;
            const nrx2_register_t envelope = {.value = Memory::readByte(MEM_SOUND_NR22)};

            if (APU::envelopeStep[Channel::square2] % envelope.bits.period == 0) {
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
            APU::envelopeStep[Channel::noise]++;
            const nrx2_register_t envelope = {.value = Memory::readByte(MEM_SOUND_NR42)};

            if (APU::envelopeStep[Channel::noise] % envelope.bits.period == 0) {
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
    APU::envelopeStep[Channel::square1] = 0;
    APU::channelEnabled[Channel::square1] = 1;
    APU::sweepStep = 0;
    APU::sweepFrequency = (uint16_t)(0x20000 / (0x800 - ((nr14.bits.frequency << 8) | Memory::readByte(MEM_SOUND_NR13))));
}

void APU::triggerSquare2() {
    APU::loadLength2();
    APU::envelopeStep[Channel::square2] = 0;
    APU::channelEnabled[Channel::square2] = 1;
}

void APU::triggerWave() {
    APU::loadLength3();
    APU::dutyStep[Channel::wave] = 0;
    APU::channelEnabled[Channel::wave] = 1;
}

void APU::triggerNoise() {
    APU::loadLength4();
    APU::envelopeStep[Channel::noise] = 0;
    APU::channelEnabled[Channel::noise] = 1;
    APU::noiseRegister = 0xFFFF;
}

void APU::loadLength1() {
    const nrx1_register_t nrx1 = {.value = Memory::readByte(MEM_SOUND_NR11)};
    APU::lengthCounter[Channel::square1] = 0x40 - nrx1.bits.length;
}

void APU::loadLength2() {
    const nrx1_register_t nrx1 = {.value = Memory::readByte(MEM_SOUND_NR21)};
    APU::lengthCounter[Channel::square2] = 0x40 - nrx1.bits.length;
}

void APU::loadLength3() {
    const uint8_t length = Memory::readByte(MEM_SOUND_NR31);
    APU::lengthCounter[Channel::wave] = 0x100 - length;
}

void APU::loadLength4() {
    const nrx1_register_t nrx1 = {.value = Memory::readByte(MEM_SOUND_NR41)};
    APU::lengthCounter[Channel::noise] = 0x40 - nrx1.bits.length;
}
