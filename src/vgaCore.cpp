#include <stm32f1xx_hal.h>
#include <string.h>

#include "startup.h"
#include "vgacore.h"
#include "font8x8.h"

volatile uint8_t Vga::VideoMemoryPixels[BITMAP_SIZE];
volatile uint16_t Vga::VideoMemoryColors[COLORS_SIZE];
uint8_t Vga::BitmapMode = MODE_TVOUT;

#define __irq extern "C"

static TIM_HandleTypeDef htim2;
static TIM_HandleTypeDef htim3;
static TIM_HandleTypeDef htim4;

static volatile int vline = 0; /* The current line being drawn */
static volatile int vflag = 0; /* When 1, can draw on the screen */
static volatile int vdraw = 0; /* Used to increment vline every 2 drawn lines */
static volatile uint8_t *GPIO_ODR;

void DoDraw();

namespace Vga
{
void setCursorPosition(uint8_t x, uint8_t y);
void select_font(const uint8_t* f);

void InitHSync(Vga::Timing::Polarity polarity, int wholeLine, int syncPulse, int startDraw);
void InitVSync(Vga::Timing::Polarity polarity, int wholeFrame, int syncPulse, int startDraw);
} 

void Vga::InitVga(Timing timing)
{
    GPIO_InitTypeDef gpioInit;

    // Set PA0..PA5 to OUTPUT with high speed
    __HAL_RCC_GPIOA_CLK_ENABLE();
    gpioInit.Pin = GPIO_PIN_0 | GPIO_PIN_1 | GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 | GPIO_PIN_5;
    gpioInit.Mode = GPIO_MODE_OUTPUT_PP;
    gpioInit.Pull = GPIO_PULLUP;
    gpioInit.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOA, &gpioInit);

    // HSync on PB0 and VSync on PB6
    __HAL_RCC_GPIOB_CLK_ENABLE();
    gpioInit.Pin = GPIO_PIN_0 | GPIO_PIN_6;
    gpioInit.Mode = GPIO_MODE_AF_PP;
    gpioInit.Pull = GPIO_PULLUP;
    gpioInit.Speed = GPIO_SPEED_FREQ_HIGH;
    HAL_GPIO_Init(GPIOB, &gpioInit);

    GPIO_ODR = (volatile uint8_t *)&GPIOA->ODR;

    double factor = HAL_RCC_GetHCLKFreq() / 1000000.0 / timing.pixel_frequency_mhz;
    int wholeLine = factor * timing.line_pixels;
    int syncPulse = factor * timing.sync_pixels;
    int startDraw = factor * (timing.sync_pixels + timing.back_porch_pixels) - 130;

    InitHSync(timing.hsync_polarity, wholeLine, syncPulse, startDraw);
    InitVSync(timing.vsync_polarity, timing.video_end_line,
              timing.vsync_end_line - timing.vsync_start_line,
              timing.video_start_line + (timing.video_end_line - timing.video_start_line - VSIZE_PIXELS * REPEAT_LINES) / 2);

    // 640 x 480 @ 60 Hz (Standard, pixel clock frequency 25.17 kHz)
    //InitHSync(Timing::Polarity::positive, 2287, 275, 300);
    //InitVSync(Timing::Polarity::positive, 525, 2, 35+5 ); // +48

    // 720 x 400 @ 70 Hz (Standard)
    //InitHSync(Timing::Polarity::negative, 2288, 274, 280);
    //InitVSync(Timing::Polarity::positive, 449, 2, 28 + 18);

    // 720 x 576 @ 50 Hz (non-standard, pixel clock frequency 27.0 MHz)
    //InitHSync(true, 2304, 171, 280);
    //InitVSync(true, 625, 5, 28 + 5);

    // 640 x 480 @ 60 Hz (non-standard, pixel clock frequency 24.0 MHz)
    //InitHSync(Timing::Polarity::negative, 2376, 264, 280);
    //InitVSync(Timing::Polarity::negative, 505, 5, 70);

    // 800 x 600 @ 56 Hz (Standard, pixel clock frequency 38.1 MHz)
    //InitHSync(true, 2056, 241, 260);
    //InitVSync(true, 619, 4, 24);

    // 800 x 600 @ 56 Hz ((non-standard, pixel clock frequency 36.0 MHz)
    //InitHSync(true, 2000, 272, 500);
    //InitVSync(true, 631, 6, 24);

    vline = 0;
    vdraw = 0;

    Vga::select_font(font8x8);
}

