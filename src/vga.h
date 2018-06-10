#ifndef _VGA_H_
#define _VGA_H_

#include "vgacore.h"
#include "vgatext.h"

namespace Vga
{
	uint16_t hres();
	uint16_t vres();
    
	void draw_line(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t c);
    void draw_rect(uint8_t x0, uint8_t y0, uint8_t w, uint8_t h, char c, char fc);
	void set_pixel(uint8_t x, uint8_t y, char c);

	void bitmap(uint16_t x, uint16_t y, const unsigned char *bmp,
				uint16_t i, uint8_t width, uint8_t lines);
    void draw_char(const uint8_t* f, uint16_t x, uint16_t y, uint8_t c);
    void draw_text(const uint8_t* f, uint16_t x, uint16_t y, const char* str);
}

#endif