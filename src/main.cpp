#include <Arduino.h>
#include "CPU.h"
#include "PPU.h"

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
    for(;;) {
        ppu.ppuStep();
        CPU::cpuStep();
    }
}