volatile uint8_t* Vga::GetBitmapAddress(uint8_t vline)
{
    if (BitmapMode == MODE_TVOUT)
    {
        // TVout addressing
        return &Vga::VideoMemoryPixels[vline * HSIZE_CHARS];
    }

    // ZX Sinclair addressing
    // 00-00-00-Y7-Y6-Y2-Y1-Y0 Y5-Y4-Y3-x4-x3-x2-x1-x0
    //          12 11 10  9  8  7  6  5  4  3  2  1  0 

    uint32_t y012 = ((vline & 0B00000111) << 8);
    uint32_t y345 = ((vline & 0B00111000) << 2);
    uint32_t y67 =  ((vline & 0B11000000) << 5);
    return &Vga::VideoMemoryPixels[y012 | y345 | y67];
}

volatile uint8_t* Vga::GetBitmapAddress(uint8_t vline, uint8_t character)
{
    character &= 0B00011111;
    return Vga::GetBitmapAddress(vline) + character;
}

uint16_t Vga::ConvertSinclairColor(uint8_t sinclairColors)
{
    // Sinclair: Flash-Bright-PaperG-PaperR-PaperB-InkG-InkR-InkB
    //               7      6      5      4      3    2    1    0
    // Our colors: 00-PaperB01-PaperG01-PaperR01 : 00-InkB01-InkG01-InkR01
    //                      54       32       10 :        54     32     10 

    bool bright = ((sinclairColors & 0B01000000) != 0);

    uint16_t ink = ((sinclairColors & 0B00000100) << 8); // InkG
    ink |= ((sinclairColors & 0B00000010) << 7);         // InkR
    ink |= ((sinclairColors & 0B00000001) << 12);        // InkB
    if (bright)
    {
        ink |= (ink << 1);
    }
    
    uint16_t paper = ((sinclairColors & 0B00100000) >> 3); // PaperG
    paper |= ((sinclairColors & 0B00010000) >> 4);         // PaperR
    paper |= ((sinclairColors & 0B00001000) << 1);         // PaperB
    if (bright)
    {
        paper |= (paper << 1);
    }

    return ink | paper;
}

void Vga::clear_screen(uint16_t color)
{
    memset((void *)Vga::VideoMemoryPixels, 0, BITMAP_SIZE);
    for (int i = 0; i < COLORS_SIZE; i++)
    {
        Vga::VideoMemoryColors[i] = color;
    }

    setCursorPosition(0, 0);
}

unsigned long Vga::millis()
{
    // https://www.tablix.org/~avian/blog/archives/2012/04/reading_stm32f1_real_time_clock/

    uint16_t divl1 = hRtc.Instance->DIVL;
    uint16_t cnth1 = (hRtc.Instance->CNTH & RTC_CNTH_RTC_CNT);
    uint16_t cntl1 = (hRtc.Instance->CNTL & RTC_CNTL_RTC_CNT);

    uint16_t divl2 = hRtc.Instance->DIVL;
    uint16_t cnth2 = (hRtc.Instance->CNTH & RTC_CNTH_RTC_CNT);
    uint16_t cntl2 = (hRtc.Instance->CNTL & RTC_CNTL_RTC_CNT);

    uint16_t divl, cnth, cntl;

    if (cntl1 != cntl2)
    {
        /* overflow occurred between reads of cntl, hence it
        * couldn't have occurred before the first read. */
        divl = divl1;
        cnth = cnth1;
        cntl = cntl1;
    }
    else
    {
        /* no overflow between reads of cntl, hence the
        * values between the reads are correct */
        divl = divl2;
        cnth = cnth2;
        cntl = cntl2;
    }

    /* CNT is incremented one RTCCLK tick after the DIV counter
    * gets reset to 32767, so to correct for that increment 
    * the seconds count if DIV just got reset */
    uint32_t sec = ((uint32_t)cnth << 16 | (uint32_t)cntl);
    if (divl == 32767)
    {
        sec++;
    }

    return sec * 1000 + (32767 - (uint32_t)divl) / 32.768;
}

void Vga::delay(uint32_t x)
{
    unsigned long startMillis = millis();
    unsigned long endMillis = startMillis + x;

    unsigned long currMillis;
    do
    {
        currMillis = millis();
    } while (currMillis < endMillis && currMillis >= startMillis);
}

void Vga::delay_frame()
{
    while (vdraw) {}
}

//*****************************************************************************
//	This irq is generated slightly before TIM3_IRQHandler
//*****************************************************************************
__irq void TIM2_IRQHandler()
{
    __HAL_TIM_CLEAR_IT(&htim2, TIM_FLAG_CC2);

    if (vflag)
    {
        // Wait for interrupt
        __asm__ volatile("wfi \n\t" :::);
    }
}

