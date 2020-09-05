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
#define AUDIO_OUT_WAVE    5

class APU {
   public:
    static void begin();
    static void apuStep();
    static void triggerSquare1();
    static void triggerSquare2();
    static void loadLength1();
    static void loadLength2();

   protected:
    static IntervalTimer squareTimer[2];
    static IntervalTimer effectTimer;

    const static uint8_t duty[4];

    volatile static bool channelEnabled[2];
    volatile static uint8_t currentSquareFrequency[2];
    volatile static uint8_t dutyStep[2];
    volatile static uint8_t lengthCounter[2];
    volatile static uint8_t envelopeStep[2];
    volatile static uint8_t sweepStep;
    volatile static uint8_t effectTimerCounter;

    static void squareUpdate1();
    static void squareUpdate2();
    static void effectUpdate();

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
            unsigned frequency : 3;
            unsigned : 3;
            unsigned lengthEnable : 1;
            unsigned initial : 1;
        } bits;
        uint8_t value;
    } nrx4_register_t;

   private:
};
