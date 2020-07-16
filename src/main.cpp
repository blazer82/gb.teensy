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

void waitForKeyPress();

FT81x ft81x;

static char title[16];

void setup() {
    Serial.begin(9600);

    SPI.begin();

    waitForKeyPress();

    Serial.println("Enable display");
    ft81x.begin();

    delay(100);

    Serial.printf("REG_ID %x\n", ft81x.read8(FT81x_REG_ID));

    Serial.printf("REG_HCYCLE %i\n", ft81x.read16(FT81x_REG_HCYCLE));
    Serial.printf("REG_HSIZE %i\n", ft81x.read16(FT81x_REG_HSIZE));

    Serial.printf("REG_VCYCLE %i\n", ft81x.read16(FT81x_REG_VCYCLE));
    Serial.printf("REG_VSIZE %i\n", ft81x.read16(FT81x_REG_VSIZE));

    waitForKeyPress();

    Serial.println("Start Gameboy...");

    Memory::initMemory();
    CPU::cpuEnabled = 1;

    Memory::getTitle(title);

    ft81x.beginDisplayList();
    ft81x.clear(FT81x_COLOR_RGB(0, 0, 0));
    ft81x.drawText(10, 460, 16, FT81x_COLOR_RGB(255, 0, 255), 0, title);
    ft81x.drawText(470, 460, 16, FT81x_COLOR_RGB(255, 0, 255), FT81x_OPT_RIGHTX, "Emulated speed: ...\0");
    ft81x.drawBitmap(0, 0, 0, 160, 144, 3);
    ft81x.swapScreen();
}

void loop() {
    uint64_t start = millis();

    while (true) {
        CPU::cpuStep();
        PPU::ppuStep(ft81x);

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
