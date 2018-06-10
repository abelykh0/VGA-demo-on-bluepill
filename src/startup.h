#ifndef __STARTUP_H
#define __STARTUP_H

#include "stm32f1xx_hal.h"
#include "usb/usbd_def.h"
#include "usb/usbd_cdc.h"
#include "usb/usbd_cdc_if.h"

#ifdef __cplusplus
extern "C" {
#endif

void setup();
void loop();

extern USBD_HandleTypeDef hUsbDeviceFS;
void UsbDeviceInit(uint32_t preemptPriority, uint32_t subPriority);

extern RTC_HandleTypeDef hRtc;
void RtcInit();

#ifdef __cplusplus
}
#endif

#endif