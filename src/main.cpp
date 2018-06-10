#include "startup.h"
#include "vga.h"

int previousDemo = -1;
int currentDemo = 3;

// Demo 1 : display ZX Spectrum screenshot
#include "demo/bubblebobble.h"

// Demo 2 : text terminal

// Demo 3 : Draw
#include "demo/demo.h"

// Demo 4 : Xonix game
#include "game/Platform.h"

void showSinclairScreenshot(const char *screenshot);
void ClearUsbBuffer();
extern uint8_t GetUsbBuffer(char *buffer, uint8_t maxLength);

void setup()
{
    UsbDeviceInit(3, 3);
    RtcInit();

    __HAL_RCC_GPIOC_CLK_ENABLE();
    GPIO_InitTypeDef gpioInit;
    gpioInit.Pin = GPIO_PIN_13;
    gpioInit.Mode = GPIO_MODE_OUTPUT_PP;
    gpioInit.Pull = GPIO_PULLUP;
    gpioInit.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOC, &gpioInit);

    HAL_SuspendTick();
    Vga::BitmapMode = MODE_SINCLAIR;
    Vga::InitVga(Vga::timing_640x480_60_01hz);
    Vga::clear_screen(0x3F10);
}

void loop()
{
    bool demoInit;
    char buffer[32];
    uint8_t len = GetUsbBuffer(buffer, 32);
    if (len > 0)
    {
        char buf[50];
        for(int i = 0; i < len; i++)
        {
            sprintf(&buf[3 * i], "%02x ", buffer[i]);
        }
        CDC_Transmit_FS((uint8_t *)buf, 3 * len);

        if (len == 5 && buffer[0] == '\x1B' && buffer[1] == '\x5B' && buffer[2] == '\x31' && buffer[4] == '\x7e')
        {
            switch (buffer[3])
            {
            case '\x31':
                currentDemo = 1;
                break;
            case '\x32':
                currentDemo = 2;
                break;
            case '\x33':
                currentDemo = 3;
                break;
            case '\x34':
                currentDemo = 4;
                break;
            }

            len = 0;
        }
    }

    demoInit = (currentDemo != previousDemo);
    previousDemo = currentDemo;

    switch (currentDemo)
    {
    case 1:
        if (demoInit)
        {
            Vga::hideCursor();
            showSinclairScreenshot(bubblebobble);
        }
        break;
    case 2:
        if (demoInit)
        {
            Vga::clear_screen(0x3F10);

            Vga::setCursorPosition(0, 17);
            for (uint8_t charCode = 32; charCode < 255; charCode++)
            {
                Vga::print(charCode, 0x3F10);
            }

            Vga::setCursorPosition(0, 0);
            Vga::showCursor();
        }
        else
        {
            if (len == 1)
            {
                Vga::print(buffer[0], 0x3F10);
            }

            if (len == 3 && buffer[0] == '\e' && buffer[1] == '[')
            {
                switch (buffer[2])
                {
                case 'D':
                    if (Vga::cursor_x > 0)
                    {
                        Vga::setCursorPosition(Vga::cursor_x - 1, Vga::cursor_y);
                    }
                    break;
                case 'C':
                    if (Vga::cursor_x < Vga::hres() - 1)
                    {
                        Vga::setCursorPosition(Vga::cursor_x + 1, Vga::cursor_y);
                    }
                    break;
                case 'A':
                    if (Vga::cursor_y > 0)
                    {
                        Vga::setCursorPosition(Vga::cursor_x, Vga::cursor_y - 1);
                    }
                    break;
                case 'B':
                    if (Vga::cursor_y < Vga::vres() - 1)
                    {
                        Vga::setCursorPosition(Vga::cursor_x, Vga::cursor_y + 1);
                    }
                    break;
                }
            }
        }
        break;
    case 3:
        if (demoInit)
        {
            Vga::hideCursor();
            VgaDemo::DemoSetup();
        }
        else
        {
            VgaDemo::DemoLoop();
        }
        break;
    case 4:
        if (demoInit)
        {
            Vga::hideCursor();
            Vga::clear_screen(0x0C00);

            GameInit();
        }
        else
        {
            GameUpdate();
        }
        break;
    }

    if (len > 0)
    {
        ClearUsbBuffer();
    }
}

void ClearUsbBuffer()
{
    USBD_CDC_HandleTypeDef *hcdc = (USBD_CDC_HandleTypeDef *)hUsbDeviceFS.pClassData;
    if (hcdc->RxLength > 0)
    {
        hcdc->RxLength = 0;
    }
}

void showSinclairScreenshot(const char *screenshot)
{
    memcpy((void *)Vga::VideoMemoryPixels, screenshot, BITMAP_SIZE);
    for (uint32_t i = 0; i < COLORS_SIZE; i++)
    {
        Vga::VideoMemoryColors[i] = Vga::ConvertSinclairColor(screenshot[BITMAP_SIZE + i]);
    }
}
