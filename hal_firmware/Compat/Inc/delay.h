#ifndef DELAY_H
#define DELAY_H

#include "sys.h"

void delay_init(u8 sysclk_mhz);
void delay_us(u32 nus);
void delay_ms(u32 nms);
void delay_xms(u32 nms);

#endif
