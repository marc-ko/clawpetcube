#ifndef LCD_SPI_H
#define LCD_SPI_H

#include "sys.h"

static inline void SPI1_Init(void) {}
static inline void SPI1_SetSpeed(u8 speed) { (void)speed; }
static inline u8 SPI1_ReadWriteByte(u8 tx) { HAL_SPI_Transmit(&hspi1, &tx, 1, HAL_MAX_DELAY); return 0; }
static inline u8 SPI1_ReadWriteByte_LVGL(u8 tx) { HAL_SPI_Transmit(&hspi1, &tx, 1, HAL_MAX_DELAY); return 0; }

#endif
