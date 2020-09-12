/*
 * Copyright (c) 2019 Brian T. Park
 * MIT License
 *
 * Parts derived from the Arduino SDK
 * Copyright (c) 2005-2013 Arduino Team
 */

#pragma once

#include <inttypes.h>
#include <time.h>    // clock_gettime()
#include <unistd.h>  // usleep()

#include "IntervalTimer.h"
#include "Print.h"
#include "SPI.h"
#include "StdioSerial.h"

// xx.yy.zz => xxyyzz (without leading 0)
#define UNIX_HOST_DUINO_VERSION        300
#define UNIX_HOST_DUINO_VERSION_STRING "0.3.0"

// Used by digitalRead() and digitalWrite()
#define HIGH 0x1
#define LOW  0x0

// Used by pinMode()
#define INPUT        0x0
#define OUTPUT       0x1
#define INPUT_PULLUP 0x2

// Various math constants.
#define PI         3.1415926535897932384626433832795
#define HALF_PI    1.5707963267948966192313216916398
#define TWO_PI     6.283185307179586476925286766559
#define DEG_TO_RAD 0.017453292519943295769236907684886
#define RAD_TO_DEG 57.295779513082320876798154814105
#define EULER      2.718281828459045235360287471352

#define SERIAL  0x0
#define DISPLAY 0x1

#define LSBFIRST 0
#define MSBFIRST 1

#define CHANGE  1
#define FALLING 2
#define RISING  3

// Arbitrarily define the pin for the LED_BUILTIN
#define LED_BUILTIN 1

extern "C" {

inline void delay(unsigned long ms) { usleep(ms * 1000); }
inline void yield();
inline unsigned long millis() {
    struct timespec spec;
    clock_gettime(CLOCK_MONOTONIC, &spec);
    unsigned long ms = spec.tv_sec * 1000U + spec.tv_nsec / 1000000UL;
    return ms;
}
inline unsigned long micros() {
    struct timespec spec;
    clock_gettime(CLOCK_MONOTONIC, &spec);
    unsigned long us = spec.tv_sec * 1000000UL + spec.tv_nsec / 1000U;
    return us;
}
inline void digitalWrite(uint8_t pin, uint8_t val) {}
inline void digitalWriteFast(uint8_t pin, uint8_t val) {}
inline int digitalRead(uint8_t pin) { return 0; }
inline int digitalReadFast(uint8_t pin) { return 0; }
inline void pinMode(uint8_t pin, uint8_t mode) {}
inline int analogWrite(uint8_t pin, uint8_t amplitude) { return 0; }
inline int analogRead(uint8_t pin) { return 0; }
inline void analogWriteFrequency(uint8_t pin, float frequency) {}
inline uint32_t analogWriteResolution(uint32_t bits) { return bits; }
inline void interrupts(){};
inline void noInterrupts(){};
}
