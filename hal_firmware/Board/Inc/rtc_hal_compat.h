#ifndef RTC_HAL_COMPAT_H
#define RTC_HAL_COMPAT_H

#include "sys.h"

typedef struct {
    u8 week;
    u16 year;
    u8 month;
    u8 day;
    u8 hour;
    u8 minute;
    u8 second;
} TimeStruct;

u8 My_RTC_Init(TimeStruct esp8266_time);
void RTC_Set_WakeUp_Compat(void);

#endif
