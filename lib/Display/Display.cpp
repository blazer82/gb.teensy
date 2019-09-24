#include "Display.h"

void Display::hLine(uint16_t y, uint16_t x, uint16_t *lineData)
{
    beginSPITransaction();
	setAddr(x, y, x + 159, y);
	writecommand_cont(ILI9341_RAMWR);
    for (uint16_t w = 0; w < 159; w++) {
		writedata16_cont(lineData[w]);
	}
	writedata16_last(lineData[159]);
	endSPITransaction();
}