//*****************************************************************************
//	This irq is generated at the end of the horizontal back porch.
//*****************************************************************************
__irq void TIM3_IRQHandler()
{
    __HAL_TIM_CLEAR_IT(&htim3, TIM_FLAG_CC2);

    if (vflag)
    {
        //__disable_irq();

        DoDraw();

        vdraw++;
        if (vdraw == 2)
        {
            vdraw = 0;
            vline++;
            if (vline == VSIZE_PIXELS)
            {
                vdraw = vline = vflag = 0;
            }
        }

        //__enable_irq();
    }
}

//*****************************************************************************
//	This irq is generated at the end of the vertical back porch.
//*****************************************************************************
__irq void TIM4_IRQHandler()
{
    __HAL_TIM_CLEAR_IT(&htim4, TIM_FLAG_CC4);

    vflag = 1;
    vline = 0;
}

void Vga::InitVSync(
    Vga::Timing::Polarity polarity,
    int wholeFrame,
    int syncPulse,
    int startDraw)
{
    __HAL_RCC_TIM4_CLK_ENABLE();

    htim4.Instance = TIM4;
    htim4.Init.Prescaler = 0;
    htim4.Init.Period = wholeFrame - 1;
    htim4.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim4.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim4.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    HAL_TIM_Base_Init(&htim4);

    // Slave mode Gated, triggered by TIM3
    TIM_SlaveConfigTypeDef sSlaveConfig;
    sSlaveConfig.SlaveMode = TIM_SLAVEMODE_GATED;
    sSlaveConfig.InputTrigger = TIM_TS_ITR2;
    sSlaveConfig.TriggerPolarity = TIM_TRIGGERPOLARITY_NONINVERTED;
    sSlaveConfig.TriggerPrescaler = TIM_TRIGGERPRESCALER_DIV1;
    sSlaveConfig.TriggerFilter = 0;
    HAL_TIM_SlaveConfigSynchronization(&htim4, &sSlaveConfig);

    HAL_NVIC_SetPriority(TIM4_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM4_IRQn);

    TIM_OC_InitTypeDef sConfigOC;

    // VSync on pin PB6
    sConfigOC.OCMode = polarity == Vga::Timing::Polarity::negative ? TIM_OCMODE_PWM2 : TIM_OCMODE_PWM1;
    sConfigOC.Pulse = syncPulse;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_HIGH;
    sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    HAL_TIM_PWM_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_1);
    TIM_CCxChannelCmd(htim4.Instance, TIM_CHANNEL_1, TIM_CCx_ENABLE);

    sConfigOC.OCMode = TIM_OCMODE_INACTIVE;
    sConfigOC.Pulse = startDraw;
    HAL_TIM_OC_ConfigChannel(&htim4, &sConfigOC, TIM_CHANNEL_4);
    __HAL_TIM_ENABLE_IT(&htim4, TIM_IT_CC4);

    if (IS_TIM_BREAK_INSTANCE(htim4.Instance) != RESET)
    {
        __HAL_TIM_MOE_ENABLE(&htim4);
    }

    __HAL_TIM_ENABLE(&htim4);
}

