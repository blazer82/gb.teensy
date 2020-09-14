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

#ifndef PLATFORM_NATIVE

// This is the main entry point for the Teensy microcontroller.
// Compile and upload to the Teensy development board in order to run the emulator.

#include <APU.h>
#include <Arduino.h>
#include <CPU.h>
#include <Cartridge.h>
#include <FT81x.h>
#include <Joypad.h>
#include <Memory.h>
#include <PPU.h>
#include <SerialDataTransfer.h>

void waitForKeyPress();

FT81x ft81x = FT81x(10, 9, 8);

static char title[17];  // 16 chars for name, 1 for null terminator

void setup() {
    Serial.begin(115200);

    SPI.begin();

    waitForKeyPress();

    Serial.println("Enable display");
    ft81x.begin();

    Serial.printf("\nStart Gameboy...\n");

    //Cartridge::begin("mario.gb");
    Cartridge::begin("tim11.gb");
    //artridge::begin("mario.gb");
    Cartridge::getGameName(title);

    Memory::initMemory();
    CPU::cpuEnabled = 1;

    ft81x.beginDisplayList();
    ft81x.clear(FT81x_COLOR_RGB(0, 0, 0));
    ft81x.drawText(10, 460, 16, FT81x_COLOR_RGB(255, 0, 255), 0, title);
    ft81x.drawText(470, 460, 16, FT81x_COLOR_RGB(255, 0, 255), FT81x_OPT_RIGHTX, "Emulated speed: ...\0");
    ft81x.drawBitmap(0, 0, 0, 160, 144, 3);
    ft81x.swapScreen();

    APU::begin();
    Joypad::begin();
}

void loop() {
    uint64_t start = millis();

    while (true) {
        CPU::cpuStep();
        PPU::ppuStep(ft81x);
        APU::apuStep();
        SerialDataTransfer::serialStep();
        Joypad::joypadStep();

        if ((CPU::totalCycles % 1000000) == 0) {
            uint64_t time = millis() - start;
            uint64_t hz = 1000 * CPU::totalCycles / time;
            uint8_t speed = hz / 10000;
            char buff[21];
            sprintf(buff, "Emulated speed: %d%%", speed);
            ft81x.beginDisplayList();
            ft81x.clear(FT81x_COLOR_RGB(0, 0, 0));
            ft81x.drawText(10, 460, 16, FT81x_COLOR_RGB(255, 0, 255), 0, title);
            ft81x.drawText(470, 460, 16, FT81x_COLOR_RGB(255, 0, 255), FT81x_OPT_RIGHTX, buff);
            ft81x.drawBitmap(0, 0, 0, 160, 144, 3);
            ft81x.swapScreen();
        }
    }
}

void waitForKeyPress() {
    Serial.println("\nPress a key to continue\n");
    while (!Serial.available()) {
    }
    while (Serial.available()) {
        Serial.read();
    }
}

#else

// The code down here is for automated runs of ROMs on a host computer (e.g. Linux)
// It depends on some mocked Arduino libraries currently placed inside the test directory
// as well as ROM data, also placed inside the test directory.
//
// Example usage:
// > pio run -e native
// > .pio/build/native/program 0 70000000
//
// The commands above will run the ROM data at ROM::getRom(0) for 70000000 cycles.
// All the Serial output is printed to stdout.

#include <Arduino.h>
#include <CPU.h>
#include <Memory.h>
#include <PPU.h>
#include <SD.h>
#include <SerialDataTransfer.h>
#include <rom.h>

SDClass SD;
StdioSerial Serial;
FT81x ft81x = FT81x(10, 9, 8);

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Invalid argument count %i instead of 3.\n", argc);
        printf("Usage: program [rom index] [cycle count]\n");
        return 1;
    }

    const unsigned int romIndex = atoi(argv[1]);
    const unsigned long cycleCount = atol(argv[2]);

    Cartridge::begin(ROM::getRom(romIndex));
    Memory::initMemory();
    CPU::cpuEnabled = 1;

    while (CPU::totalCycles < cycleCount) {
        CPU::cpuStep();
        PPU::ppuStep(ft81x);
        SerialDataTransfer::serialStep();
    }

    return 0;
}

#endif
