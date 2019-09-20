#include "Display.h"

void Display::hLine(uint8_t y, uint16_t color)
{
    beginSPITransaction();
	setAddr(0, y, 160, y);
	writecommand_cont(ILI9341_RAMWR);
    for (uint8_t w = 160; w > 1; w--) {
		writedata16_cont(color);
	}
	writedata16_last(color);
	endSPITransaction();
}