void Vga::InitHSync(
    Vga::Timing::Polarity polarity,
    int wholeLine,
    int syncPulse,
    int startDraw)
{
    TIM_OC_InitTypeDef sConfigOC;
    TIM_MasterConfigTypeDef sMasterConfig;

    __HAL_RCC_TIM2_CLK_ENABLE();

    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 0;
    htim2.Init.Period = wholeLine - 1;
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    HAL_TIM_Base_Init(&htim2);

    // Master mode
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_ENABLE;
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_ENABLE;
    HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig);

    HAL_NVIC_SetPriority(TIM2_IRQn, 1, 1);
    HAL_NVIC_EnableIRQ(TIM2_IRQn);

    sConfigOC.OCMode = TIM_OCMODE_INACTIVE;
    sConfigOC.Pulse = startDraw - 12 - 1;
    HAL_TIM_OC_ConfigChannel(&htim2, &sConfigOC, TIM_CHANNEL_2);
    __HAL_TIM_ENABLE_IT(&htim2, TIM_IT_CC2);

    if (IS_TIM_BREAK_INSTANCE(htim2.Instance) != RESET)
    {
        __HAL_TIM_MOE_ENABLE(&htim2);
    }

    __HAL_RCC_TIM3_CLK_ENABLE();

    htim3.Instance = TIM3;
    htim3.Init.Prescaler = 0;
    htim3.Init.Period = wholeLine - 1;
    htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    htim3.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_DISABLE;
    HAL_TIM_Base_Init(&htim3);

    // Master mode
    sMasterConfig.MasterSlaveMode = TIM_MASTERSLAVEMODE_ENABLE;
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_UPDATE;
    HAL_TIMEx_MasterConfigSynchronization(&htim3, &sMasterConfig);

    // Slave mode Trigger, triggered by TIM2
    TIM_SlaveConfigTypeDef sSlaveConfig;
    sSlaveConfig.SlaveMode = TIM_SLAVEMODE_TRIGGER;
    sSlaveConfig.InputTrigger = TIM_TS_ITR1;
    sSlaveConfig.TriggerPolarity = TIM_TRIGGERPOLARITY_NONINVERTED;
    sSlaveConfig.TriggerPrescaler = TIM_TRIGGERPRESCALER_DIV1;
    sSlaveConfig.TriggerFilter = 0;
    HAL_TIM_SlaveConfigSynchronization(&htim3, &sSlaveConfig);

    HAL_NVIC_SetPriority(TIM3_IRQn, 0, 0);
    HAL_NVIC_EnableIRQ(TIM3_IRQn);

    // HSync on pin PB0
    sConfigOC.OCMode = polarity == Vga::Timing::Polarity::negative ? TIM_OCMODE_PWM2 : TIM_OCMODE_PWM1;
    sConfigOC.Pulse = syncPulse;
    sConfigOC.OCIdleState = TIM_OCIDLESTATE_RESET;
    sConfigOC.OCPolarity = TIM_OCPOLARITY_LOW;
    sConfigOC.OCFastMode = TIM_OCFAST_DISABLE;
    HAL_TIM_PWM_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_3);
    TIM_CCxChannelCmd(htim3.Instance, TIM_CHANNEL_3, TIM_CCx_ENABLE);

    sConfigOC.OCMode = TIM_OCMODE_INACTIVE;
    sConfigOC.Pulse = startDraw - 1;
    HAL_TIM_OC_ConfigChannel(&htim3, &sConfigOC, TIM_CHANNEL_2);
    __HAL_TIM_ENABLE_IT(&htim3, TIM_IT_CC2);

    if (IS_TIM_BREAK_INSTANCE(htim3.Instance) != RESET)
    {
        __HAL_TIM_MOE_ENABLE(&htim3);
    }

    __HAL_TIM_SET_COUNTER(&htim2, 0);
    __HAL_TIM_SET_COUNTER(&htim3, 0);

    // Starts TIM3 as well
    __HAL_TIM_ENABLE(&htim2);
    //__HAL_TIM_ENABLE(&htim3);
}

