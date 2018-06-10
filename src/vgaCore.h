#ifndef _VGACORE_H_
#define _VGACORE_H_

#include <stdint.h>

#include "timing.h"

// Horizontal resolution
#define	HSIZE_CHARS   32
#define	HSIZE_PIXELS  (HSIZE_CHARS * 8)

// Vertical resolution
#define	VSIZE_CHARS   24  
#define	VSIZE_PIXELS  (VSIZE_CHARS * 8)
#define REPEAT_LINES 2

#define	BITMAP_SIZE (HSIZE_CHARS * VSIZE_PIXELS)
#define	COLORS_SIZE (HSIZE_CHARS * VSIZE_CHARS)

#define MODE_TVOUT 0
#define MODE_SINCLAIR 1

/*
	Pinout
	======

	PB6 - VSync
	PB0 - HSync

	PA0, PA1 - Red
	PA2, PA3 - Green
	PA4, PA5 - Blue

	Resistors
	=========
	680 Ohm on PA0, PA2, PA4 (0.33 V)
	470 Ohm on PA1, PA2, PA3
	====
	Parallel: 278 Ohm (0.7 V)

*/

#define PIXEL_CLEAR 0
#define PIXEL_SET 1
#define PIXEL_INVERT 2

namespace Vga
{
    extern volatile uint8_t VideoMemoryPixels[];
    extern volatile uint16_t VideoMemoryColors[];
    extern uint8_t BitmapMode;

	void InitVga(Timing timing);

    volatile uint8_t* GetBitmapAddress(uint8_t vline);
    volatile uint8_t* GetBitmapAddress(uint8_t vline, uint8_t character);
    uint16_t ConvertSinclairColor(uint8_t sinclairColors);

    void delay(uint32_t x);
    unsigned long millis();
    void delay_frame();
	void clear_screen(uint16_t color);
}

#endif