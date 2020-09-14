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

#pragma once

#include <Arduino.h>

class Timer{
   public:
    // Perform a single step of the timer
    static void timerStep();

    // Note to future Grant
    // For this to work, these reads and writes need to happen before 
    // cpuStep
    // Manipulating DIV 
    static uint8_t readDiv();
    static void writeDiv(uint8_t data);

    // Manipulating TIMA
    static uint8_t readTima();
    static void writeTima(uint8_t data);

    // Manipulating TMA
    static uint8_t readTma();
    static void writeTma(uint8_t data);

    // Manipulating TAC
    static uint8_t readTac();
    static void writeTac(uint8_t data);

    // Check if an interrupt has been requested
    static bool checkInt();
    // Clear the interrupt bit
    static void clearInt();
    // Set the interrupt bit
    static void setInt();
    
   private:
    // So much of the timer logic is based on falling
    // edges, so we need to keep copies of the previous
    // register values to check for that
    
    // The divider register. Reading from 0xFF04 
    // will get the top 8 bits of this register
    // This is the actual system internal timer too
    static uint16_t div;
    static uint16_t divPrev;

    // The TAC register, used to control the timer
    static uint8_t tac;
    static uint8_t tacPrev;

    // The TIMA register, where all the magic happens
    static uint8_t tima;
    static uint8_t timaPrev;

    // The TMA register, timer modulo
    static uint8_t tma;
    static uint8_t tmaPrev;

    // The timer interrupt request flag
    static bool timerInt;

    // There are some glitched conditions where TIMA will
    // need to be increased on the next cycle to match the 
    // hardware behavior. This keeps track of that
    static bool timaGlitch;

    // TIMA overflow doesn't cause immediate changes, it
    // happens one cycle (4 clocks) later. This keeps 
    // track of how many cycles until overflow changes
    // need to occur. It it is -1, no overflow happened yet
    static int8_t timaOverflowCountdown;
};

// Lookup table to see which DIV bit controls the TIMA increment
// based on bits 0, 1 in TAC
const uint8_t tacDivBit[] = {9, 3, 5, 7};