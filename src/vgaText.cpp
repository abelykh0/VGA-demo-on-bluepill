#include <usb/usbd_def.h>

#include "vgatext.h"

namespace Vga
{
    uint8_t cursor_x;
    uint8_t cursor_y;

    static const uint8_t* font;
    static bool isCursorVisible = false;
    
    void cursorNext();
	void print_char(uint8_t x, uint8_t y, unsigned char c, uint16_t color);
    void InvertColor();
}

void Vga::cursorNext()
{
    uint8_t x = Vga::cursor_x;
    uint8_t y = Vga::cursor_y;
    if (x < HSIZE_CHARS - 1)
    {
        x++;
    }
    else
    {
        if (y < VSIZE_CHARS - 1)
        {
            x = 0;
            y++;
        }
    }
    Vga::setCursorPosition(x, y);
}

void Vga::showCursor()
{
    if (!isCursorVisible)
    {
        isCursorVisible = true;
        InvertColor();
    }
}

void Vga::hideCursor()
{
    if (isCursorVisible)
    {
        isCursorVisible = false;
        InvertColor();
    }
}

void Vga::select_font(const unsigned char *f)
{
    Vga::font = f;
}

/*
 * print an 8x8 char c at x,y
 */
void Vga::print_char(uint8_t x, uint8_t y, unsigned char c, uint16_t color)
{
    draw_char(Vga::font, x * 8, y * 8, c);
    Vga::VideoMemoryColors[y * HSIZE_CHARS + x] = color;
}

void Vga::setCursorPosition(uint8_t x, uint8_t y)
{
    if (cursor_x == x && cursor_y == y)
    {
        return;
    }

    if (isCursorVisible)
    {
        InvertColor();
    }

    cursor_x = x;
    cursor_y = y;

    if (isCursorVisible)
    {
        InvertColor();
    }
}

void Vga::printAt(uint8_t x, uint8_t y, const char str[], uint16_t color)
{
    setCursorPosition(x, y);
    print(str, color);
}

void Vga::print(uint8_t c, uint16_t color)
{
    switch (c)
    {
    case '\0': //null
        break;
    case '\n': //line feed
        if (cursor_y < VSIZE_CHARS - 1)
        {
            setCursorPosition(0, cursor_y + 1);
        }
        break;
    case 8: //backspace
        if (cursor_x > 0)
        {
            print_char(cursor_x - 1, cursor_y, ' ', color);
            setCursorPosition(cursor_x - 1, cursor_y);
        }
        break;
    case 13: //carriage return !?!?!?!VT!?!??!?!
        cursor_x = 0;
        break;
    case 14: //form feed new page(clear screen)
        //clear_screen();
        break;
    default:
        uint8_t x = cursor_x;
        uint8_t y = cursor_y;
        cursorNext();
        print_char(x, y, c, color);
    }
}

void Vga::print(const char *str, uint16_t color)
{
    while (*str)
    {
        print(*str++, color);
    }
}

void Vga::InvertColor()
{
    uint16_t originalColor = Vga::VideoMemoryColors[Vga::cursor_y * HSIZE_CHARS + Vga::cursor_x];
    uint16_t newColor = LOBYTE(originalColor) << 8 | HIBYTE(originalColor);
    Vga::VideoMemoryColors[Vga::cursor_y * HSIZE_CHARS + Vga::cursor_x] = newColor;
}
