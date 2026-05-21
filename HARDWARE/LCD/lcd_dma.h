#ifndef __LCD_DMA_H
#define	__LCD_DMA_H	   
#include "sys.h"

extern uint8_t fresh_flag;	// 刷新完成标志

void LCD_DMA2_Config(u32 par,u32 mar,u16 ndtr);//配置DMAx_CHx
void LCD_DMA_Enable(DMA_Stream_TypeDef *DMA_Streamx, u32 par, u32 mar,u16 ndtr);	//使能一次DMA传输		   
#endif
