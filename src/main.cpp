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
