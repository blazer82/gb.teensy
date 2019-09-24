#include <Arduino.h>
#include "CPU.h"
#include "PPU.h"

//#define BENCHMARK

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
#ifdef BENCHMARK
    uint64_t start = millis();
#endif

    for(;;) {
        ppu.ppuStep();
        CPU::cpuStep();

#ifdef BENCHMARK
        if (CPU::totalCycles > 10000000) {
            uint64_t time = millis() - start;
            uint64_t hz = 1000 * CPU::totalCycles / time;
            Serial.printf("Emulated %lu Hz\n", hz);
            for (;;) {}
        }
#endif
    }
}
