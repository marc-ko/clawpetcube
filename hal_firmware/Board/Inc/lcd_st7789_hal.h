#ifndef LCD_ST7789_HAL_H
#define LCD_ST7789_HAL_H

#include "sys.h"

#define LCD_W APP_LCD_WIDTH
#define LCD_H APP_LCD_HEIGHT

#define LCD_BLK_Clr() HAL_GPIO_WritePin(LCD_BLK_GPIO_Port, LCD_BLK_Pin, GPIO_PIN_RESET)
#define LCD_BLK_Set() HAL_GPIO_WritePin(LCD_BLK_GPIO_Port, LCD_BLK_Pin, GPIO_PIN_SET)
#define LCD_RES_Clr() HAL_GPIO_WritePin(LCD_RES_GPIO_Port, LCD_RES_Pin, GPIO_PIN_RESET)
#define LCD_RES_Set() HAL_GPIO_WritePin(LCD_RES_GPIO_Port, LCD_RES_Pin, GPIO_PIN_SET)
#define LCD_DC_Clr() HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_RESET)
#define LCD_DC_Set() HAL_GPIO_WritePin(LCD_DC_GPIO_Port, LCD_DC_Pin, GPIO_PIN_SET)
#define LCD_CS_Clr() HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_RESET)
#define LCD_CS_Set() HAL_GPIO_WritePin(LCD_CS_GPIO_Port, LCD_CS_Pin, GPIO_PIN_SET)

void LCD_Init(void);
void LCD_WR_REG(u8 dat);
void LCD_WR_DATA8(u8 dat);
void LCD_WR_DATA(u16 dat);
void LCD_WR_DATA_LVGL(u16 dat);
void LCD_Address_Set(u16 x1, u16 y1, u16 x2, u16 y2);
HAL_StatusTypeDef LCD_WritePixels(const uint16_t *pixels, uint32_t count);
HAL_StatusTypeDef LCD_WritePixels_DMA(const uint16_t *pixels, uint32_t count);
void LCD_DMA_TxComplete(void);

#endif
