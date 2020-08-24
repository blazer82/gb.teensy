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

#include <Arduino.h>
#include <CPU.h>
#include <FT81x.h>
#include <Memory.h>
#include <PPU.h>

// #define BENCHMARK_AFTER_CYCLES 10000000

void waitForKeyPress();

FT81x ft81x(1, 2, 3);

void setup() {
    Serial.begin(9600);

    SPI.begin();

    waitForKeyPress();
    Memory* mem = new Memory;
    mem->initMemory("game.rom");
    PPU::setMemoryHandle(mem);
    CPU::setMemoryHandle(mem);


    Serial.println("Enable display");
    ft81x.begin();

    delay(100);

    Serial.printf("REG_ID %x\n", ft81x.read8(FT81x_REG_ID));

    Serial.printf("REG_HCYCLE %i\n", ft81x.read16(FT81x_REG_HCYCLE));
    Serial.printf("REG_HSIZE %i\n", ft81x.read16(FT81x_REG_HSIZE));

    Serial.printf("REG_VCYCLE %i\n", ft81x.read16(FT81x_REG_VCYCLE));
    Serial.printf("REG_VSIZE %i\n", ft81x.read16(FT81x_REG_VSIZE));

    waitForKeyPress();
    ft81x.beginDisplayList();
    ft81x.clear(FT81x_COLOR_RGB(0, 0, 0));
    ft81x.drawBitmap(0, 0, 24, 160, 144, 3);
    ft81x.swapScreen();

    // waitForKeyPress();
    CPU::cpuEnabled = 1;
}

void loop() {
#ifdef BENCHMARK_AFTER_CYCLES
    uint64_t start = millis();
#endif
    while (true) {
        CPU::cpuStep();
        PPU::ppuStep(ft81x);
#ifdef BENCHMARK_AFTER_CYCLES
        if (CPU::totalCycles > BENCHMARK_AFTER_CYCLES) {
            uint64_t time = millis() - start;
            uint64_t hz = 1000 * CPU::totalCycles / time;
            Serial.printf("Emulated %lu Hz\n", hz);
            for (;;) {
            }
        }
#endif
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
