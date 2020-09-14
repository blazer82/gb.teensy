#pragma once

#include <functional>

namespace TeensyTimerTool {

#define TMR1 0
#define TMR2 0
#define TMR3 0
#define TMR4 0
#define GPT1 0
#define GPT2 0

enum class errorCode { OK = 0 };

using callback_t = std::function<void(void)>;

class PeriodicTimer {
   public:
    PeriodicTimer(int) {}
    inline errorCode begin(callback_t callback, int period, bool start = true) { return errorCode::OK; }
    inline errorCode end() { return errorCode::OK; }
    inline errorCode start() { return errorCode::OK; }
    inline errorCode stop() { return errorCode::OK; }
};

}  // namespace TeensyTimerTool