inline void DoDraw()
{
    __asm__ volatile(
        "  ldr r1, [%[pix]], #4 \n\t" // pixels for characters 0..3
        "  ldr r3, [%[col]], #4 \n\t" // colors for characters 0..1

        // character #0
        "  ror r1, r1, #4    \n\t" // pixels >> 4
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  strb r0, [%[odr]] \n\t"
        ".rept 7 \n\t"
        "  nop               \n\t"
        "  ror r1, r1, #31   \n\t" // pixels << 1
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  strb r0, [%[odr]] \n\t"
        ".endr   \n\t"

        // character #1
        "  ror r3, r3, #16   \n\t" // colors
        "  ror r1, r1, #15   \n\t" // pixels >> 15
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  strb r0, [%[odr]] \n\t"
        ".rept 5 \n\t"
        "  nop               \n\t"
        "  ror r1, r1, #31   \n\t" // pixels << 1
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  strb r0, [%[odr]] \n\t"
        ".endr   \n\t"
        "  ror r1, r1, #31   \n\t" // pixels << 1
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  ror r1, r1, #31   \n\t" // pixels << 1
        "  strb r0, [%[odr]] \n\t"
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  ldr r3, [%[col]], #4 \n\t" // colors for next 2 characters
        "  strb r0, [%[odr]] \n\t"

        // character #2
        "  nop               \n\t"
        "  ror r1, r1, #15   \n\t" // pixels >> 15
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  strb r0, [%[odr]] \n\t"
        ".rept 7 \n\t"
        "  nop               \n\t"
        "  ror r1, r1, #31   \n\t" // pixels << 1
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  strb r0, [%[odr]] \n\t"
        ".endr   \n\t"

        // character #3
        "  ror r3, r3, #16   \n\t" // colors
        "  ror r1, r1, #15   \n\t" // pixels >> 15
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  strb r0, [%[odr]] \n\t"
        ".rept 3 \n\t"
        "  nop               \n\t"
        "  ror r1, r1, #31   \n\t" // pixels << 1
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  strb r0, [%[odr]] \n\t"
        ".endr   \n\t"
        "  ror r1, r1, #31   \n\t" // pixels << 1
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  ror r1, r1, #31   \n\t" // pixels << 1
        "  strb r0, [%[odr]] \n\t"
        "  ldr r2, [%[pix]], #4 \n\t" // r2: pixels for next 4 characters
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  strb r0, [%[odr]] \n\t"
        "  ror r1, r1, #31   \n\t" // pixels << 1
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  ror r1, r1, #31   \n\t" // pixels << 1
        "  strb r0, [%[odr]] \n\t"
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  ldr r3, [%[col]], #4 \n\t" // colors for next 2 characters
        "  strb r0, [%[odr]] \n\t"

        // character #4
        "  mov r1, r2        \n\t" // pixels
        "  ror r1, r1, #4    \n\t" // pixels >> 4
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  strb r0, [%[odr]] \n\t"
        ".rept 7 \n\t"
        "  nop               \n\t"
        "  ror r1, r1, #31   \n\t" // pixels << 1
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  strb r0, [%[odr]] \n\t"
        ".endr   \n\t"

        // character #5
        "  ror r3, r3, #16   \n\t" // colors
        "  ror r1, r1, #15   \n\t" // pixels >> 15
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  strb r0, [%[odr]] \n\t"
        ".rept 5 \n\t"
        "  nop               \n\t"
        "  ror r1, r1, #31   \n\t" // pixels << 1
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  strb r0, [%[odr]] \n\t"
        ".endr   \n\t"
        "  ror r1, r1, #31   \n\t" // pixels << 1
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  ror r1, r1, #31   \n\t" // pixels << 1
        "  strb r0, [%[odr]] \n\t"
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  ldr r3, [%[col]], #4 \n\t" // colors for next 2 characters
        "  strb r0, [%[odr]] \n\t"

        // character #6
        "  nop               \n\t"
        "  ror r1, r1, #15   \n\t" // pixels >> 15
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  strb r0, [%[odr]] \n\t"
        ".rept 7 \n\t"
        "  nop               \n\t"
        "  ror r1, r1, #31   \n\t" // pixels << 1
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  strb r0, [%[odr]] \n\t"
        ".endr   \n\t"

        // character #7
        "  ror r3, r3, #16   \n\t" // colors
        "  ror r1, r1, #15   \n\t" // pixels >> 15
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  strb r0, [%[odr]] \n\t"
        ".rept 3 \n\t"
        "  nop               \n\t"
        "  ror r1, r1, #31   \n\t" // pixels << 1
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  strb r0, [%[odr]] \n\t"
        ".endr   \n\t"
        "  ror r1, r1, #31   \n\t" // pixels << 1
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  ror r1, r1, #31   \n\t" // pixels << 1
        "  strb r0, [%[odr]] \n\t"
        "  ldr r2, [%[pix]], #4 \n\t" // r2: pixels for next 4 characters
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  strb r0, [%[odr]] \n\t"
        "  ror r1, r1, #31   \n\t" // pixels << 1
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  ror r1, r1, #31   \n\t" // pixels << 1
        "  strb r0, [%[odr]] \n\t"
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  ldr r3, [%[col]], #4 \n\t" // colors for next 2 characters
        "  strb r0, [%[odr]] \n\t"

        // character #8
        "  mov r1, r2        \n\t" // pixels
        "  ror r1, r1, #4    \n\t" // pixels >> 4
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  strb r0, [%[odr]] \n\t"
        ".rept 7 \n\t"
        "  nop               \n\t"
        "  ror r1, r1, #31   \n\t" // pixels << 1
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  strb r0, [%[odr]] \n\t"
        ".endr   \n\t"

        // character #9
        "  ror r3, r3, #16   \n\t" // colors
        "  ror r1, r1, #15   \n\t" // pixels >> 15
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  strb r0, [%[odr]] \n\t"
        ".rept 5 \n\t"
        "  nop               \n\t"
        "  ror r1, r1, #31   \n\t" // pixels << 1
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  strb r0, [%[odr]] \n\t"
        ".endr   \n\t"
        "  ror r1, r1, #31   \n\t" // pixels << 1
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  ror r1, r1, #31   \n\t" // pixels << 1
        "  strb r0, [%[odr]] \n\t"
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  ldr r3, [%[col]], #4 \n\t" // colors for next 2 characters
        "  strb r0, [%[odr]] \n\t"

        // character #10
        "  nop               \n\t"
        "  ror r1, r1, #15   \n\t" // pixels >> 15
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  strb r0, [%[odr]] \n\t"
        ".rept 7 \n\t"
        "  nop               \n\t"
        "  ror r1, r1, #31   \n\t" // pixels << 1
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  strb r0, [%[odr]] \n\t"
        ".endr   \n\t"

        // character #11
        "  ror r3, r3, #16   \n\t" // colors
        "  ror r1, r1, #15   \n\t" // pixels >> 15
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  strb r0, [%[odr]] \n\t"
        ".rept 3 \n\t"
        "  nop               \n\t"
        "  ror r1, r1, #31   \n\t" // pixels << 1
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  strb r0, [%[odr]] \n\t"
        ".endr   \n\t"
        "  ror r1, r1, #31   \n\t" // pixels << 1
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  ror r1, r1, #31   \n\t" // pixels << 1
        "  strb r0, [%[odr]] \n\t"
        "  ldr r2, [%[pix]], #4 \n\t" // r2: pixels for next 4 characters
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  strb r0, [%[odr]] \n\t"
        "  ror r1, r1, #31   \n\t" // pixels << 1
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  ror r1, r1, #31   \n\t" // pixels << 1
        "  strb r0, [%[odr]] \n\t"
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  ldr r3, [%[col]], #4 \n\t" // colors for next 2 characters
        "  strb r0, [%[odr]] \n\t"

        // character #12
        "  mov r1, r2        \n\t" // pixels
        "  ror r1, r1, #4    \n\t" // pixels >> 4
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  strb r0, [%[odr]] \n\t"
        ".rept 7 \n\t"
        "  nop               \n\t"
        "  ror r1, r1, #31   \n\t" // pixels << 1
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  strb r0, [%[odr]] \n\t"
        ".endr   \n\t"

        // character #13
        "  ror r3, r3, #16   \n\t" // colors
        "  ror r1, r1, #15   \n\t" // pixels >> 15
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  strb r0, [%[odr]] \n\t"
        ".rept 5 \n\t"
        "  nop               \n\t"
        "  ror r1, r1, #31   \n\t" // pixels << 1
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  strb r0, [%[odr]] \n\t"
        ".endr   \n\t"
        "  ror r1, r1, #31   \n\t" // pixels << 1
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  ror r1, r1, #31   \n\t" // pixels << 1
        "  strb r0, [%[odr]] \n\t"
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  ldr r3, [%[col]], #4 \n\t" // colors for next 2 characters
        "  strb r0, [%[odr]] \n\t"

        // character #14
        "  nop               \n\t"
        "  ror r1, r1, #15   \n\t" // pixels >> 15
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  strb r0, [%[odr]] \n\t"
        ".rept 7 \n\t"
        "  nop               \n\t"
        "  ror r1, r1, #31   \n\t" // pixels << 1
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  strb r0, [%[odr]] \n\t"
        ".endr   \n\t"

        // character #15
        "  ror r3, r3, #16   \n\t" // colors
        "  ror r1, r1, #15   \n\t" // pixels >> 15
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  strb r0, [%[odr]] \n\t"
        ".rept 3 \n\t"
        "  nop               \n\t"
        "  ror r1, r1, #31   \n\t" // pixels << 1
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  strb r0, [%[odr]] \n\t"
        ".endr   \n\t"
        "  ror r1, r1, #31   \n\t" // pixels << 1
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  ror r1, r1, #31   \n\t" // pixels << 1
        "  strb r0, [%[odr]] \n\t"
        "  ldr r2, [%[pix]], #4 \n\t" // r2: pixels for next 4 characters
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  strb r0, [%[odr]] \n\t"
        "  ror r1, r1, #31   \n\t" // pixels << 1
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  ror r1, r1, #31   \n\t" // pixels << 1
        "  strb r0, [%[odr]] \n\t"
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  ldr r3, [%[col]], #4 \n\t" // colors for next 2 characters
        "  strb r0, [%[odr]] \n\t"

        // character #16
        "  mov r1, r2        \n\t" // pixels
        "  ror r1, r1, #4    \n\t" // pixels >> 4
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  strb r0, [%[odr]] \n\t"
        ".rept 7 \n\t"
        "  nop               \n\t"
        "  ror r1, r1, #31   \n\t" // pixels << 1
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  strb r0, [%[odr]] \n\t"
        ".endr   \n\t"

        // character #17
        "  ror r3, r3, #16   \n\t" // colors
        "  ror r1, r1, #15   \n\t" // pixels >> 15
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  strb r0, [%[odr]] \n\t"
        ".rept 5 \n\t"
        "  nop               \n\t"
        "  ror r1, r1, #31   \n\t" // pixels << 1
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  strb r0, [%[odr]] \n\t"
        ".endr   \n\t"
        "  ror r1, r1, #31   \n\t" // pixels << 1
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  ror r1, r1, #31   \n\t" // pixels << 1
        "  strb r0, [%[odr]] \n\t"
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  ldr r3, [%[col]], #4 \n\t" // colors for next 2 characters
        "  strb r0, [%[odr]] \n\t"

        // character #18
        "  nop               \n\t"
        "  ror r1, r1, #15   \n\t" // pixels >> 15
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  strb r0, [%[odr]] \n\t"
        ".rept 7 \n\t"
        "  nop               \n\t"
        "  ror r1, r1, #31   \n\t" // pixels << 1
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  strb r0, [%[odr]] \n\t"
        ".endr   \n\t"

        // character #19
        "  ror r3, r3, #16   \n\t" // colors
        "  ror r1, r1, #15   \n\t" // pixels >> 15
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  strb r0, [%[odr]] \n\t"
        ".rept 3 \n\t"
        "  nop               \n\t"
        "  ror r1, r1, #31   \n\t" // pixels << 1
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  strb r0, [%[odr]] \n\t"
        ".endr   \n\t"
        "  ror r1, r1, #31   \n\t" // pixels << 1
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  ror r1, r1, #31   \n\t" // pixels << 1
        "  strb r0, [%[odr]] \n\t"
        "  ldr r2, [%[pix]], #4 \n\t" // r2: pixels for next 4 characters
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  strb r0, [%[odr]] \n\t"
        "  ror r1, r1, #31   \n\t" // pixels << 1
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  ror r1, r1, #31   \n\t" // pixels << 1
        "  strb r0, [%[odr]] \n\t"
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  ldr r3, [%[col]], #4 \n\t" // colors for next 2 characters
        "  strb r0, [%[odr]] \n\t"

        // character #20
        "  mov r1, r2        \n\t" // pixels
        "  ror r1, r1, #4    \n\t" // pixels >> 4
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  strb r0, [%[odr]] \n\t"
        ".rept 7 \n\t"
        "  nop               \n\t"
        "  ror r1, r1, #31   \n\t" // pixels << 1
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  strb r0, [%[odr]] \n\t"
        ".endr   \n\t"

        // character #21
        "  ror r3, r3, #16   \n\t" // colors
        "  ror r1, r1, #15   \n\t" // pixels >> 15
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  strb r0, [%[odr]] \n\t"
        ".rept 5 \n\t"
        "  nop               \n\t"
        "  ror r1, r1, #31   \n\t" // pixels << 1
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  strb r0, [%[odr]] \n\t"
        ".endr   \n\t"
        "  ror r1, r1, #31   \n\t" // pixels << 1
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  ror r1, r1, #31   \n\t" // pixels << 1
        "  strb r0, [%[odr]] \n\t"
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  ldr r3, [%[col]], #4 \n\t" // colors for next 2 characters
        "  strb r0, [%[odr]] \n\t"

        // character #22
        "  nop               \n\t"
        "  ror r1, r1, #15   \n\t" // pixels >> 15
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  strb r0, [%[odr]] \n\t"
        ".rept 7 \n\t"
        "  nop               \n\t"
        "  ror r1, r1, #31   \n\t" // pixels << 1
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  strb r0, [%[odr]] \n\t"
        ".endr   \n\t"

        // character #23
        "  ror r3, r3, #16   \n\t" // colors
        "  ror r1, r1, #15   \n\t" // pixels >> 15
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  strb r0, [%[odr]] \n\t"
        ".rept 3 \n\t"
        "  nop               \n\t"
        "  ror r1, r1, #31   \n\t" // pixels << 1
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  strb r0, [%[odr]] \n\t"
        ".endr   \n\t"
        "  ror r1, r1, #31   \n\t" // pixels << 1
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  ror r1, r1, #31   \n\t" // pixels << 1
        "  strb r0, [%[odr]] \n\t"
        "  ldr r2, [%[pix]], #4 \n\t" // r2: pixels for next 4 characters
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  strb r0, [%[odr]] \n\t"
        "  ror r1, r1, #31   \n\t" // pixels << 1
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  ror r1, r1, #31   \n\t" // pixels << 1
        "  strb r0, [%[odr]] \n\t"
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  ldr r3, [%[col]], #4 \n\t" // colors for next 2 characters
        "  strb r0, [%[odr]] \n\t"

        // character #24
        "  mov r1, r2        \n\t" // pixels
        "  ror r1, r1, #4    \n\t" // pixels >> 4
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  strb r0, [%[odr]] \n\t"
        ".rept 7 \n\t"
        "  nop               \n\t"
        "  ror r1, r1, #31   \n\t" // pixels << 1
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  strb r0, [%[odr]] \n\t"
        ".endr   \n\t"

        // character #25
        "  ror r3, r3, #16   \n\t" // colors
        "  ror r1, r1, #15   \n\t" // pixels >> 15
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  strb r0, [%[odr]] \n\t"
        ".rept 5 \n\t"
        "  nop               \n\t"
        "  ror r1, r1, #31   \n\t" // pixels << 1
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  strb r0, [%[odr]] \n\t"
        ".endr   \n\t"
        "  ror r1, r1, #31   \n\t" // pixels << 1
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  ror r1, r1, #31   \n\t" // pixels << 1
        "  strb r0, [%[odr]] \n\t"
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  ldr r3, [%[col]], #4 \n\t" // colors for next 2 characters
        "  strb r0, [%[odr]] \n\t"

        // character #26
        "  nop               \n\t"
        "  ror r1, r1, #15   \n\t" // pixels >> 15
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  strb r0, [%[odr]] \n\t"
        ".rept 7 \n\t"
        "  nop               \n\t"
        "  ror r1, r1, #31   \n\t" // pixels << 1
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  strb r0, [%[odr]] \n\t"
        ".endr   \n\t"

        // character #27
        "  ror r3, r3, #16   \n\t" // colors
        "  ror r1, r1, #15   \n\t" // pixels >> 15
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  strb r0, [%[odr]] \n\t"
        ".rept 3 \n\t"
        "  nop               \n\t"
        "  ror r1, r1, #31   \n\t" // pixels << 1
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  strb r0, [%[odr]] \n\t"
        ".endr   \n\t"
        "  ror r1, r1, #31   \n\t" // pixels << 1
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  ror r1, r1, #31   \n\t" // pixels << 1
        "  strb r0, [%[odr]] \n\t"
        "  ldr r2, [%[pix]], #4 \n\t" // r2: pixels for next 4 characters
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  strb r0, [%[odr]] \n\t"
        "  ror r1, r1, #31   \n\t" // pixels << 1
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  ror r1, r1, #31   \n\t" // pixels << 1
        "  strb r0, [%[odr]] \n\t"
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  ldr r3, [%[col]], #4 \n\t" // colors for next 2 characters
        "  strb r0, [%[odr]] \n\t"

        // character #28
        "  mov r1, r2        \n\t" // pixels
        "  ror r1, r1, #4    \n\t" // pixels >> 4
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  strb r0, [%[odr]] \n\t"
        ".rept 7 \n\t"
        "  nop               \n\t"
        "  ror r1, r1, #31   \n\t" // pixels << 1
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  strb r0, [%[odr]] \n\t"
        ".endr   \n\t"

        // character #29
        "  ror r3, r3, #16   \n\t" // colors
        "  ror r1, r1, #15   \n\t" // pixels >> 15
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  strb r0, [%[odr]] \n\t"
        ".rept 5 \n\t"
        "  nop               \n\t"
        "  ror r1, r1, #31   \n\t" // pixels << 1
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  strb r0, [%[odr]] \n\t"
        ".endr   \n\t"
        "  ror r1, r1, #31   \n\t" // pixels << 1
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  ror r1, r1, #31   \n\t" // pixels << 1
        "  strb r0, [%[odr]] \n\t"
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  ldr r3, [%[col]], #4 \n\t" // colors for next 2 characters
        "  strb r0, [%[odr]] \n\t"

        // character #30
        "  nop               \n\t"
        "  ror r1, r1, #15   \n\t" // pixels >> 15
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  strb r0, [%[odr]] \n\t"
        ".rept 7 \n\t"
        "  nop               \n\t"
        "  ror r1, r1, #31   \n\t" // pixels << 1
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  strb r0, [%[odr]] \n\t"
        ".endr   \n\t"

        // character #31
        "  ror r3, r3, #16   \n\t" // colors
        "  ror r1, r1, #15   \n\t" // pixels >> 15
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  strb r0, [%[odr]] \n\t"
        ".rept 3 \n\t"
        "  nop               \n\t"
        "  ror r1, r1, #31   \n\t" // pixels << 1
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  strb r0, [%[odr]] \n\t"
        ".endr   \n\t"
        "  ror r1, r1, #31   \n\t" // pixels << 1
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  ror r1, r1, #31   \n\t" // pixels << 1
        "  strb r0, [%[odr]] \n\t"
        "  ldr r2, [%[pix]], #4 \n\t" // r2: pixels for next 4 characters
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  strb r0, [%[odr]] \n\t"
        "  ror r1, r1, #31   \n\t" // pixels << 1
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  ror r1, r1, #31   \n\t" // pixels << 1
        "  strb r0, [%[odr]] \n\t"
        "  and r0, r1, #8    \n\t"
        "  lsr r0, r3, r0    \n\t"
        "  ldr r3, [%[col]], #4 \n\t" // colors for next 2 characters
        "  strb r0, [%[odr]] \n\t"

        // 0 at the end
        "  mov r0, #0        \n\t"
        "  nop               \n\t"
        "  nop               \n\t"
        "  nop               \n\t"
        "  strb r0, [%[odr]] \n\t"

        :
        : [pix] "r"(Vga::GetBitmapAddress(vline)),
          [col] "r"(&Vga::VideoMemoryColors[vline / 8 * HSIZE_CHARS]),
          [odr] "r"(GPIO_ODR)
        : "r0", "r1", "r2", "r3");
}
