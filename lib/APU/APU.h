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

#define AUDIO_OUT1 7
#define AUDIO_OUT2 6

class APU {
   public:
    static void begin();
    static void apuStep();

   protected:
    static IntervalTimer apuTimer1;
    static IntervalTimer apuTimer2;
    static IntervalTimer sweepTimer;
    static IntervalTimer lengthTimer;
    static IntervalTimer envelopeTimer;

    static void timer1Step();
    static void timer2Step();
    static void sweepUpdate();
    static void lengthUpdate();
    static void envelopeUpdate();

   private:
};
