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

#pragma once

#include <Arduino.h>
#include <TeensyTimerTool.h>

using namespace TeensyTimerTool;

#define AUDIO_OUT_SQUARE1 7
#define AUDIO_OUT_SQUARE2 6
#define AUDIO_OUT_WAVE    5
#define AUDIO_OUT_NOISE   4

typedef union {
    struct {
        unsigned shift : 3;
        unsigned direction : 1;
        unsigned time : 3;
        unsigned : 1;
    } bits;
    uint8_t value;
} nr10_register_t;

typedef union {
    struct {
        unsigned length : 6;
        unsigned duty : 2;
    } bits;
    uint8_t value;
} nrx1_register_t;

typedef union {
    struct {
        unsigned period : 3;
        unsigned direction : 1;
        unsigned volume : 4;
    } bits;
    uint8_t value;
} nrx2_register_t;

typedef union {
    struct {
        unsigned divisor : 3;
        unsigned width : 1;
        unsigned shift : 4;
    } bits;
    uint8_t value;
} nr43_register_t;

typedef union {
    struct {
        unsigned frequency : 3;
        unsigned : 3;
        unsigned lengthEnable : 1;
        unsigned initial : 1;
    } bits;
    uint8_t value;
} nrx4_register_t;

typedef union {
    struct {
        unsigned terminal1Volume : 3;
        unsigned VINTerminal1 : 1;
        unsigned terminal2Volume : 3;
        unsigned VINTerminal2 : 1;
    } bits;
    uint8_t value;
} nr50_register_t;

typedef union {
    struct {
        unsigned square1Terminal1 : 1;
        unsigned square2Terminal1 : 1;
        unsigned waveTerminal1 : 1;
        unsigned noiseTerminal1 : 1;
        unsigned square1Terminal2 : 1;
        unsigned square2Terminal2 : 1;
        unsigned waveTerminal2 : 1;
        unsigned noiseTerminal2 : 1;
    } bits;
    uint8_t value;
} nr51_register_t;

typedef union {
    struct {
        unsigned sound1On : 1;
        unsigned sound2On : 1;
        unsigned sound3On : 1;
        unsigned sound4On : 1;
        unsigned : 3;
        unsigned masterSwitch : 1;
    } bits;
    uint8_t value;
} nr52_register_t;

class APU {
   public:
    static void begin();
    static void apuStep();
    static void triggerSquare1();
    static void triggerSquare2();
    static void triggerWave();
    static void triggerNoise();
    static void loadLength1();
    static void loadLength2();
    static void loadLength3();
    static void loadLength4();
    static void disableDac1();
    static void disableDac2();
    static void disableDac3();
    static void disableDac4();
    static void enableDac1();
    static void enableDac2();
    static void enableDac3();
    static void enableDac4();

   protected:
    enum Channel { square1, square2, wave, noise };

    static IntervalTimer frequencyTimer[4];
    static PeriodicTimer effectTimer;

    static void (*frequencyUpdate[4])();

    const static uint8_t duty[4];
    const static uint8_t divisor[8];

    volatile static bool dacEnabled[4];
    volatile static bool channelEnabled[4];
    volatile static uint16_t currentFrequency[4];
    volatile static uint8_t dutyStep[3];
    volatile static uint8_t lengthCounter[4];
    volatile static uint8_t envelopeStep[4];
    volatile static uint16_t sweepFrequency;
    volatile static uint8_t sweepStep;
    volatile static uint8_t effectTimerCounter;
    volatile static uint16_t noiseRegister;

    static void squareUpdate1();
    static void squareUpdate2();
    static void waveUpdate();
    static void noiseUpdate();
    static void effectUpdate();
    static void disableSquare1();
    static void disableSquare2();
    static void disableWave();
    static void disableNoise();

   private:
};
