#include "startup.h"
#include "usb/usbd_core.h"
#include "usb/usbd_desc.h"

USBD_HandleTypeDef hUsbDeviceFS;
RTC_HandleTypeDef hRtc;

void SystemClock_Config();
extern PCD_HandleTypeDef hpcd_USB_FS;

int main()
{
    HAL_Init();
    SystemClock_Config();

    setup();

    while (1)
    {
        loop();
    }
}

void UsbDeviceInit(uint32_t preemptPriority, uint32_t subPriority)
{
    USBD_Init(&hUsbDeviceFS, &FS_Desc, DEVICE_FS);
    USBD_RegisterClass(&hUsbDeviceFS, &USBD_CDC);
    USBD_CDC_RegisterInterface(&hUsbDeviceFS, &USBD_Interface_fops_FS);
    USBD_Start(&hUsbDeviceFS);

    HAL_NVIC_SetPriority(USB_LP_CAN1_RX0_IRQn, preemptPriority, subPriority);
}

void RtcInit()
{
    // Enable LSE Oscillator
    RCC_OscInitTypeDef RCC_OscInitStruct;
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_LSE;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_NONE; /* Mandatory, otherwise the PLL is reconfigured! */
    RCC_OscInitStruct.LSEState = RCC_LSE_ON;       /* External 32.768 kHz clock on OSC_IN/OSC_OUT */
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    HAL_PWR_EnableBkUpAccess();
    __HAL_RCC_BKP_CLK_ENABLE();
    __HAL_RCC_RTC_ENABLE();

    /**Initialize RTC Only 
    */
    hRtc.Instance = RTC;
    if (HAL_RTCEx_BKUPRead(&hRtc, RTC_BKP_DR1) != 0x32F2)
    {
        hRtc.Init.AsynchPrediv = RTC_AUTO_1_SECOND;
        hRtc.Init.OutPut = RTC_OUTPUTSOURCE_NONE;
        HAL_RTC_Init(&hRtc);

        /**Initialize RTC and set the Time and Date */
        RTC_TimeTypeDef sTime;
        sTime.Hours = 0x1;
        sTime.Minutes = 0x0;
        sTime.Seconds = 0x0;
        HAL_RTC_SetTime(&hRtc, &sTime, RTC_FORMAT_BCD);

        RTC_DateTypeDef DateToUpdate;
        DateToUpdate.WeekDay = RTC_WEEKDAY_MONDAY;
        DateToUpdate.Month = RTC_MONTH_JANUARY;
        DateToUpdate.Date = 0x1;
        DateToUpdate.Year = 0x0;
        HAL_RTC_SetDate(&hRtc, &DateToUpdate, RTC_FORMAT_BCD);

        HAL_RTCEx_BKUPWrite(&hRtc, RTC_BKP_DR1, 0x32F2);
    }
}

void SystemClock_Config()
{
    /**Initializes the CPU, AHB and APB busses clocks */
    RCC_OscInitTypeDef RCC_OscInitStruct;
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.HSIState = RCC_HSI_OFF;
    RCC_OscInitStruct.HSICalibrationValue = 0;
    RCC_OscInitStruct.LSIState = 0;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL = RCC_PLL_MUL9;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    /**Initializes the CPU, AHB and APB busses clocks */
    RCC_ClkInitTypeDef RCC_ClkInitStruct;
    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2);

    // USB and RTC
    RCC_PeriphCLKInitTypeDef PeriphClkInit;
    PeriphClkInit.PeriphClockSelection = RCC_PERIPHCLK_RTC | RCC_PERIPHCLK_USB;
    PeriphClkInit.RTCClockSelection = RCC_RTCCLKSOURCE_LSE;
    PeriphClkInit.UsbClockSelection = RCC_USBCLKSOURCE_PLL_DIV1_5;
    HAL_RCCEx_PeriphCLKConfig(&PeriphClkInit);

    /**Configure the Systick */
    HAL_SYSTICK_Config(HAL_RCC_GetHCLKFreq() / 1000);
    HAL_SYSTICK_CLKSourceConfig(SYSTICK_CLKSOURCE_HCLK);
    HAL_NVIC_SetPriority(SysTick_IRQn, 0, 0);
}

void SysTick_Handler(void)
{
    HAL_IncTick();
}

void NMI_Handler(void)
{
}

void HardFault_Handler(void)
{
    while (1)
    {
    }
}

void MemManage_Handler(void)
{
    while (1)
    {
    }
}

void BusFault_Handler(void)
{
    while (1)
    {
    }
}

void UsageFault_Handler(void)
{
    while (1)
    {
    }
}

void SVC_Handler(void)
{
}

void DebugMon_Handler(void)
{
}

void PendSV_Handler(void)
{
}

/**
* @brief This function handles USB low priority or CAN RX0 interrupts.
*/
void USB_LP_CAN1_RX0_IRQHandler(void)
{
    HAL_PCD_IRQHandler(&hpcd_USB_FS);
}
