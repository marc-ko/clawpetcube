#include "main.h"
#include "app_debug.h"
#include "lcd_st7789_hal.h"
#include "esp8266_common_hal.h"
#include "mpu6050_hal_port.h"
#include "rtc_hal_compat.h"
#include "usart_compat_hal.h"
#include "FreeRTOS.h"
#include "task.h"
#include "semphr.h"
#include "lvgl.h"
#include <stdio.h>

static TaskHandle_t ui_task_handle;
static TaskHandle_t monitor_task_handle;
static SemaphoreHandle_t mpu_irq_sem;
static SemaphoreHandle_t rtc_tick_sem;

static void ui_task(void *arg);
static void monitor_task(void *arg);
static void create_init_screen(void);
extern void lv_port_disp_init(void);

void App_CreateTasks(void)
{
    mpu_irq_sem = xSemaphoreCreateBinary();
    rtc_tick_sem = xSemaphoreCreateBinary();

    xTaskCreate(ui_task, "ui", 1024, NULL, 3, &ui_task_handle);
    xTaskCreate(monitor_task, "monitor", 768, NULL, 2, &monitor_task_handle);
}

static void ui_task(void *arg)
{
    (void)arg;

    printf("LVGL init\r\n");
    lv_init();
    HAL_TIM_Base_Start_IT(&htim3);
    lv_port_disp_init();
    create_init_screen();
    printf("LVGL init screen ready\r\n");

    while (1) {
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

static void create_init_screen(void)
{
    lv_obj_t *screen = lv_scr_act();
    lv_obj_set_style_bg_color(screen, lv_color_make(0x00, 0x00, 0x00), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);

    lv_obj_t *bar = lv_bar_create(screen);
    lv_obj_set_pos(bar, 20, 80);
    lv_obj_set_size(bar, 200, 20);
    lv_obj_set_style_radius(bar, 10, LV_PART_MAIN);
    lv_obj_set_style_bg_color(bar, lv_color_make(0xff, 0xff, 0xff), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(bar, LV_OPA_80, LV_PART_MAIN);
    lv_obj_set_style_radius(bar, 10, LV_PART_INDICATOR);
    lv_obj_set_style_bg_color(bar, lv_color_make(0x21, 0x95, 0xf6), LV_PART_INDICATOR);
    lv_bar_set_mode(bar, LV_BAR_MODE_NORMAL);
    lv_bar_set_value(bar, 75, LV_ANIM_OFF);

    lv_obj_t *label = lv_label_create(screen);
    lv_obj_set_pos(label, 0, 110);
    lv_obj_set_size(label, 240, 70);
    lv_label_set_text(label, "LVGL display online");
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(label, lv_color_make(0xff, 0xff, 0xff), 0);
}

static void monitor_task(void *arg)
{
    (void)arg;

    printf("MPU6050 probe: %s\r\n", MPU_Init() == 0 ? "ok" : "fail");
    printf("USART2 ESP8266 and USART6 input RX armed\r\n");
    esp8266_sta_connect();
    TimeStruct network_time = esp8266_gettime();
    printf("RTC init: %u\r\n", My_RTC_Init(network_time));
    RTC_Set_WakeUp_Compat();

    while (1) {
        if (receive_end) {
            printf("USART6 event: %c%c\r\n", USART6_RX_BUF[0], USART6_RX_BUF[1]);
            receive_end = 0;
        }
        vTaskDelay(pdMS_TO_TICKS(100));
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    BaseType_t hpw = pdFALSE;
    if (GPIO_Pin == MPU6050_INT_Pin && mpu_irq_sem != NULL) {
        xSemaphoreGiveFromISR(mpu_irq_sem, &hpw);
        portYIELD_FROM_ISR(hpw);
    }
}

void HAL_RTCEx_WakeUpTimerEventCallback(RTC_HandleTypeDef *hrtc_cb)
{
    BaseType_t hpw = pdFALSE;
    if (hrtc_cb == &hrtc && rtc_tick_sem != NULL) {
        xSemaphoreGiveFromISR(rtc_tick_sem, &hpw);
        portYIELD_FROM_ISR(hpw);
    }
}
