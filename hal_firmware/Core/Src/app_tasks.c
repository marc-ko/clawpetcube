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
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define CUBE_MESSAGE_TTL_SECONDS (45UL * 60UL)
#define HKT_OFFSET_SECONDS (8L * 60L * 60L)
#define MASCOT_FACE_PREVIEW 0
#define MASCOT_BASE_X 13
#define MASCOT_BASE_Y 74
#define MASCOT_WANDER_LEFT_X 2
#define MASCOT_WANDER_RIGHT_X 54

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

typedef enum
{
    MASCOT_FACE_HAPPY,
    MASCOT_FACE_BLINK,
    MASCOT_FACE_SLEEPY,
    MASCOT_FACE_CONCERNED,
    MASCOT_FACE_XX
} MascotFace;

static TaskHandle_t ui_task_handle;
static TaskHandle_t monitor_task_handle;
static SemaphoreHandle_t mpu_irq_sem;
static SemaphoreHandle_t rtc_tick_sem;
static lv_obj_t *status_pill;
static lv_obj_t *status_pill_label;
static lv_obj_t *time_label;
static lv_obj_t *date_label;
static lv_obj_t *mascot_obj;
static lv_obj_t *mascot_face_layer;
static lv_obj_t *speech_bubble;
static lv_obj_t *speech_tail;
static lv_obj_t *speech_tail_tip;
static lv_obj_t *speech_label;
static lv_obj_t *cron_card;
static lv_obj_t *cron_value_label;
static lv_obj_t *health_card;
static lv_obj_t *health_value_label;
static lv_obj_t *error_card;
static lv_obj_t *error_title_label;
static lv_obj_t *error_value_label;
static volatile uint8_t rtc_time_valid;
static volatile uint8_t claw_metrics_valid;
static volatile uint8_t claw_degraded;
static volatile uint8_t claw_cron_warning;
static volatile uint8_t claw_alert_active;
static volatile NetState net_state = NET_STATE_BOOTING;
static volatile ClawState claw_state = CLAW_STATE_WAITING;
static char sync_text[80] = "Starting";
static char claw_state_text[24] = "CLAW";
static char claw_health_text[24] = "Waiting";
static char claw_process_text[12] = "--";
static char claw_cron_text[16] = "--";
static char claw_disk_text[12] = "--";
static char claw_mem_text[12] = "--";
static char claw_error_text[24] = "none";
static char speech_text[80] = "";
static char cube_message_text[96] = "";
static char cube_message_from[24] = "";
static char cube_message_timestamp[32] = "";
static char claw_uptime_text[24] = "No data";
static MascotFace mascot_current_face = MASCOT_FACE_HAPPY;
static uint8_t mascot_face_valid;
static uint8_t mascot_wander_active;
static uint8_t mascot_wander_dir;
static TickType_t mascot_next_move_tick;

static void ui_task(void *arg);
static void monitor_task(void *arg);
static void create_clock_dashboard(void);
static void update_clock_dashboard(uint32_t seconds);
static void set_net_status(NetState state, const char *text);
static void set_claw_health(ClawState state, const char *title, const char *health);
static void set_claw_metrics(const OpenClawStatusStruct *status);
static void set_cube_message(const OpenClawMessageStruct *message);
static uint8_t cube_message_is_expired(const char *timestamp, uint32_t *age_seconds);
static void format_rtc_display(char *time_text, size_t time_len, char *date_text, size_t date_len);
static lv_obj_t *create_card(lv_obj_t *parent, int16_t x, int16_t y, int16_t w, int16_t h);
static lv_obj_t *create_metric_card(lv_obj_t *parent, lv_obj_t **card, int16_t x, const char *title);
static void update_metric_style(lv_obj_t *card, lv_obj_t *value, lv_color_t bg, lv_color_t border, lv_color_t text);
static void update_alert_pill(NetState net, ClawState claw, uint8_t alert_active);
static lv_obj_t *create_pixel_mascot(lv_obj_t *parent, int16_t x, int16_t y);
static void draw_mascot_face(MascotFace face);
#if MASCOT_FACE_PREVIEW
static void update_mascot_face_preview(TickType_t now);
static const char *mascot_face_name(MascotFace face);
#endif
static void set_mascot_face(MascotFace face);
static void update_mascot_behavior(uint32_t seconds, uint8_t bubble_visible, NetState net, ClawState claw, uint8_t alert_active);
static void set_mascot_wander(uint8_t enabled, TickType_t now);
static void mascot_wander_ready_cb(lv_anim_t *anim);
static uint8_t mascot_sleepy_time(void);
static void create_mascot_rect(lv_obj_t *parent, int16_t x, int16_t y, int16_t w, int16_t h, lv_color_t color);
static void start_mascot_float(lv_obj_t *obj, int16_t base_y);
static uint8_t get_current_hkt_seconds(int64_t *out);
static uint8_t parse_timestamp_hkt_seconds(const char *timestamp, int64_t *out);
static int64_t date_time_to_seconds(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second);
static uint8_t is_leap_year_local(uint16_t year);
static const char *net_state_text(NetState state);
static lv_color_t net_state_color(NetState state);
static lv_color_t claw_state_color(ClawState state);
extern void lv_port_disp_init(void);

