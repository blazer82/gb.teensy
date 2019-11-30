#include <Audio.h>
#include <Wire.h>
#include "APU.h"
#include "Memory.h"

AudioSynthWaveform waveform1;
AudioSynthWaveform waveform2;

AudioMixer4 mixerL;
AudioMixer4 mixerR;

AudioOutputI2S i2s1;

AudioConnection patchCord1(waveform1, 0, mixerL, 0);
AudioConnection patchCord2(waveform1, 0, mixerR, 0);
AudioConnection patchCord3(waveform2, 0, mixerL, 1);
AudioConnection patchCord4(waveform2, 0, mixerR, 1);
AudioConnection patchCord5(mixerL, 0, i2s1, 0);
AudioConnection patchCord6(mixerR, 0, i2s1, 1);

AudioControlSGTL5000 sgtl5000_1;

APU::APU() {
    AudioMemory(20); // TODO determine how much is needed
    sgtl5000_1.enable();
    sgtl5000_1.volume(0.32);

    // Test sound
    waveform1.begin(WAVEFORM_SQUARE);
    waveform1.amplitude(0.2);
    waveform1.frequency(200);
    waveform1.pulseWidth(0.15);

    waveform2.begin(WAVEFORM_SQUARE);
    waveform2.amplitude(0.2);
    waveform2.frequency(1200);
    waveform2.pulseWidth(0.25);
}

void APU::apuStep() {
    uint8_t square1Duty = Memory::readByte(MEM_SOUND_NR11) >> 6;
    uint16_t square1Freq = ((Memory::readByte(MEM_SOUND_NR14) && 0x3) << 8) | Memory::readByte(MEM_SOUND_NR13);

    uint8_t square2Duty = Memory::readByte(MEM_SOUND_NR21) >> 6;
    uint16_t square2Freq = ((Memory::readByte(MEM_SOUND_NR24) && 0x3) << 8) | Memory::readByte(MEM_SOUND_NR23);

    waveform1.pulseWidth(getDuty(square1Duty));
    waveform1.frequency(square1Freq);

    waveform2.pulseWidth(getDuty(square2Duty));
    waveform2.frequency(square2Freq);
}

float APU::getDuty(uint8_t memInput) {
    switch (memInput)
    {
    case 1:
        return 0.25;

    case 2:
        return 0.5;

    case 3:
        return 0.75;
    
    default:
        return 0.125;
    }
}