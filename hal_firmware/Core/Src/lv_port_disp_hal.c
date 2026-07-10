#include "main.h"
#include "lcd_st7789_hal.h"

#if __has_include("lvgl.h")
#include "lvgl.h"

static lv_disp_draw_buf_t draw_buf;
static lv_color_t buf1[APP_LCD_WIDTH * 20];
static lv_color_t buf2[APP_LCD_WIDTH * 20];
static lv_disp_drv_t disp_drv;

static void disp_flush(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_p)
{
    uint32_t w = (uint32_t)(area->x2 - area->x1 + 1);
    uint32_t h = (uint32_t)(area->y2 - area->y1 + 1);

    LCD_Address_Set((uint16_t)area->x1, (uint16_t)area->y1, (uint16_t)area->x2, (uint16_t)area->y2);
    LCD_WritePixels_DMA((const uint16_t *)color_p, w * h);
    (void)drv;
}

void lv_port_disp_init(void)
{
    LCD_Init();
    lv_disp_draw_buf_init(&draw_buf, buf1, buf2, APP_LCD_WIDTH * 20);
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = APP_LCD_WIDTH;
    disp_drv.ver_res = APP_LCD_HEIGHT;
    disp_drv.flush_cb = disp_flush;
    disp_drv.draw_buf = &draw_buf;
    lv_disp_drv_register(&disp_drv);
}

void LCD_LVGL_FlushReady(void)
{
    lv_disp_flush_ready(&disp_drv);
}

#else
void lv_port_disp_init(void)
{
    LCD_Init();
}

void LCD_LVGL_FlushReady(void)
{
}
#endif
