#ifndef LCD_DMA_H
#define LCD_DMA_H

#include "lcd_st7789_hal.h"

static inline void LCD_DMA2_Config(u32 par, u32 mar, u16 ndtr)
{
    (void)par;
    (void)mar;
    (void)ndtr;
}

#endif
