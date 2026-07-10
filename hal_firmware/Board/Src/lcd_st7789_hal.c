#include "lcd_st7789_hal.h"
#include "delay.h"

static void lcd_write_bus(uint8_t dat)
{
    LCD_CS_Clr();
    HAL_SPI_Transmit(&hspi1, &dat, 1, HAL_MAX_DELAY);
    LCD_CS_Set();
}

void LCD_WR_DATA8(u8 dat)
{
    LCD_DC_Set();
    lcd_write_bus(dat);
}

void LCD_WR_DATA(u16 dat)
{
    uint8_t bytes[2] = {(uint8_t)(dat >> 8), (uint8_t)dat};
    LCD_DC_Set();
    LCD_CS_Clr();
    HAL_SPI_Transmit(&hspi1, bytes, sizeof(bytes), HAL_MAX_DELAY);
    LCD_CS_Set();
}

void LCD_WR_DATA_LVGL(u16 dat)
{
    LCD_WR_DATA(dat);
}

void LCD_WR_REG(u8 dat)
{
    LCD_DC_Clr();
    lcd_write_bus(dat);
    LCD_DC_Set();
}

void LCD_Address_Set(u16 x1, u16 y1, u16 x2, u16 y2)
{
    LCD_WR_REG(0x2A);
    LCD_WR_DATA(x1);
    LCD_WR_DATA(x2);
    LCD_WR_REG(0x2B);
    LCD_WR_DATA(y1);
    LCD_WR_DATA(y2);
    LCD_WR_REG(0x2C);
}

void LCD_WritePixels_DMA(const uint16_t *pixels, uint32_t count)
{
    LCD_DC_Set();
    LCD_CS_Clr();
    if (HAL_SPI_Transmit_DMA(&hspi1, (uint8_t *)pixels, count * 2U) != HAL_OK) {
        LCD_CS_Set();
    }
}

void LCD_DMA_TxComplete(void)
{
    LCD_CS_Set();
}

static void lcd_clear_color(uint16_t color)
{
    uint32_t pixels = LCD_W * LCD_H;

    LCD_Address_Set(0, 0, LCD_W - 1, LCD_H - 1);
    while (pixels--) {
        LCD_WR_DATA(color);
    }
}

void LCD_Init(void)
{
    LCD_CS_Set();
    LCD_DC_Set();
    LCD_BLK_Set();

    LCD_RES_Clr();
    delay_ms(100);
    LCD_RES_Set();
    delay_ms(100);
    LCD_BLK_Set();
    delay_ms(100);

    LCD_WR_REG(0x11);
    delay_ms(120);

    LCD_WR_REG(0x36);
#if APP_LCD_FLIP_BY_PRISM
    LCD_WR_DATA8(0x40);
#else
    LCD_WR_DATA8(0x00);
#endif

    LCD_WR_REG(0x3A);
    LCD_WR_DATA8(0x05);

    LCD_WR_REG(0xB2);
    LCD_WR_DATA8(0x0C);
    LCD_WR_DATA8(0x0C);
    LCD_WR_DATA8(0x00);
    LCD_WR_DATA8(0x33);
    LCD_WR_DATA8(0x33);
    LCD_WR_REG(0xB7);
    LCD_WR_DATA8(0x35);
    LCD_WR_REG(0xBB);
    LCD_WR_DATA8(0x19);
    LCD_WR_REG(0xC0);
    LCD_WR_DATA8(0x2C);
    LCD_WR_REG(0xC2);
    LCD_WR_DATA8(0x01);
    LCD_WR_REG(0xC3);
    LCD_WR_DATA8(0x12);
    LCD_WR_REG(0xC4);
    LCD_WR_DATA8(0x20);
    LCD_WR_REG(0xC6);
    LCD_WR_DATA8(0x0F);
    LCD_WR_REG(0xD0);
    LCD_WR_DATA8(0xA4);
    LCD_WR_DATA8(0xA1);

    LCD_WR_REG(0xE0);
    LCD_WR_DATA8(0xD0); LCD_WR_DATA8(0x04); LCD_WR_DATA8(0x0D); LCD_WR_DATA8(0x11);
    LCD_WR_DATA8(0x13); LCD_WR_DATA8(0x2B); LCD_WR_DATA8(0x3F); LCD_WR_DATA8(0x54);
    LCD_WR_DATA8(0x4C); LCD_WR_DATA8(0x18); LCD_WR_DATA8(0x0D); LCD_WR_DATA8(0x0B);
    LCD_WR_DATA8(0x1F); LCD_WR_DATA8(0x23);

    LCD_WR_REG(0xE1);
    LCD_WR_DATA8(0xD0); LCD_WR_DATA8(0x04); LCD_WR_DATA8(0x0C); LCD_WR_DATA8(0x11);
    LCD_WR_DATA8(0x13); LCD_WR_DATA8(0x2C); LCD_WR_DATA8(0x3F); LCD_WR_DATA8(0x44);
    LCD_WR_DATA8(0x51); LCD_WR_DATA8(0x2F); LCD_WR_DATA8(0x1F); LCD_WR_DATA8(0x1F);
    LCD_WR_DATA8(0x20); LCD_WR_DATA8(0x23);

    LCD_WR_REG(0x29);
    delay_ms(20);
    lcd_clear_color(0x0000);
}
