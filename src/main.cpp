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

#include <APU.h>
#include <Arduino.h>
#include <CPU.h>
#include <FT81x.h>
#include <Memory.h>
#include <PPU.h>

void waitForKeyPress();
void printDiagnostics();

FT81x ft81x = FT81x(10, 9, 8);

static char title[16];

#define JOYPAD_START 16
#define JOYPAD_LEFT  17
#define JOYPAD_RIGHT 18
#define JOYPAD_DOWN  19
#define JOYPAD_A     20

void setup() {
    Serial.begin(9600);

    SPI.begin();

    pinMode(JOYPAD_START, INPUT);
    pinMode(JOYPAD_LEFT, INPUT);
    pinMode(JOYPAD_RIGHT, INPUT);
    pinMode(JOYPAD_DOWN, INPUT);
    pinMode(JOYPAD_A, INPUT);

    // waitForKeyPress();

    Serial.println("Enable display");
    ft81x.begin();

    printDiagnostics();

    Serial.println("");
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

    APU::begin();
}

void loop() {
    uint64_t start = millis();

    while (true) {
        CPU::cpuStep();
        PPU::ppuStep(ft81x);
        APU::apuStep();

        uint8_t joypad = Memory::readByte(MEM_JOYPAD);
        if ((joypad & 0x10) == 0) {
            bool left = digitalReadFast(JOYPAD_LEFT);
            bool right = digitalReadFast(JOYPAD_RIGHT);
            bool down = digitalReadFast(JOYPAD_DOWN);
            joypad = (joypad & 0xF0) | 0x4 | (down << 3) | (left << 1) | right;
            Memory::writeByteInternal(MEM_JOYPAD, joypad, true);
        }
        if ((joypad & 0x20) == 0) {
            bool start = digitalReadFast(JOYPAD_START);
            bool a = digitalReadFast(JOYPAD_A);
            bool b = 1;  // digitalReadFast(JOYPAD_B);
            joypad = (joypad & 0xF0) | 0x4 | (start << 3) | (b << 1) | a;
            Memory::writeByteInternal(MEM_JOYPAD, joypad, true);
        }

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

void printDiagnostics() {
    Serial.print("SPI clock set to: ");
    Serial.println(FT81x_SPI_CLOCK_SPEED);

    Serial.println("");

    Serial.println("Read chip ID...");

    const uint32_t chipID = ft81x.read32(0x0C0000);

    Serial.print("0x0C0000: ");
    Serial.print(chipID & 0xFF, HEX);
    Serial.println(" (supposed to be 0x8)");

    Serial.print("0x0C0001: ");
    Serial.print((chipID >> 8) & 0xFF, HEX);
    Serial.println(" (supposed to be 0x12 or 0x13)");

    Serial.print("0x0C0002: ");
    Serial.print((chipID >> 16) & 0xFF, HEX);
    Serial.println(" (supposed to be 0x1)");

    Serial.print("0x0C0003: ");
    Serial.print((chipID >> 24) & 0xFF, HEX);
    Serial.println(" (supposed to be 0x0)");

    Serial.println("");

    Serial.println("Read FT81x configuration...");

    Serial.print("REG_ID ");
    Serial.print(ft81x.read8(FT81x_REG_ID), HEX);
    Serial.println(" (supposed to be 0x7C)");

    Serial.print("REG_HCYCLE ");
    Serial.print(ft81x.read16(FT81x_REG_HCYCLE));
    Serial.println(" (supposed to be 548)");

    Serial.print("REG_HSIZE ");
    Serial.print(ft81x.read16(FT81x_REG_HSIZE));
    Serial.println(" (supposed to be 480)");

    Serial.print("REG_VCYCLE ");
    Serial.print(ft81x.read16(FT81x_REG_VCYCLE));
    Serial.println(" (supposed to be 518)");

    Serial.print("REG_VSIZE ");
    Serial.print(ft81x.read16(FT81x_REG_VSIZE));
    Serial.println(" (supposed to be 480)");

    Serial.println("");

    Serial.println("Read display parameters...");

    Serial.print("Power mode: ");
    Serial.print(ft81x.queryDisplay(ST7701_RDDPM), HEX);
    Serial.println(" (supposed to be 0x9C)");
}
