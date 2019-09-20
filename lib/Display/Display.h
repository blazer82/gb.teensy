#pragma once

#include <SPI.h>
#include <ILI9341_t3.h>

class Display : public ILI9341_t3
{
    public:
        Display(uint8_t _CS = 10, uint8_t _DC = 9, uint8_t _RST = 255, uint8_t _MOSI=11, uint8_t _SCLK=13, uint8_t _MISO=12) : ILI9341_t3(_CS, _DC, _RST, _MOSI, _SCLK, _MISO) {};
        void hLine(uint8_t y, uint16_t *lineData);
    protected:
    private:
};