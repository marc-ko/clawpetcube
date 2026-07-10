#include "delay.h"

void delay_init(u8 sysclk_mhz)
{
    (void)sysclk_mhz;
}

void delay_us(u32 nus)
{
    uint32_t cycles_per_us = HAL_RCC_GetHCLKFreq() / 1000000U;
    uint32_t start = DWT->CYCCNT;
    uint32_t ticks = nus * cycles_per_us;

    if ((CoreDebug->DEMCR & CoreDebug_DEMCR_TRCENA_Msk) == 0U) {
        CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
        DWT->CYCCNT = 0;
        DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
        start = DWT->CYCCNT;
    }

    while ((DWT->CYCCNT - start) < ticks) {
    }
}

void delay_ms(u32 nms)
{
    HAL_Delay(nms);
}

void delay_xms(u32 nms)
{
    HAL_Delay(nms);
}
