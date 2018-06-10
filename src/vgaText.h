#ifndef _VGATEXT_H_
#define _VGATEXT_H_

#include "vga.h"

namespace Vga
{
    extern uint8_t cursor_x;
    extern uint8_t cursor_y;

	void select_font(const uint8_t* f);

    void showCursor();
    void hideCursor();
    void setCursorPosition(uint8_t x, uint8_t y);
	void print(const char *str, uint16_t color);
	void print(uint8_t c, uint16_t color);
	void printAt(uint8_t x, uint8_t y, const char* str, uint16_t color);
}

#endif