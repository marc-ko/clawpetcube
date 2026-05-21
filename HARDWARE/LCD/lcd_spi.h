#ifndef __LCD_SPI_H
#define __LCD_SPI_H
#include "sys.h"

void SPI1_Init(void);			 								// 初始化SPI1口
void SPI1_SetSpeed(u8 SpeedSet); 					// 设置SPI1速度   
u8 SPI1_ReadWriteByte(u8 TxData);					// SPI1总线读写一个字节
u8 SPI1_ReadWriteByte_LVGL(u8 TxData); 		// SPI1总线读写一个字节（寄存器版本）

#endif
