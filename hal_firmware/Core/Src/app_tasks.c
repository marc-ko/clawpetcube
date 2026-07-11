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
#include <string.h>

typedef enum
{
    NET_STATE_BOOTING,
    NET_STATE_WIFI_CONNECTING,
    NET_STATE_TIME_SYNCING,
    NET_STATE_ONLINE,
    NET_STATE_SYNC_WARN,
    NET_STATE_OFFLINE
} NetState;

typedef enum
{
    CLAW_STATE_WAITING,
    CLAW_STATE_CHECKING,
    CLAW_STATE_OK,
    CLAW_STATE_WARN,
    CLAW_STATE_OFFLINE
} ClawState;

static TaskHandle_t ui_task_handle;
static TaskHandle_t monitor_task_handle;
static SemaphoreHandle_t mpu_irq_sem;
static SemaphoreHandle_t rtc_tick_sem;
static lv_obj_t *status_pill;
static lv_obj_t *status_pill_label;
static lv_obj_t *time_label;
static lv_obj_t *date_label;
static lv_obj_t *mascot_obj;
static lv_obj_t *claw_state_label;
static lv_obj_t *claw_detail_label;
static lv_obj_t *footer_label;
static volatile uint8_t rtc_time_valid;
static volatile NetState net_state = NET_STATE_BOOTING;
static volatile ClawState claw_state = CLAW_STATE_WAITING;
static char sync_text[80] = "Starting";
static char claw_state_text[24] = "CLAW";
static char claw_detail_text[48] = "Waiting";

static void ui_task(void *arg);
static void monitor_task(void *arg);
static void create_clock_dashboard(void);
static void update_clock_dashboard(uint32_t seconds);
static void set_net_status(NetState state, const char *text);
static void set_claw_status(ClawState state, const char *title, const char *detail);
static void format_rtc_display(char *time_text, size_t time_len, char *date_text, size_t date_len);
static lv_obj_t *create_pixel_mascot(lv_obj_t *parent, int16_t x, int16_t y);
static void create_mascot_rect(lv_obj_t *parent, int16_t x, int16_t y, int16_t w, int16_t h, lv_color_t color);
static void start_mascot_float(lv_obj_t *obj, int16_t base_y);
static const char *net_state_text(NetState state);
static lv_color_t net_state_color(NetState state);
static lv_color_t claw_state_color(ClawState state);
extern void lv_port_disp_init(void);

void App_CreateTasks(void)
{
    mpu_irq_sem = xSemaphoreCreateBinary();
    rtc_tick_sem = xSemaphoreCreateBinary();

    xTaskCreate(ui_task, "ui", 1024, NULL, 3, &ui_task_handle);
    xTaskCreate(monitor_task, "net", 1024, NULL, 2, &monitor_task_handle);
}

