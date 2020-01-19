/**
 * This file is part of the gb.teensy emulator
 * <https://github.com/blazer82/gb.teensy>.
 * 
 * It is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * It is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 **/

#include <Arduino.h>
#include "CPU.h"
#include "PPU.h"

//#define BENCHMARK_AFTER_CYCLES 20000000

PPU ppu;

void setup()
{
    // initialize LED digital pin as an output.
    pinMode(LED_BUILTIN, OUTPUT);
    Serial.begin(115200);

    ppu = PPU();

    digitalWrite(LED_BUILTIN, HIGH);
}

void loop()
{
#ifdef BENCHMARK_AFTER_CYCLES
    uint64_t start = millis();
#endif

    for(;;) {
        ppu.ppuStep();
        CPU::cpuStep();

#ifdef BENCHMARK_AFTER_CYCLES
        if (CPU::totalCycles > BENCHMARK_AFTER_CYCLES) {
            uint64_t time = millis() - start;
            uint64_t hz = 1000 * CPU::totalCycles / time;
            Serial.printf("Emulated %lu Hz\n", hz);
            for (;;) {}
        }
#endif
    }
}
