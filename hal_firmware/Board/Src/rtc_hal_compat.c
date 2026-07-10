#include "rtc_hal_compat.h"
#include "ff.h"

u8 My_RTC_Init(TimeStruct esp8266_time)
{
    RTC_TimeTypeDef time = {0};
    RTC_DateTypeDef date = {0};

    time.Hours = esp8266_time.hour;
    time.Minutes = esp8266_time.minute;
    time.Seconds = esp8266_time.second;
    time.TimeFormat = RTC_HOURFORMAT12_AM;
    if (HAL_RTC_SetTime(&hrtc, &time, RTC_FORMAT_BIN) != HAL_OK) {
        return 1;
    }

    date.Year = (uint8_t)(esp8266_time.year % 100U);
    date.Month = esp8266_time.month;
    date.Date = esp8266_time.day;
    date.WeekDay = esp8266_time.week;
    if (HAL_RTC_SetDate(&hrtc, &date, RTC_FORMAT_BIN) != HAL_OK) {
        return 2;
    }
    return 0;
}

void RTC_Set_WakeUp_Compat(void)
{
    HAL_RTCEx_SetWakeUpTimer_IT(&hrtc, 0, RTC_WAKEUPCLOCK_CK_SPRE_16BITS);
}

DWORD get_fattime(void)
{
    RTC_TimeTypeDef time = {0};
    RTC_DateTypeDef date = {0};

    if (HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN) != HAL_OK ||
        HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN) != HAL_OK) {
        return ((DWORD)(2022U - 1980U) << 25) |
               ((DWORD)1U << 21) |
               ((DWORD)1U << 16);
    }

    return ((DWORD)(date.Year + 20U) << 25) |
           ((DWORD)date.Month << 21) |
           ((DWORD)date.Date << 16) |
           ((DWORD)time.Hours << 11) |
           ((DWORD)time.Minutes << 5) |
           ((DWORD)(time.Seconds / 2U));
}
