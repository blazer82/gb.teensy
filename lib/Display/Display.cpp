/**
 * This file is part of the gb.teensy emulator
 * <https://github.com/blazer82/gb.teensy>.
 * 
 * This is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 **/

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