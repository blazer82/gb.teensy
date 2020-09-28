/**
 * gb.teensy Emulation Software
 * Copyright (C) 2020  Raphael Stäbler, Grant Haack
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

#include "Timer.h"

#include <Arduino.h>

uint16_t Timer::div = 0xABCC;
uint16_t Timer::divPrev = 0xABCC;

uint8_t Timer::tac = 0;
uint8_t Timer::tacPrev = 0;

uint8_t Timer::tima = 0;
uint8_t Timer::timaPrev = 0;

uint8_t Timer::tma = 0;
uint8_t Timer::tmaPrev = 0;

bool Timer::timerInt = 0;

bool Timer::timaGlitch = 0;

int8_t Timer::timaOverflowCountdown = -1;

uint8_t Timer::readDiv() { return (div & 0xFF00) >> 8; }

uint8_t Timer::readTima() { return tima; }

uint8_t Timer::readTma() { return tma; }

uint8_t Timer::readTac() { return tac; }

bool Timer::checkInt() { return timerInt; }

void Timer::clearInt() { timerInt = false; }

void Timer::setInt() { timerInt = true; }

void Timer::writeDiv(uint8_t data) {
    // No matter what, all writes to DIV just clear DIV
    divPrev = div;
    div = 0;
    // Look for falling edges in DIV bits to see if the
    // timer glitch will need to happen
    // Check to see if the previous DIV bit that TAC is
    // pointing to was 1
    if (divPrev & 1 << tacDivBit[tac & 3]) {
        // If it is 1, then changing it to 0 will cause
        // a falling edge and make TIMA increment
        timaGlitch = true;
    }
}

void Timer::writeTima(uint8_t data) {
    timaPrev = tima;
    tima = data;
}

void Timer::writeTma(uint8_t data) {
    tmaPrev = tma;
    tma = data;
}

void Timer::writeTac(uint8_t data) {
    tacPrev = tac;
    tac = data;
}

void Timer::timerStep() {
    // DIV increments 4 times for each clock cycle
    for (uint i = 0; i < 4; i++) {
        divPrev = div;
        div += 1;
        // Check to see if TIMA should be incremented because
        // of a glitch
        if (timaGlitch) {
            timaGlitch = false;
            timaPrev = tima;
            tima++;
        } else {
            // Check if the timer is enabled
            if (tac & 0x04) {
                // Check to see if TIMA should be incremented normally
                // TIMA is incremented if the current bit being pointed
                // to by TAC transitions from 1 to 0
                if (((divPrev & (1 << tacDivBit[tac & 0x03])) == (1 << tacDivBit[tac & 0x03])) & ((div & (1 << tacDivBit[tac & 0x03])) == 0)) {
                    timaPrev = tima;
                    tima++;
                }
            }
        }
        // Check to see if TIMA has overflowed
        if ((timaPrev == 0xFF) && (tima == 0x00)) {
            timaPrev = tima;
            tima = tma;
            timerInt = true;
        }
    }
}