#pragma once

class APU
{
    public:
        static void init();
        static void apuStep();
    protected:
        static float getDuty(uint8_t memInput);
    private:
};
