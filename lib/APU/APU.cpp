#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include "APU.h"

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
    waveform1.begin(WAVEFORM_TRIANGLE);
    waveform1.amplitude(0.5);
    waveform1.frequency(200);
    waveform1.pulseWidth(0.15);

    waveform2.begin(WAVEFORM_SINE);
    waveform2.amplitude(0.6);
    waveform2.frequency(1200);
    waveform2.pulseWidth(0.25);
}

void APU::apuStep() {
}