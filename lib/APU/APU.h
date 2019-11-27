#pragma once

class APU
{
    public:
        APU();
        void apuStep();
    protected:
        float getDuty(uint8_t memInput);
    private:
};
