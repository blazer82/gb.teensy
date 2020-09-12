#pragma once

#include <stdint.h>

class IntervalTimer {
   public:
    constexpr IntervalTimer() {}
    ~IntervalTimer() {}
    bool begin(void (*funct)(), unsigned int microseconds) { return true; }
    bool begin(void (*funct)(), int microseconds) { return true; }
    bool begin(void (*funct)(), unsigned long microseconds) { return true; }
    bool begin(void (*funct)(), long microseconds) { return true; }
    bool begin(void (*funct)(), float microseconds) { return true; }
    bool begin(void (*funct)(), double microseconds) { return true; }
    void update(unsigned int microseconds) {}
    void update(int microseconds) {}
    void update(unsigned long microseconds) {}
    void update(long microseconds) {}
    void update(float microseconds) {}
    void update(double microseconds) {}
    void end();
    void priority(uint8_t n) {}
};
