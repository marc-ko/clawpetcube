#ifndef __LCD_INIT_H
#define __LCD_INIT_H
#include "sys.h"
#include "delay.h"
#include "lcd_spi.h"

#define FLIP_DISP_BY_PRISM 1 // Set whether to flip the display by prism: 0 or 1
#define USE_HORIZONTAL 0 // Set landscape or portrait display: 0 or 1 for portrait, 2 or 3 for landscape

#define LCD_W 240
#define LCD_H 240

#define LCD_BLK_Clr() GPIO_ResetBits(GPIOB, GPIO_Pin_0) // BLK
#define LCD_BLK_Set() GPIO_SetBits(GPIOB, GPIO_Pin_0)

#define LCD_RES_Clr() GPIO_ResetBits(GPIOC, GPIO_Pin_4) // RES
#define LCD_RES_Set() GPIO_SetBits(GPIOC, GPIO_Pin_4)

#define LCD_DC_Clr() GPIO_ResetBits(GPIOC, GPIO_Pin_5) // DC
#define LCD_DC_Set() GPIO_SetBits(GPIOC, GPIO_Pin_5)

#define LCD_CS_Clr() GPIO_ResetBits(GPIOB, GPIO_Pin_1) // CS
#define LCD_CS_Set() GPIO_SetBits(GPIOB, GPIO_Pin_1)

void LCD_GPIO_Init(void);
void LCD_Writ_Bus(u8 dat);                            // Simultaneous write of 8 bits of data
void LCD_WR_DATA8(u8 dat);                            // Write one byte
void LCD_WR_DATA(u16 dat);                            // Write two bytes
void LCD_WR_DATA_LVGL(u16 dat);                       // Write two bytes, LVGL version
void LCD_WR_REG(u8 dat);                              // Write cmd to register
void LCD_Address_Set(u16 x1, u16 y1, u16 x2, u16 y2); // Set the display area
void LCD_Init(void);
#endif
