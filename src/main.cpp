/**
 * FT81x on ST7701S Arduino Driver
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
#include <FT81x.h>
#include <CPU.h>
#include <PPU.h>

void waitForKeyPress();

static PPU ppu;

void setup() {
    Serial.begin(9600);

    SPI.begin();

    waitForKeyPress();

    ppu = PPU();

    Serial.println("Enable display");
    FT81x::init();

    delay(100);

    Serial.printf("REG_ID %x\n", FT81x::read8(FT81x_REG_ID));

    Serial.printf("REG_HCYCLE %i\n", FT81x::read16(FT81x_REG_HCYCLE));
    Serial.printf("REG_HSIZE %i\n", FT81x::read16(FT81x_REG_HSIZE));

    Serial.printf("REG_VCYCLE %i\n", FT81x::read16(FT81x_REG_VCYCLE));
    Serial.printf("REG_VSIZE %i\n", FT81x::read16(FT81x_REG_VSIZE));

    // waitForKeyPress();
    FT81x::begin();
    FT81x::clear(FT81x_COLOR_RGB(0, 0, 0));
    FT81x::drawBitmap(0, 0, 0, 160, 160, 3);
    FT81x::swap();

    // waitForKeyPress();
}

void loop() {
    while (true) {
        CPU::cpuStep();
        ppu.ppuStep();
    }
}

void waitForKeyPress() {
    Serial.println("\nPress a key to continue\n");
    while (!Serial.available()) {}
    while (Serial.available()) {
        Serial.read();
    }
}