static void ui_task(void *arg)
{
    (void)arg;

    printf("LVGL init\r\n");
    lv_init();
    HAL_TIM_Base_Start_IT(&htim3);
    lv_port_disp_init();
    create_clock_dashboard();
    update_clock_dashboard(0);
    lv_timer_handler();
    printf("LVGL clock dashboard ready\r\n");

    uint32_t shown_seconds = UINT32_MAX;
    while (1)
    {
        uint32_t seconds = xTaskGetTickCount() / configTICK_RATE_HZ;
        if (seconds != shown_seconds)
        {
            update_clock_dashboard(seconds);
            shown_seconds = seconds;
        }
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

static void create_clock_dashboard(void)
{
    lv_obj_t *screen = lv_scr_act();
    lv_obj_clean(screen);
    lv_obj_set_style_bg_color(screen, lv_color_make(0x04, 0x07, 0x0c), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);

    status_pill = lv_obj_create(screen);
    lv_obj_set_pos(status_pill, 158, 10);
    lv_obj_set_size(status_pill, 72, 22);
    lv_obj_set_style_radius(status_pill, 12, 0);
    lv_obj_set_style_border_width(status_pill, 0, 0);
    lv_obj_set_style_pad_all(status_pill, 0, 0);

    status_pill_label = lv_label_create(status_pill);
    lv_obj_center(status_pill_label);
    lv_obj_set_style_text_color(status_pill_label, lv_color_make(0x02, 0x08, 0x0a), 0);

    time_label = lv_label_create(screen);
    lv_obj_set_pos(time_label, 9, 5);
    lv_obj_set_size(time_label, 135, 31);
    lv_obj_set_style_text_font(time_label, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(time_label, lv_color_make(0x63, 0xe6, 0xbe), 0);

    date_label = lv_label_create(screen);
    lv_obj_set_pos(date_label, 11, 36);
    lv_obj_set_size(date_label, 155, 18);
    lv_obj_set_style_text_font(date_label, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(date_label, lv_color_make(0x89, 0x95, 0xa3), 0);

    mascot_obj = create_pixel_mascot(screen, 18, 88);
    start_mascot_float(mascot_obj, 88);

    claw_state_label = lv_label_create(screen);
    lv_obj_set_pos(claw_state_label, 136, 92);
    lv_obj_set_size(claw_state_label, 92, 22);
    lv_obj_set_style_text_align(claw_state_label, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_style_text_font(claw_state_label, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(claw_state_label, lv_color_make(0xa9, 0xb4, 0xc0), 0);

    claw_detail_label = lv_label_create(screen);
    lv_obj_set_pos(claw_detail_label, 126, 118);
    lv_obj_set_size(claw_detail_label, 102, 54);
    lv_label_set_long_mode(claw_detail_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_align(claw_detail_label, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_style_text_color(claw_detail_label, lv_color_make(0x89, 0x95, 0xa3), 0);

    footer_label = lv_label_create(screen);
    lv_obj_set_pos(footer_label, 10, 210);
    lv_obj_set_size(footer_label, 220, 24);
    lv_label_set_long_mode(footer_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_align(footer_label, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_style_text_color(footer_label, lv_color_make(0x7c, 0x88, 0x95), 0);
}

static lv_obj_t *create_pixel_mascot(lv_obj_t *parent, int16_t x, int16_t y)
{
    const int16_t u = 6;
    lv_obj_t *sprite = lv_obj_create(parent);
    lv_color_t body = lv_color_make(0xf2, 0x57, 0x45);
    lv_color_t eye = lv_color_make(0x02, 0x03, 0x05);

    lv_obj_set_pos(sprite, x, y);
    lv_obj_set_size(sprite, 18 * u, 14 * u);
    lv_obj_set_style_bg_opa(sprite, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(sprite, 0, 0);
    lv_obj_set_style_pad_all(sprite, 0, 0);

    create_mascot_rect(sprite, 3 * u, 0 * u, 12 * u, 4 * u, body);
    create_mascot_rect(sprite, 0 * u, 4 * u, 18 * u, 4 * u, body);
    create_mascot_rect(sprite, 3 * u, 8 * u, 12 * u, 2 * u, body);

    create_mascot_rect(sprite, 3 * u, 10 * u, 2 * u, 4 * u, body);
    create_mascot_rect(sprite, 6 * u, 10 * u, 2 * u, 4 * u, body);
    create_mascot_rect(sprite, 10 * u, 10 * u, 2 * u, 4 * u, body);
    create_mascot_rect(sprite, 13 * u, 10 * u, 2 * u, 4 * u, body);

    create_mascot_rect(sprite, 5 * u, 2 * u, 2 * u, 2 * u, eye);
    create_mascot_rect(sprite, 11 * u, 2 * u, 2 * u, 2 * u, eye);

    return sprite;
}

static void create_mascot_rect(lv_obj_t *parent, int16_t x, int16_t y, int16_t w, int16_t h, lv_color_t color)
{
    lv_obj_t *rect = lv_obj_create(parent);
    lv_obj_set_pos(rect, x, y);
    lv_obj_set_size(rect, w, h);
    lv_obj_set_style_radius(rect, 0, 0);
    lv_obj_set_style_border_width(rect, 0, 0);
    lv_obj_set_style_bg_color(rect, color, 0);
    lv_obj_set_style_bg_opa(rect, LV_OPA_COVER, 0);
    lv_obj_set_style_pad_all(rect, 0, 0);
}

static void start_mascot_float(lv_obj_t *obj, int16_t base_y)
{
    lv_anim_t anim;

    lv_anim_init(&anim);
    lv_anim_set_var(&anim, obj);
    lv_anim_set_exec_cb(&anim, (lv_anim_exec_xcb_t)lv_obj_set_y);
    lv_anim_set_values(&anim, base_y, base_y - 7);
    lv_anim_set_time(&anim, 850);
    lv_anim_set_playback_time(&anim, 850);
    lv_anim_set_repeat_count(&anim, LV_ANIM_REPEAT_INFINITE);
    lv_anim_set_path_cb(&anim, lv_anim_path_ease_in_out);
    lv_anim_start(&anim);
}

static void update_clock_dashboard(uint32_t seconds)
{
    char time_text[24];
    char date_text[32];
    char footer[48];
    NetState state;
    ClawState claw_snapshot;
    char claw_title[24];
    char claw_detail[48];

    taskENTER_CRITICAL();
    state = net_state;
    claw_snapshot = claw_state;
    strncpy(claw_title, claw_state_text, sizeof(claw_title) - 1U);
    strncpy(claw_detail, claw_detail_text, sizeof(claw_detail) - 1U);
    taskEXIT_CRITICAL();
    claw_title[sizeof(claw_title) - 1U] = '\0';
    claw_detail[sizeof(claw_detail) - 1U] = '\0';

    format_rtc_display(time_text, sizeof(time_text), date_text, sizeof(date_text));

    lv_obj_set_style_bg_color(status_pill, net_state_color(state), 0);
    lv_label_set_text(status_pill_label, net_state_text(state));
    lv_label_set_text(time_label, time_text);
    lv_label_set_text(date_label, date_text);
    lv_label_set_text(claw_state_label, claw_title);
    lv_label_set_text(claw_detail_label, claw_detail);
    lv_obj_set_style_text_color(claw_state_label, claw_state_color(claw_snapshot), 0);

    if (state == NET_STATE_OFFLINE || state == NET_STATE_SYNC_WARN)
    {
        snprintf(footer, sizeof(footer), "UNSYNCED");
        lv_obj_set_style_text_color(footer_label, lv_color_make(0xff, 0x66, 0x66), 0);
    }
    else
    {
        snprintf(footer, sizeof(footer), "Uptime %lus", (unsigned long)seconds);
        lv_obj_set_style_text_color(footer_label, lv_color_make(0x7c, 0x88, 0x95), 0);
    }
    lv_label_set_text(footer_label, footer);
}

static void monitor_task(void *arg)
{
    (void)arg;

    printf("Network/time task started\r\n");
    set_net_status(NET_STATE_WIFI_CONNECTING, "Connecting ESP8266 WiFi");
    set_claw_status(CLAW_STATE_WAITING, "CLAW", "Waiting");
    esp8266_sta_connect();
    set_net_status(NET_STATE_TIME_SYNCING, "WiFi connected, syncing time");

    TickType_t last_time_sync = 0;
    TickType_t last_health_check = 0;
    TickType_t last_status_check = 0;
    uint8_t first_loop = 1U;

    while (1)
    {
        TickType_t now = xTaskGetTickCount();

        if (first_loop || (now - last_time_sync) >= pdMS_TO_TICKS(30UL * 60UL * 1000UL))
        {
            TimeStruct network_time = esp8266_gettime();
            last_time_sync = now;
            if (network_time.year != 0U && My_RTC_Init(network_time) == 0U)
            {
                char text[80];
                taskENTER_CRITICAL();
                rtc_time_valid = 1U;
                taskEXIT_CRITICAL();

                snprintf(text, sizeof(text), "Synced HKO %04u-%02u-%02u %02u:%02u:%02u",
                         network_time.year, network_time.month, network_time.day,
                         network_time.hour, network_time.minute, network_time.second);
                set_net_status(NET_STATE_ONLINE, text);
                printf("RTC sync OK\r\n");
            }
            else
            {
                set_net_status(rtc_time_valid ? NET_STATE_SYNC_WARN : NET_STATE_OFFLINE,
                               rtc_time_valid ? "Time sync retry failed" : "Waiting for network time");
                printf("RTC sync failed\r\n");
            }
        }

        if (first_loop || (now - last_health_check) >= pdMS_TO_TICKS(30UL * 1000UL))
        {
            OpenClawHealthStruct health;
            set_claw_status(CLAW_STATE_CHECKING, "CLAW", "Health...");
            health = ESP8266_GetOpenClawHealth();
            last_health_check = xTaskGetTickCount();
            if (health.ok)
            {
                set_claw_status(CLAW_STATE_OK, "CLAW OK", health.status);
            }
            else if (health.error_code == 2U)
            {
                set_claw_status(CLAW_STATE_WARN, "CLAW TLS", "Need proxy");
            }
            else if (health.error_code == 3U)
            {
                set_claw_status(CLAW_STATE_OFFLINE, "PROXY ERR", "TCP fail");
            }
            else
            {
                set_claw_status(CLAW_STATE_OFFLINE, "CLAW OFF", "Health fail");
            }
        }

        if (first_loop || (xTaskGetTickCount() - last_status_check) >= pdMS_TO_TICKS(5UL * 60UL * 1000UL))
        {
            OpenClawStatusStruct status;
            char detail[48];
            set_claw_status(CLAW_STATE_CHECKING, "CLAW", "Status...");
            status = ESP8266_GetOpenClawStatus();
            last_status_check = xTaskGetTickCount();
            if (status.ok)
            {
                snprintf(detail, sizeof(detail), "P%u Cron %u/%u\nD%u%% M%u%%",
                         status.process_count, status.cron_ok, status.cron_total,
                         status.disk_percent, status.memory_percent);
                set_claw_status(status.cron_failed > 0U ? CLAW_STATE_WARN : CLAW_STATE_OK,
                                status.cron_failed > 0U ? "CLAW WARN" : "CLAW OK",
                                detail);
            }
            else
            {
                set_claw_status(CLAW_STATE_WARN,
                                status.error_code == 2U ? "CLAW TLS" : (status.error_code == 3U ? "PROXY ERR" : "CLAW ?"),
                                status.error_code == 2U ? "Need proxy" : (status.error_code == 3U ? "TCP fail" : "Status wait"));
            }
        }

        first_loop = 0U;
        vTaskDelay(pdMS_TO_TICKS(1000UL));
    }
}

static void set_net_status(NetState state, const char *text)
{
    taskENTER_CRITICAL();
    net_state = state;
    snprintf(sync_text, sizeof(sync_text), "%s", text);
    taskEXIT_CRITICAL();
}

static void set_claw_status(ClawState state, const char *title, const char *detail)
{
    taskENTER_CRITICAL();
    claw_state = state;
    snprintf(claw_state_text, sizeof(claw_state_text), "%s", title);
    snprintf(claw_detail_text, sizeof(claw_detail_text), "%s", detail);
    taskEXIT_CRITICAL();
}

static void format_rtc_display(char *time_text, size_t time_len, char *date_text, size_t date_len)
{
    RTC_TimeTypeDef time = {0};
    RTC_DateTypeDef date = {0};
    uint8_t valid;

    taskENTER_CRITICAL();
    valid = rtc_time_valid;
    taskEXIT_CRITICAL();

    if (valid == 0U ||
        HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN) != HAL_OK ||
        HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN) != HAL_OK)
    {
        snprintf(time_text, time_len, "--:--:--");
        snprintf(date_text, date_len, "Waiting for HKT");
        return;
    }

    snprintf(time_text, time_len, "%02u:%02u:%02u",
             time.Hours, time.Minutes, time.Seconds);
    snprintf(date_text, date_len, "20%02u-%02u-%02u",
             date.Year, date.Month, date.Date);
}

static const char *net_state_text(NetState state)
{
    switch (state)
    {
    case NET_STATE_WIFI_CONNECTING:
        return "WIFI";
    case NET_STATE_TIME_SYNCING:
        return "SYNC";
    case NET_STATE_ONLINE:
        return "ONLINE";
    case NET_STATE_SYNC_WARN:
        return "STALE";
    case NET_STATE_OFFLINE:
        return "OFFLINE";
    case NET_STATE_BOOTING:
    default:
        return "BOOT";
    }
}

static lv_color_t net_state_color(NetState state)
{
    switch (state)
    {
    case NET_STATE_WIFI_CONNECTING:
    case NET_STATE_TIME_SYNCING:
        return lv_color_make(0x42, 0x9d, 0xff);
    case NET_STATE_ONLINE:
        return lv_color_make(0x55, 0xe0, 0x82);
    case NET_STATE_SYNC_WARN:
        return lv_color_make(0xff, 0xd1, 0x66);
    case NET_STATE_OFFLINE:
        return lv_color_make(0xff, 0x66, 0x66);
    case NET_STATE_BOOTING:
    default:
        return lv_color_make(0x8a, 0x96, 0xa3);
    }
}

static lv_color_t claw_state_color(ClawState state)
{
    switch (state)
    {
    case CLAW_STATE_CHECKING:
        return lv_color_make(0x42, 0x9d, 0xff);
    case CLAW_STATE_OK:
        return lv_color_make(0x55, 0xe0, 0x82);
    case CLAW_STATE_WARN:
        return lv_color_make(0xff, 0xd1, 0x66);
    case CLAW_STATE_OFFLINE:
        return lv_color_make(0xff, 0x66, 0x66);
    case CLAW_STATE_WAITING:
    default:
        return lv_color_make(0xa9, 0xb4, 0xc0);
    }
}

void HAL_GPIO_EXTI_Callback(uint16_t GPIO_Pin)
{
    BaseType_t hpw = pdFALSE;
    if (GPIO_Pin == MPU6050_INT_Pin && mpu_irq_sem != NULL)
    {
        xSemaphoreGiveFromISR(mpu_irq_sem, &hpw);
        portYIELD_FROM_ISR(hpw);
    }
}

void HAL_RTCEx_WakeUpTimerEventCallback(RTC_HandleTypeDef *hrtc_cb)
{
    BaseType_t hpw = pdFALSE;
    if (hrtc_cb == &hrtc && rtc_tick_sem != NULL)
    {
        xSemaphoreGiveFromISR(rtc_tick_sem, &hpw);
        portYIELD_FROM_ISR(hpw);
    }
}