void App_CreateTasks(void)
{
    mpu_irq_sem = xSemaphoreCreateBinary();
    rtc_tick_sem = xSemaphoreCreateBinary();

    xTaskCreate(ui_task, "ui", 1024, NULL, 3, &ui_task_handle);
    xTaskCreate(monitor_task, "net", 1280, NULL, 2, &monitor_task_handle);
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
        TickType_t now = xTaskGetTickCount();
        uint32_t seconds = now / configTICK_RATE_HZ;
        if (seconds != shown_seconds)
        {
            update_clock_dashboard(seconds);
            shown_seconds = seconds;
        }
#if MASCOT_FACE_PREVIEW
        update_mascot_face_preview(now);
#endif
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
    lv_obj_set_pos(status_pill, 154, 8);
    lv_obj_set_size(status_pill, 76, 22);
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

    mascot_obj = create_pixel_mascot(screen, MASCOT_BASE_X, MASCOT_BASE_Y);
    start_mascot_float(mascot_obj, MASCOT_BASE_Y);

    speech_bubble = create_card(screen, 132, 62, 98, 58);
    lv_obj_set_style_radius(speech_bubble, 14, 0);
    lv_obj_set_style_bg_color(speech_bubble, lv_color_make(0x17, 0x22, 0x30), 0);
    lv_obj_set_style_border_color(speech_bubble, lv_color_make(0x4b, 0x63, 0x79), 0);
    lv_obj_set_style_shadow_width(speech_bubble, 7, 0);
    lv_obj_set_style_shadow_opa(speech_bubble, LV_OPA_30, 0);

    speech_tail = lv_obj_create(screen);
    lv_obj_set_pos(speech_tail, 124, 93);
    lv_obj_set_size(speech_tail, 13, 9);
    lv_obj_set_style_radius(speech_tail, 5, 0);
    lv_obj_set_style_bg_color(speech_tail, lv_color_make(0x17, 0x22, 0x30), 0);
    lv_obj_set_style_bg_opa(speech_tail, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(speech_tail, 0, 0);
    lv_obj_set_style_pad_all(speech_tail, 0, 0);

    speech_tail_tip = lv_obj_create(screen);
    lv_obj_set_pos(speech_tail_tip, 119, 96);
    lv_obj_set_size(speech_tail_tip, 8, 6);
    lv_obj_set_style_radius(speech_tail_tip, 4, 0);
    lv_obj_set_style_bg_color(speech_tail_tip, lv_color_make(0x17, 0x22, 0x30), 0);
    lv_obj_set_style_bg_opa(speech_tail_tip, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(speech_tail_tip, 0, 0);
    lv_obj_set_style_pad_all(speech_tail_tip, 0, 0);

    speech_label = lv_label_create(speech_bubble);
    lv_obj_set_pos(speech_label, 8, 8);
    lv_obj_set_size(speech_label, 82, 42);
    lv_obj_set_style_text_font(speech_label, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(speech_label, lv_color_make(0xf1, 0xf7, 0xff), 0);
    lv_label_set_long_mode(speech_label, LV_LABEL_LONG_WRAP);
    lv_label_set_text(speech_label, speech_text);
    lv_obj_add_flag(speech_bubble, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(speech_tail, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_flag(speech_tail_tip, LV_OBJ_FLAG_HIDDEN);

    cron_value_label = create_metric_card(screen, &cron_card, 10, "CRON");
    health_value_label = create_metric_card(screen, &health_card, 86, "HEALTH");
    error_value_label = create_metric_card(screen, &error_card, 162, "ERROR");
}

static lv_obj_t *create_pixel_mascot(lv_obj_t *parent, int16_t x, int16_t y)
{
    const int16_t u = 6;
    lv_obj_t *sprite = lv_obj_create(parent);
    lv_color_t body = lv_color_make(0xf2, 0x57, 0x45);

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

    mascot_face_layer = lv_obj_create(sprite);
    lv_obj_set_pos(mascot_face_layer, 0, 0);
    lv_obj_set_size(mascot_face_layer, 18 * u, 8 * u);
    lv_obj_set_style_bg_opa(mascot_face_layer, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(mascot_face_layer, 0, 0);
    lv_obj_set_style_pad_all(mascot_face_layer, 0, 0);
    set_mascot_face(MASCOT_FACE_HAPPY);

    return sprite;
}

static void draw_mascot_face(MascotFace face)
{
    const int16_t u = 6;
    lv_color_t eye = lv_color_make(0x02, 0x03, 0x05);

    if (mascot_face_layer == NULL)
    {
        return;
    }

    lv_obj_clean(mascot_face_layer);

    switch (face)
    {
    case MASCOT_FACE_BLINK:
        create_mascot_rect(mascot_face_layer, 5 * u, 3 * u, 2 * u, 1 * u, eye);
        create_mascot_rect(mascot_face_layer, 11 * u, 3 * u, 2 * u, 1 * u, eye);
        break;

    case MASCOT_FACE_SLEEPY:
        create_mascot_rect(mascot_face_layer, 4 * u, 3 * u, 3 * u, 1 * u, eye);
        create_mascot_rect(mascot_face_layer, 11 * u, 3 * u, 3 * u, 1 * u, eye);
        create_mascot_rect(mascot_face_layer, 8 * u, 6 * u, 1 * u, 1 * u, eye);
        break;

    case MASCOT_FACE_CONCERNED:
        create_mascot_rect(mascot_face_layer, 4 * u, 1 * u, 3 * u, 1 * u, eye);
        create_mascot_rect(mascot_face_layer, 5 * u, 2 * u, 2 * u, 2 * u, eye);
        create_mascot_rect(mascot_face_layer, 11 * u, 2 * u, 2 * u, 2 * u, eye);
        create_mascot_rect(mascot_face_layer, 11 * u, 1 * u, 3 * u, 1 * u, eye);
        create_mascot_rect(mascot_face_layer, 8 * u, 6 * u, 3 * u, 1 * u, eye);
        break;

    case MASCOT_FACE_XX:
        create_mascot_rect(mascot_face_layer, 4 * u, 1 * u, 1 * u, 1 * u, eye);
        create_mascot_rect(mascot_face_layer, 6 * u, 1 * u, 1 * u, 1 * u, eye);
        create_mascot_rect(mascot_face_layer, 5 * u, 2 * u, 1 * u, 1 * u, eye);
        create_mascot_rect(mascot_face_layer, 4 * u, 3 * u, 1 * u, 1 * u, eye);
        create_mascot_rect(mascot_face_layer, 6 * u, 3 * u, 1 * u, 1 * u, eye);
        create_mascot_rect(mascot_face_layer, 11 * u, 1 * u, 1 * u, 1 * u, eye);
        create_mascot_rect(mascot_face_layer, 13 * u, 1 * u, 1 * u, 1 * u, eye);
        create_mascot_rect(mascot_face_layer, 12 * u, 2 * u, 1 * u, 1 * u, eye);
        create_mascot_rect(mascot_face_layer, 11 * u, 3 * u, 1 * u, 1 * u, eye);
        create_mascot_rect(mascot_face_layer, 13 * u, 3 * u, 1 * u, 1 * u, eye);
        create_mascot_rect(mascot_face_layer, 8 * u, 6 * u, 1 * u, 1 * u, eye);
        break;

    case MASCOT_FACE_HAPPY:
    default:
        create_mascot_rect(mascot_face_layer, 5 * u, 2 * u, 2 * u, 2 * u, eye);
        create_mascot_rect(mascot_face_layer, 11 * u, 2 * u, 2 * u, 2 * u, eye);
        break;
    }
}

#if MASCOT_FACE_PREVIEW
static void update_mascot_face_preview(TickType_t now)
{
    static const MascotFace preview_faces[] = {
        MASCOT_FACE_HAPPY,
        MASCOT_FACE_BLINK,
        MASCOT_FACE_SLEEPY,
        MASCOT_FACE_CONCERNED,
        MASCOT_FACE_XX
    };
    static TickType_t last_change = 0;
    static uint8_t face_index = 0xFFU;

    if (face_index == 0xFFU ||
        (now - last_change) >= pdMS_TO_TICKS(1500U))
    {
        face_index = (face_index == 0xFFU) ? 0U : (uint8_t)((face_index + 1U) % (sizeof(preview_faces) / sizeof(preview_faces[0])));
        last_change = now;
        draw_mascot_face(preview_faces[face_index]);
        printf("Mascot face: %s\r\n", mascot_face_name(preview_faces[face_index]));
    }
}

static const char *mascot_face_name(MascotFace face)
{
    switch (face)
    {
    case MASCOT_FACE_BLINK:
        return "blink";
    case MASCOT_FACE_SLEEPY:
        return "sleepy";
    case MASCOT_FACE_CONCERNED:
        return "concerned";
    case MASCOT_FACE_XX:
        return "x.x";
    case MASCOT_FACE_HAPPY:
    default:
        return "happy";
    }
}
#endif

static void set_mascot_face(MascotFace face)
{
    if (mascot_face_valid != 0U && mascot_current_face == face)
    {
        return;
    }

    mascot_current_face = face;
    mascot_face_valid = 1U;
    draw_mascot_face(face);
}

static void update_mascot_behavior(uint32_t seconds, uint8_t bubble_visible, NetState net, ClawState claw, uint8_t alert_active)
{
    MascotFace face = MASCOT_FACE_HAPPY;
    uint8_t serious_alert;
    TickType_t now;

    serious_alert = (net == NET_STATE_OFFLINE || net == NET_STATE_SYNC_WARN || claw == CLAW_STATE_OFFLINE) ? 1U : 0U;
    now = xTaskGetTickCount();

    if (serious_alert != 0U)
    {
        face = MASCOT_FACE_XX;
    }
    else if (alert_active != 0U || claw == CLAW_STATE_WAITING)
    {
        face = MASCOT_FACE_CONCERNED;
    }
    else if (bubble_visible != 0U)
    {
        face = MASCOT_FACE_HAPPY;
    }
    else if ((seconds % 12U) == 5U)
    {
        face = MASCOT_FACE_BLINK;
    }
    else if (mascot_sleepy_time() != 0U)
    {
        face = MASCOT_FACE_SLEEPY;
    }

    set_mascot_face(face);
    set_mascot_wander(bubble_visible == 0U ? 1U : 0U, now);
}

static void set_mascot_wander(uint8_t enabled, TickType_t now)
{
    lv_anim_t anim;
    int16_t from_x;
    int16_t to_x;

    if (mascot_obj == NULL)
    {
        return;
    }

    if (enabled == 0U)
    {
        if (mascot_wander_active != 0U)
        {
            lv_anim_del(mascot_obj, (lv_anim_exec_xcb_t)lv_obj_set_x);
            mascot_wander_active = 0U;
        }
        lv_obj_set_x(mascot_obj, MASCOT_BASE_X);
        mascot_next_move_tick = 0;
        return;
    }

    if (mascot_wander_active != 0U)
    {
        return;
    }

    if (mascot_next_move_tick != 0 && (int32_t)(now - mascot_next_move_tick) < 0)
    {
        return;
    }

    from_x = (int16_t)lv_obj_get_x(mascot_obj);
    to_x = mascot_wander_dir == 0U ? MASCOT_WANDER_RIGHT_X : MASCOT_WANDER_LEFT_X;
    mascot_wander_dir = mascot_wander_dir == 0U ? 1U : 0U;

    lv_anim_del(mascot_obj, (lv_anim_exec_xcb_t)lv_obj_set_x);
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, mascot_obj);
    lv_anim_set_exec_cb(&anim, (lv_anim_exec_xcb_t)lv_obj_set_x);
    lv_anim_set_values(&anim, from_x, to_x);
    lv_anim_set_time(&anim, 4200);
    lv_anim_set_repeat_count(&anim, 1);
    lv_anim_set_path_cb(&anim, lv_anim_path_ease_in_out);
    lv_anim_set_ready_cb(&anim, mascot_wander_ready_cb);
    lv_anim_start(&anim);
    mascot_wander_active = 1U;
}

static void mascot_wander_ready_cb(lv_anim_t *anim)
{
    TickType_t now;
    uint32_t rest_ms;

    (void)anim;

    now = xTaskGetTickCount();
    rest_ms = 18000UL + (((uint32_t)now / configTICK_RATE_HZ) % 14UL) * 1000UL;
    mascot_wander_active = 0U;
    mascot_next_move_tick = now + pdMS_TO_TICKS(rest_ms);
}

static uint8_t mascot_sleepy_time(void)
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
        return 0U;
    }

    return (time.Hours >= 23U || time.Hours < 7U) ? 1U : 0U;
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
    NetState state;
    ClawState claw_snapshot;
    uint8_t alert_active;
    char claw_health[24];
    char claw_process[12];
    char claw_cron[16];
    char claw_disk[12];
    char claw_mem[12];
    char claw_error[24];
    char bubble[144];
    char message_text[96];
    char message_from[24];
    char message_timestamp[32];
    const char *detail_title = "ERROR";
    const char *detail_value;
    uint8_t detail_page;
    uint32_t message_age_seconds;
    uint8_t bubble_visible;
    uint8_t cron_warning;
    lv_color_t claw_color;

    taskENTER_CRITICAL();
    state = net_state;
    claw_snapshot = claw_state;
    alert_active = claw_alert_active;
    strncpy(claw_health, claw_health_text, sizeof(claw_health) - 1U);
    strncpy(claw_process, claw_process_text, sizeof(claw_process) - 1U);
    strncpy(claw_cron, claw_cron_text, sizeof(claw_cron) - 1U);
    strncpy(claw_disk, claw_disk_text, sizeof(claw_disk) - 1U);
    strncpy(claw_mem, claw_mem_text, sizeof(claw_mem) - 1U);
    strncpy(claw_error, claw_error_text, sizeof(claw_error) - 1U);
    cron_warning = claw_cron_warning;
    strncpy(bubble, speech_text, sizeof(bubble) - 1U);
    strncpy(message_text, cube_message_text, sizeof(message_text) - 1U);
    strncpy(message_from, cube_message_from, sizeof(message_from) - 1U);
    strncpy(message_timestamp, cube_message_timestamp, sizeof(message_timestamp) - 1U);
    taskEXIT_CRITICAL();
    claw_health[sizeof(claw_health) - 1U] = '\0';
    claw_process[sizeof(claw_process) - 1U] = '\0';
    claw_cron[sizeof(claw_cron) - 1U] = '\0';
    claw_disk[sizeof(claw_disk) - 1U] = '\0';
    claw_mem[sizeof(claw_mem) - 1U] = '\0';
    claw_error[sizeof(claw_error) - 1U] = '\0';
    bubble[sizeof(bubble) - 1U] = '\0';
    message_text[sizeof(message_text) - 1U] = '\0';
    message_from[sizeof(message_from) - 1U] = '\0';
    message_timestamp[sizeof(message_timestamp) - 1U] = '\0';
    claw_color = claw_state_color(claw_snapshot);
    detail_value = claw_error;

    if (alert_active == 0U && message_timestamp[0] != '\0' &&
        cube_message_is_expired(message_timestamp, &message_age_seconds) == 0U)
    {
        if (message_from[0] != '\0')
        {
            snprintf(bubble, sizeof(bubble), "%s:\n%s", message_from, message_text);
        }
        else
        {
            snprintf(bubble, sizeof(bubble), "%s", message_text);
        }
    }

    format_rtc_display(time_text, sizeof(time_text), date_text, sizeof(date_text));

    update_alert_pill(state, claw_snapshot, alert_active);
    lv_label_set_text(time_label, time_text);
    lv_label_set_text(date_label, date_text);
    lv_label_set_text(speech_label, bubble);
    bubble_visible = bubble[0] == '\0' ? 0U : 1U;
    if (bubble[0] == '\0')
    {
        lv_obj_add_flag(speech_bubble, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(speech_tail, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(speech_tail_tip, LV_OBJ_FLAG_HIDDEN);
    }
    else
    {
        lv_obj_clear_flag(speech_bubble, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(speech_tail, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(speech_tail_tip, LV_OBJ_FLAG_HIDDEN);
    }
    update_mascot_behavior(seconds, bubble_visible, state, claw_snapshot, alert_active);
    lv_label_set_text(cron_value_label, claw_cron);
    lv_label_set_text(health_value_label, claw_health);

    if (alert_active == 0U && strcmp(claw_error, "none") == 0)
    {
        detail_page = (uint8_t)((seconds / 2U) % 3U);
        if (detail_page == 0U)
        {
            detail_title = "PROC";
            detail_value = claw_process;
        }
        else if (detail_page == 1U)
        {
            detail_title = "DISK";
            detail_value = claw_disk;
        }
        else
        {
            detail_title = "MEM";
            detail_value = claw_mem;
        }
    }
    lv_label_set_text(error_title_label, detail_title);
    lv_label_set_text(error_value_label, detail_value);
    if (cron_warning != 0U)
    {
        update_metric_style(cron_card, cron_value_label,
                            lv_color_make(0x24, 0x1c, 0x0c),
                            lv_color_make(0xff, 0xd1, 0x66),
                            lv_color_make(0xff, 0xdc, 0x86));
    }
    else
    {
        update_metric_style(cron_card, cron_value_label,
                            lv_color_make(0x0b, 0x1a, 0x2a),
                            lv_color_make(0x42, 0x9d, 0xff),
                            lv_color_make(0x9d, 0xcb, 0xff));
    }

    if (alert_active != 0U)
    {
        update_metric_style(health_card, health_value_label,
                            lv_color_make(0x2a, 0x10, 0x13),
                            claw_color,
                            lv_color_make(0xff, 0x9a, 0x9a));
        update_metric_style(error_card, error_value_label,
                            lv_color_make(0x2a, 0x10, 0x13),
                            claw_color,
                            lv_color_make(0xff, 0x9a, 0x9a));
    }
    else
    {
        update_metric_style(health_card, health_value_label,
                            lv_color_make(0x0c, 0x21, 0x18),
                            lv_color_make(0x55, 0xe0, 0x82),
                            lv_color_make(0x90, 0xf2, 0xae));
        if (strcmp(detail_title, "PROC") == 0)
        {
            update_metric_style(error_card, error_value_label,
                                lv_color_make(0x1a, 0x12, 0x2a),
                                lv_color_make(0xb0, 0x8d, 0xff),
                                lv_color_make(0xcf, 0xbd, 0xff));
        }
        else if (strcmp(detail_title, "DISK") == 0)
        {
            update_metric_style(error_card, error_value_label,
                                lv_color_make(0x08, 0x1e, 0x24),
                                lv_color_make(0x4d, 0xd7, 0xee),
                                lv_color_make(0x9f, 0xec, 0xf7));
        }
        else if (strcmp(detail_title, "MEM") == 0)
        {
            update_metric_style(error_card, error_value_label,
                                lv_color_make(0x0d, 0x22, 0x1f),
                                lv_color_make(0x49, 0xd7, 0xb8),
                                lv_color_make(0x97, 0xeb, 0xd9));
        }
        else
        {
            update_metric_style(error_card, error_value_label,
                                lv_color_make(0x13, 0x16, 0x1d),
                                lv_color_make(0x34, 0x3f, 0x4d),
                                lv_color_make(0xb8, 0xc3, 0xcf));
        }
    }
}

static lv_obj_t *create_card(lv_obj_t *parent, int16_t x, int16_t y, int16_t w, int16_t h)
{
    lv_obj_t *card = lv_obj_create(parent);

    lv_obj_set_pos(card, x, y);
    lv_obj_set_size(card, w, h);
    lv_obj_set_style_radius(card, 6, 0);
    lv_obj_set_style_bg_color(card, lv_color_make(0x10, 0x16, 0x20), 0);
    lv_obj_set_style_bg_opa(card, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(card, 1, 0);
    lv_obj_set_style_border_color(card, lv_color_make(0x25, 0x31, 0x40), 0);
    lv_obj_set_style_shadow_width(card, 5, 0);
    lv_obj_set_style_shadow_color(card, lv_color_make(0x00, 0x00, 0x00), 0);
    lv_obj_set_style_shadow_opa(card, LV_OPA_30, 0);
    lv_obj_set_style_shadow_ofs_y(card, 2, 0);
    lv_obj_set_style_pad_all(card, 0, 0);
    return card;
}

static lv_obj_t *create_metric_card(lv_obj_t *parent, lv_obj_t **card, int16_t x, const char *title)
{
    lv_obj_t *title_label;
    lv_obj_t *value_label;

    *card = create_card(parent, x, 166, 68, 58);

    title_label = lv_label_create(*card);
    lv_obj_set_pos(title_label, 6, 4);
    lv_obj_set_size(title_label, 56, 14);
    lv_obj_set_style_text_font(title_label, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_color(title_label, lv_color_make(0x9a, 0xa7, 0xb6), 0);
    lv_obj_set_style_text_align(title_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_long_mode(title_label, LV_LABEL_LONG_CLIP);
    lv_label_set_text(title_label, title);
    if (strcmp(title, "ERROR") == 0)
    {
        error_title_label = title_label;
    }

    value_label = lv_label_create(*card);
    lv_obj_set_pos(value_label, 5, 22);
    lv_obj_set_size(value_label, 58, 30);
    lv_obj_set_style_text_font(value_label, &lv_font_montserrat_12, 0);
    lv_obj_set_style_text_align(value_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(value_label, lv_color_make(0xd9, 0xe3, 0xee), 0);
    lv_label_set_long_mode(value_label, LV_LABEL_LONG_WRAP);
    lv_label_set_text(value_label, "--");
    return value_label;
}

static void update_metric_style(lv_obj_t *card, lv_obj_t *value, lv_color_t bg, lv_color_t border, lv_color_t text)
{
    lv_obj_set_style_bg_color(card, bg, 0);
    lv_obj_set_style_border_color(card, border, 0);
    lv_obj_set_style_shadow_color(card, border, 0);
    lv_obj_set_style_shadow_opa(card, LV_OPA_20, 0);
    lv_obj_set_style_text_color(value, text, 0);
}

static void update_alert_pill(NetState net, ClawState claw, uint8_t alert_active)
{
    if (net == NET_STATE_OFFLINE || net == NET_STATE_SYNC_WARN)
    {
        lv_obj_clear_flag(status_pill, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_style_bg_color(status_pill, net_state_color(net), 0);
        lv_label_set_text(status_pill_label, net_state_text(net));
        return;
    }

    if (alert_active != 0U)
    {
        lv_obj_clear_flag(status_pill, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_style_bg_color(status_pill, claw_state_color(claw), 0);
        lv_label_set_text(status_pill_label, claw == CLAW_STATE_OFFLINE ? "OFF" : "WARN");
        return;
    }

    lv_obj_add_flag(status_pill, LV_OBJ_FLAG_HIDDEN);
}

static void monitor_task(void *arg)
{
    (void)arg;

    printf("Network/time task started\r\n");
    set_net_status(NET_STATE_WIFI_CONNECTING, "Connecting ESP8266 WiFi");
    set_claw_health(CLAW_STATE_WAITING, "WAITING", "Network");
    esp8266_sta_connect();
    set_net_status(NET_STATE_TIME_SYNCING, "WiFi connected, syncing time");

    TickType_t last_time_sync = 0;
    TickType_t last_health_check = 0;
    TickType_t last_status_check = 0;
    TickType_t last_message_check = 0;
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
            set_claw_health(CLAW_STATE_CHECKING, "CHECK", "Health...");
            health = ESP8266_GetOpenClawHealth();
            last_health_check = xTaskGetTickCount();
            if (health.ok)
            {
                set_claw_health(strcmp(health.status, "alive") == 0 ? CLAW_STATE_OK : CLAW_STATE_WARN,
                                strcmp(health.status, "alive") == 0 ? "ONLINE" : "HEALTH",
                                health.status);
            }
            else if (health.error_code == 2U)
            {
                set_claw_health(CLAW_STATE_WARN, "TLS", "Need proxy");
            }
            else if (health.error_code == 3U)
            {
                set_claw_health(CLAW_STATE_OFFLINE, "PROXY", "TCP fail");
            }
            else
            {
                set_claw_health(CLAW_STATE_OFFLINE, "OFFLINE", "Health fail");
            }
        }

        if (first_loop || (xTaskGetTickCount() - last_message_check) >= pdMS_TO_TICKS(30UL * 1000UL))
        {
            OpenClawMessageStruct message;
            message = ESP8266_GetOpenClawMessage();
            last_message_check = xTaskGetTickCount();
            if (message.ok)
            {
                set_cube_message(&message);
            }
        }

        if (first_loop || (xTaskGetTickCount() - last_status_check) >= pdMS_TO_TICKS(5UL * 60UL * 1000UL))
        {
            OpenClawStatusStruct status;
            set_claw_health(CLAW_STATE_CHECKING, "CHECK", "Status...");
            status = ESP8266_GetOpenClawStatus();
            last_status_check = xTaskGetTickCount();
            if (status.ok)
            {
                set_claw_metrics(&status);
            }
            else
            {
                set_claw_health(CLAW_STATE_WARN,
                                status.error_code == 2U ? "TLS" : (status.error_code == 3U ? "PROXY" : "WAIT"),
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
    if (state == NET_STATE_OFFLINE || state == NET_STATE_SYNC_WARN)
    {
        snprintf(speech_text, sizeof(speech_text), "Time sync\nfailed");
    }
    else if (claw_alert_active == 0U)
    {
        speech_text[0] = '\0';
    }
    taskEXIT_CRITICAL();
}

static void set_claw_health(ClawState state, const char *title, const char *health)
{
    ClawState effective_state;
    uint8_t is_alert;

    taskENTER_CRITICAL();
    effective_state = (state == CLAW_STATE_OK && claw_degraded != 0U) ? CLAW_STATE_WARN : state;
    is_alert = (state == CLAW_STATE_WARN || state == CLAW_STATE_OFFLINE) ? 1U : 0U;
    claw_state = effective_state;
    claw_alert_active = is_alert;
    snprintf(claw_state_text, sizeof(claw_state_text), "%s", title);
    if (state != CLAW_STATE_CHECKING || strcmp(claw_health_text, "Waiting") == 0)
    {
        snprintf(claw_health_text, sizeof(claw_health_text), "%s", health);
    }
    snprintf(speech_text, sizeof(speech_text), "%s",
             is_alert == 0U ? "" : (effective_state == CLAW_STATE_OFFLINE ? "I can't reach\nOpenClaw" : "Please check\nOpenClaw"));
    if (effective_state == CLAW_STATE_OFFLINE)
    {
        snprintf(claw_error_text, sizeof(claw_error_text), "%s", health);
    }
    else if (effective_state == CLAW_STATE_WARN && claw_degraded == 0U && strcmp(health, "alive") != 0)
    {
        snprintf(claw_error_text, sizeof(claw_error_text), "%s", health);
    }
    if (claw_metrics_valid == 0U)
    {
        snprintf(claw_cron_text, sizeof(claw_cron_text), "%s", state == CLAW_STATE_CHECKING ? "..." : "--");
        if (state == CLAW_STATE_OK)
        {
            snprintf(claw_error_text, sizeof(claw_error_text), "none");
        }
    }
    taskEXIT_CRITICAL();
}

static void set_claw_metrics(const OpenClawStatusStruct *status)
{
    ClawState state;

    if (status == NULL)
    {
        return;
    }

    state = status->gateway_ok == 0U ? CLAW_STATE_WARN : CLAW_STATE_OK;

    taskENTER_CRITICAL();
    claw_metrics_valid = 1U;
    claw_degraded = status->gateway_ok == 0U ? 1U : 0U;
    claw_cron_warning = status->cron_failed > 0U ? 1U : 0U;
    claw_alert_active = status->gateway_ok == 0U ? 1U : 0U;
    claw_state = state;
    snprintf(claw_state_text, sizeof(claw_state_text), "%s", state == CLAW_STATE_WARN ? "DEGRADED" : "ONLINE");
    if (status->gateway_ok == 0U)
    {
        snprintf(claw_health_text, sizeof(claw_health_text), "gateway down");
    }
    else if (strcmp(claw_health_text, "Waiting") == 0 || strcmp(claw_health_text, "Health...") == 0)
    {
        snprintf(claw_health_text, sizeof(claw_health_text), "alive");
    }
    snprintf(claw_cron_text, sizeof(claw_cron_text), "%u/%u", status->cron_ok, status->cron_total);
    snprintf(claw_process_text, sizeof(claw_process_text), "%u", status->process_count);
    snprintf(claw_disk_text, sizeof(claw_disk_text), "%u%%", status->disk_percent);
    snprintf(claw_mem_text, sizeof(claw_mem_text), "%u%%", status->memory_percent);
    if (status->gateway_ok == 0U)
    {
        snprintf(claw_error_text, sizeof(claw_error_text), "gateway");
    }
    else if (status->cron_failed > 0U)
    {
        snprintf(claw_error_text, sizeof(claw_error_text), "cron %u", status->cron_failed);
    }
    else
    {
        snprintf(claw_error_text, sizeof(claw_error_text), "none");
    }
    snprintf(speech_text, sizeof(speech_text), "%s",
             status->gateway_ok == 0U ? "I can't reach\nOpenClaw" : "");
    snprintf(claw_uptime_text, sizeof(claw_uptime_text), "%s", status->uptime[0] != '\0' ? status->uptime : "No data");
    taskEXIT_CRITICAL();
}

static void set_cube_message(const OpenClawMessageStruct *message)
{
    uint32_t age_seconds = 0U;
    uint8_t expired;

    if (message == NULL || message->ok == 0U || message->timestamp[0] == '\0')
    {
        return;
    }

    expired = cube_message_is_expired(message->timestamp, &age_seconds);
    printf("OpenClaw message %s%s%lum from %s: %s\r\n",
           expired != 0U ? "(expired) age=" : "(fresh) age=",
           age_seconds == 0U && expired != 0U ? "unknown/" : "",
           (unsigned long)(age_seconds / 60U),
           message->from[0] != '\0' ? message->from : "?",
           message->message);

    taskENTER_CRITICAL();
    if (expired != 0U)
    {
        cube_message_text[0] = '\0';
        cube_message_from[0] = '\0';
        cube_message_timestamp[0] = '\0';
    }
    else if (strcmp(cube_message_timestamp, message->timestamp) != 0)
    {
        snprintf(cube_message_text, sizeof(cube_message_text), "%s", message->message);
        snprintf(cube_message_from, sizeof(cube_message_from), "%s", message->from);
        snprintf(cube_message_timestamp, sizeof(cube_message_timestamp), "%s", message->timestamp);
    }
    else
    {
        snprintf(cube_message_text, sizeof(cube_message_text), "%s", message->message);
        snprintf(cube_message_from, sizeof(cube_message_from), "%s", message->from);
    }
    taskEXIT_CRITICAL();
}

static uint8_t cube_message_is_expired(const char *timestamp, uint32_t *age_seconds)
{
    int64_t now_seconds;
    int64_t message_seconds;
    int64_t age;

    if (age_seconds != NULL)
    {
        *age_seconds = 0U;
    }

    if (get_current_hkt_seconds(&now_seconds) == 0U ||
        parse_timestamp_hkt_seconds(timestamp, &message_seconds) == 0U)
    {
        return 1U;
    }

    age = now_seconds - message_seconds;
    if (age < 0)
    {
        age = 0;
    }

    if (age_seconds != NULL)
    {
        *age_seconds = age > 0xFFFFFFFFLL ? 0xFFFFFFFFUL : (uint32_t)age;
    }

    return age >= (int64_t)CUBE_MESSAGE_TTL_SECONDS ? 1U : 0U;
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

static uint8_t get_current_hkt_seconds(int64_t *out)
{
    RTC_TimeTypeDef time = {0};
    RTC_DateTypeDef date = {0};
    uint8_t valid;

    if (out == NULL)
    {
        return 0U;
    }

    taskENTER_CRITICAL();
    valid = rtc_time_valid;
    taskEXIT_CRITICAL();

    if (valid == 0U ||
        HAL_RTC_GetTime(&hrtc, &time, RTC_FORMAT_BIN) != HAL_OK ||
        HAL_RTC_GetDate(&hrtc, &date, RTC_FORMAT_BIN) != HAL_OK)
    {
        return 0U;
    }

    *out = date_time_to_seconds((uint16_t)(2000U + date.Year), date.Month, date.Date,
                                time.Hours, time.Minutes, time.Seconds);
    return 1U;
}

static uint8_t parse_timestamp_hkt_seconds(const char *timestamp, int64_t *out)
{
    int year;
    int month;
    int day;
    int hour;
    int minute;
    int second;
    char separator;
    const char *zone;
    int offset_sign = 1;
    int offset_hour = 0;
    int offset_minute = 0;
    int offset_present = 0;
    int64_t seconds;

    if (timestamp == NULL || out == NULL || strlen(timestamp) < 19U)
    {
        return 0U;
    }

    if (sscanf(timestamp, "%4d-%2d-%2d%c%2d:%2d:%2d",
               &year, &month, &day, &separator, &hour, &minute, &second) != 7)
    {
        return 0U;
    }

    if ((separator != 'T' && separator != ' ') ||
        year < 1970 || month < 1 || month > 12 || day < 1 || day > 31 ||
        hour < 0 || hour > 23 || minute < 0 || minute > 59 || second < 0 || second > 60)
    {
        return 0U;
    }

    zone = timestamp + 19;
    while (*zone != '\0' && *zone != 'Z' && *zone != '+' && *zone != '-')
    {
        zone++;
    }

    if (*zone == 'Z')
    {
        offset_present = 1;
        offset_hour = 0;
        offset_minute = 0;
    }
    else if (*zone == '+' || *zone == '-')
    {
        offset_present = 1;
        offset_sign = *zone == '-' ? -1 : 1;
        if (sscanf(zone + 1, "%2d:%2d", &offset_hour, &offset_minute) < 1 ||
            offset_hour < 0 || offset_hour > 23 || offset_minute < 0 || offset_minute > 59)
        {
            return 0U;
        }
    }

    seconds = date_time_to_seconds((uint16_t)year, (uint8_t)month, (uint8_t)day,
                                   (uint8_t)hour, (uint8_t)minute, (uint8_t)second);
    if (offset_present != 0)
    {
        seconds -= (int64_t)offset_sign * (int64_t)((offset_hour * 60 + offset_minute) * 60);
        seconds += HKT_OFFSET_SECONDS;
    }

    *out = seconds;
    return 1U;
}

static int64_t date_time_to_seconds(uint16_t year, uint8_t month, uint8_t day, uint8_t hour, uint8_t minute, uint8_t second)
{
    static const uint8_t days_in_month[] = {
        31U, 28U, 31U, 30U, 31U, 30U, 31U, 31U, 30U, 31U, 30U, 31U};
    int64_t days = 0;
    uint16_t y;
    uint8_t m;

    for (y = 1970U; y < year; y++)
    {
        days += is_leap_year_local(y) != 0U ? 366 : 365;
    }

    for (m = 1U; m < month; m++)
    {
        days += days_in_month[m - 1U];
        if (m == 2U && is_leap_year_local(year) != 0U)
        {
            days += 1;
        }
    }

    days += (int64_t)(day - 1U);
    return (((days * 24) + hour) * 60 + minute) * 60 + second;
}

static uint8_t is_leap_year_local(uint16_t year)
{
    return ((year % 4U == 0U && year % 100U != 0U) || year % 400U == 0U) ? 1U : 0U;
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
