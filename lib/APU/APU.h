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

#define AUDIO_OUT_SQUARE1 7
#define AUDIO_OUT_SQUARE2 6
#define AUDIO_OUT_NOISE   4

class APU {
   public:
    static void begin();
    static void apuStep();
    static void triggerSquare1();
    static void triggerSquare2();
    static void triggerNoise();
    static void loadLength1();
    static void loadLength2();
    static void loadLength4();

   protected:
    static IntervalTimer squareTimer[2];
    static IntervalTimer effectTimer;
    static IntervalTimer noiseTimer;

    const static uint8_t duty[4];
    const static uint8_t divisor[8];

    volatile static bool channelEnabled[4];
    volatile static uint16_t currentSquareFrequency[2];
    volatile static uint16_t currentNoiseFrequency;
    volatile static uint8_t dutyStep[2];
    volatile static uint8_t lengthCounter[4];
    volatile static uint8_t envelopeStep[4];
    volatile static uint16_t sweepFrequency;
    volatile static uint8_t sweepStep;
    volatile static uint8_t effectTimerCounter;
    volatile static uint16_t noiseRegister;

    static void squareUpdate1();
    static void squareUpdate2();
    static void noiseUpdate();
    static void effectUpdate();

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

   private:
};
