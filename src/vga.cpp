#include <stm32f1xx_hal.h>
#include <string.h>

#include "vga.h"
#include "startup.h"
#include "font8x8.h"

namespace Vga
{
void draw_column(uint8_t row, uint16_t y0, uint16_t y1, uint8_t c);
void draw_row(uint8_t line, uint16_t x0, uint16_t x1, uint8_t c);
void sp(uint8_t x, uint8_t y, char c);
} 

void Vga::draw_char(const uint8_t *f, uint16_t x, uint16_t y, uint8_t c)
{
    c -= *(f + 2);
    bitmap(x, y, f, (c * *(f + 1)) + 3, *f, *(f + 1));
}

void Vga::draw_text(const uint8_t *f, uint16_t x, uint16_t y, const char *str)
{
    uint8_t width = *f;
    uint8_t height = *(f + 1);

    uint16_t currX = x;
    uint16_t currY = y;
    char *text = (char *)str;
    char c;
    while ((c = *text))
    {
        switch (c)
        {
        case '\0': //null
            break;
        case '\n': //line feed
            currY += height;
            currX = 0;
            break;
        default:
            draw_char(f, currX, currY, c);
            currX += width;
            break;
        }

        text++;
    }
}

void Vga::draw_rect(uint8_t x0, uint8_t y0, uint8_t w, uint8_t h, char c, char fc)
{

    if (fc != -1)
    {
        for (unsigned char i = y0; i < y0 + h; i++)
        {
            draw_row(i, x0, x0 + w, fc);
        }
    }
    draw_line(x0, y0, x0 + w, y0, c);
    draw_line(x0, y0, x0, y0 + h, c);
    draw_line(x0 + w, y0, x0 + w, y0 + h, c);
    draw_line(x0, y0 + h, x0 + w, y0 + h, c);
}

/* place a bitmap at x,y where the bitmap is defined as {width,height,imagedata....}
 *
 * Arguments:
 *	x:
 *		The x coordinate of the upper left corner.
 *	y:
 *		The y coordinate of the upper left corner.
 *	bmp:
 *		The bitmap data to print.
 *	i:
 *		The offset into the image data to start at.  This is mainly used for fonts.
 *		default	=0
 *	width:
 *		Override the bitmap width. This is mainly used for fonts.
 *		default =0 (do not override)
 *	height:
 *		Override the bitmap height. This is mainly used for fonts.
 *		default	=0 (do not override)
*/
void Vga::bitmap(uint16_t x, uint16_t y, const unsigned char *bmp,
                 uint16_t i, uint8_t width, uint8_t lines)
{

    uint8_t temp, lshift, rshift, save, xtra;
    volatile uint8_t *si;

    rshift = x & 7;
    lshift = 8 - rshift;
    if (width == 0)
    {
        width = *(const uint8_t *)((uint32_t)(bmp) + i);
        i++;
    }
    if (lines == 0)
    {
        lines = *(const uint8_t *)((uint32_t)(bmp) + i);
        i++;
    }

    if (width & 7)
    {
        xtra = width & 7;
        width = width / 8;
        width++;
    }
    else
    {
        xtra = 8;
        width = width / 8;
    }

    for (uint8_t l = 0; l < lines; l++)
    {
        si = Vga::GetBitmapAddress(y + l) + x / 8;
        if (width == 1)
        {
            temp = 0xff >> (rshift + xtra);
        }
        else
        {
            temp = 0;
        }

        save = *si;
        *si &= ((0xff << lshift) | temp);
        temp = *(const uint8_t *)((uint32_t)(bmp) + i++);
        *si |= temp >> rshift;
        si++;
        for (uint16_t b = i + width - 1; i < b; i++)
        {
            save = *si;
            *si = temp << lshift;
            temp = *(const uint8_t *)((uint32_t)(bmp) + i);
            *si |= temp >> rshift;
            si++;
        }

        if (rshift + xtra < 8)
        {
            *(si - 1) |= (save & (0xff >> (rshift + xtra))); //test me!!!
        }

        if (rshift + xtra - 8 > 0)
        {
            *si &= (0xff >> (rshift + xtra - 8));
        }

        *si |= temp << lshift;
    }
} // end of bitmap

uint16_t Vga::hres()
{
    return HSIZE_PIXELS;
}

uint16_t Vga::vres()
{
    return VSIZE_PIXELS;
}

/* Fill a row from one point to another
 *
 * Argument:
 *	line:
 *		The row that fill will be performed on.
 *	x0:
 *		edge 0 of the fill.
 *	x1:
 *		edge 1 of the fill.
 *	c:
 *		the color of the fill.
 *		(see color note at the top of this file)
*/
void Vga::draw_row(uint8_t line, uint16_t x0, uint16_t x1, uint8_t c)
{
    uint8_t lbit, rbit;
    volatile uint8_t *pos0;
    volatile uint8_t *pos1;

    if (x0 == x1)
    {
        Vga::set_pixel(x0, line, c);
    }
    else
    {
        if (x0 > x1)
        {
            lbit = x0;
            x0 = x1;
            x1 = lbit;
        }
        lbit = 0xff >> (x0 & 7);
        pos0 = Vga::GetBitmapAddress(line, x0 / 8);
        rbit = ~(0xff >> (x1 & 7));
        pos1 = Vga::GetBitmapAddress(line, x1 / 8);
        if (x0 == x1)
        {
            lbit = lbit & rbit;
            rbit = 0;
        }
        if (c == SET)
        {
            *pos0 |= lbit;
            pos0++;
            while (pos0 < pos1)
            {
                *pos0 = 0xff;
                pos0++;
            }
            *pos0 |= rbit;
        }
        else if (c == PIXEL_CLEAR)
        {
            *pos0 &= ~lbit;
            pos0++;
            while (pos0 < pos1)
            {
                *pos0 = 0;
                pos0++;
            }
            *pos0 &= ~rbit;
        }
        else if (c == PIXEL_INVERT)
        {
            *pos0 ^= lbit;
            pos0++;
            while (pos0 < pos1)
            {
                *pos0 ^= 0xff;
                pos0++;
            }
            *pos0 ^= rbit;
        }
    }
} // end of draw_row

/* Fill a column from one point to another
 *
 * Argument:
 *	row:
 *		The row that fill will be performed on.
 *	y0:
 *		edge 0 of the fill.
 *	y1:
 *		edge 1 of the fill.
 *	c:
 *		the color of the fill.
 *		(see color note at the top of this file)
*/
void Vga::draw_column(uint8_t row, uint16_t y0, uint16_t y1, uint8_t c)
{
    unsigned char bit;
    volatile uint8_t *byte;

    if (y0 == y1)
    {
        set_pixel(row, y0, c);
    }
    else
    {
        if (y1 < y0)
        {
            bit = y0;
            y0 = y1;
            y1 = bit;
        }
        bit = 0x80 >> (row & 7);
        uint8_t charRow = row / 8;
        byte = Vga::GetBitmapAddress(y0, charRow);
        if (c == SET)
        {
            while (y0 <= y1)
            {
                *byte |= bit;
                y0++;
                byte = Vga::GetBitmapAddress(y0, charRow);
            }
        }
        else if (c == PIXEL_CLEAR)
        {
            while (y0 <= y1)
            {
                *byte &= ~bit;
                y0++;
                byte = Vga::GetBitmapAddress(y0, charRow);
            }
        }
        else if (c == PIXEL_INVERT)
        {
            while (y0 <= y1)
            {
                *byte ^= bit;
                y0++;
                byte = Vga::GetBitmapAddress(y0, charRow);
            }
        }
    }
}

void Vga::draw_line(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t c)
{
    if (x0 > HSIZE_PIXELS || y0 > VSIZE_PIXELS || x1 > HSIZE_PIXELS || y1 > VSIZE_PIXELS)
    {
        return;
    }
    if (x0 == x1)
    {
        draw_column(x0, y0, y1, c);
    }
    else if (y0 == y1)
    {
        draw_row(y0, x0, x1, c);
    }
    else
    {
        int e;
        signed int dx, dy, j, temp;
        signed char s1, s2, xchange;
        signed int x, y;

        x = x0;
        y = y0;

        //take absolute value
        if (x1 < x0)
        {
            dx = x0 - x1;
            s1 = -1;
        }
        else if (x1 == x0)
        {
            dx = 0;
            s1 = 0;
        }
        else
        {
            dx = x1 - x0;
            s1 = 1;
        }

        if (y1 < y0)
        {
            dy = y0 - y1;
            s2 = -1;
        }
        else if (y1 == y0)
        {
            dy = 0;
            s2 = 0;
        }
        else
        {
            dy = y1 - y0;
            s2 = 1;
        }

        xchange = 0;

        if (dy > dx)
        {
            temp = dx;
            dx = dy;
            dy = temp;
            xchange = 1;
        }

        e = ((int)dy << 1) - dx;

        for (j = 0; j <= dx; j++)
        {
            sp(x, y, c);

            if (e >= 0)
            {
                if (xchange == 1)
                    x = x + s1;
                else
                    y = y + s2;
                e = e - ((int)dx << 1);
            }
            if (xchange == 1)
                y = y + s2;
            else
                x = x + s1;
            e = e + ((int)dy << 1);
        }
    }
}

/* Set the color of a pixel
 * 
 * Arguments:
 *	x:
 *		The x coordinate of the pixel.
 *	y:
 *		The y coordinate of the pixel.
 *	c:
 *		The color of the pixel
 *		(see color note at the top of this file)
 */
void Vga::set_pixel(uint8_t x, uint8_t y, char c)
{
    if (x >= HSIZE_PIXELS || y >= VSIZE_PIXELS)
    {
        return;
    }
    sp(x, y, c);
} // end of set_pixel

/* Inline version of set_pixel that does not perform a bounds check
 * This function will be replaced by a macro.
*/
void inline Vga::sp(uint8_t x, uint8_t y, char c)
{
    if (c == SET)
    {
        *Vga::GetBitmapAddress(y, x / 8) |= 0x80 >> (x & 7);
    }
    else if (c == PIXEL_CLEAR)
    {
        *Vga::GetBitmapAddress(y, x / 8) &= ~0x80 >> (x & 7);
    }
    else
    {
        *Vga::GetBitmapAddress(y, x / 8) ^= 0x80 >> (x & 7);
    }
} // end of sp
