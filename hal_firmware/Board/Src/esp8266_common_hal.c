#include "esp8266_common_hal.h"
#include "usart_compat_hal.h"
#include "app_debug.h"
#include <stdio.h>
#include <string.h>

static const char *wifista_ssid = VPC_WIFI_SSID;
static const char *wifista_password = VPC_WIFI_PASSWORD;
static const char *time_host = "www.hko.gov.hk";
static const char *time_port = "80";

static uint8_t parse_http_date_hkt(const char *rx, TimeStruct *out);
static uint8_t parse_month(const char *month);
static uint8_t parse_weekday(const char *weekday);
static uint8_t days_in_month(uint16_t year, uint8_t month);
static uint8_t is_leap_year(uint16_t year);

void esp8266_at_response(u8 mode)
{
    if (USART2_RX_STA & 0x8000U) {
        USART2_RX_BUF[USART2_RX_STA & 0x7FFFU] = 0;
        printf("%s", USART2_RX_BUF);
        if (mode) {
            USART2_RX_STA = 0;
        }
    }
}

u8 *esp8266_check_cmd(u8 *str)
{
    if ((USART2_RX_STA & 0x8000U) == 0U) {
        return NULL;
    }
    USART2_RX_BUF[USART2_RX_STA & 0x7FFFU] = 0;
    return (u8 *)strstr((const char *)USART2_RX_BUF, (const char *)str);
}

u8 esp8266_send_cmd(u8 *cmd, u8 *ack, u16 waittime)
{
    USART2_RX_STA = 0;
    memset(USART2_RX_BUF, 0, USART2_MAX_RECV_LEN);
    printf("ESP8266 cmd: %s\r\n", cmd);
    u2_printf("%s\r\n", cmd);

    if (ack == NULL || waittime == 0U) {
        return 0;
    }

    while (waittime-- > 0U) {
        HAL_Delay(10);
        if ((USART2_RX_STA & 0x8000U) != 0U) {
            if (esp8266_check_cmd(ack) != NULL) {
                return 0;
            }
            printf("ESP8266 ack miss: %s\r\n", USART2_RX_BUF);
            USART2_RX_STA = 0;
        }
    }
    return 1;
}

u8 esp8266_send_data(u8 *data, u8 *ack, u16 waittime)
{
    USART2_RX_STA = 0;
    memset(USART2_RX_BUF, 0, USART2_MAX_RECV_LEN);
    u2_printf("%s", data);

    if (ack == NULL || waittime == 0U) {
        return 0;
    }

    while (waittime-- > 0U) {
        HAL_Delay(10);
        if ((USART2_RX_STA & 0x8000U) != 0U && esp8266_check_cmd(ack) != NULL) {
            return 0;
        }
    }
    return 1;
}

u8 esp8266_quit_trans(void)
{
    HAL_UART_Transmit(&huart2, (uint8_t *)"+++", 3, 100U);
    HAL_Delay(500);
    return esp8266_send_cmd((u8 *)"AT", (u8 *)"OK", 20);
}

u8 esp8266_consta_check(void)
{
    return esp8266_send_cmd((u8 *)"AT+CIPSTATUS", (u8 *)"STATUS", 50) == 0 ? 1 : 0;
}

void esp8266_ctr_gpio_init(void)
{
}

void esp8266_sta_connect(void)
{
    char command[160];

    while (esp8266_send_cmd((u8 *)"AT", (u8 *)"OK", 20)) {
        printf("No ESP8266-12F detected\r\n");
        HAL_Delay(800);
    }

    esp8266_send_cmd((u8 *)"ATE1", (u8 *)"OK", 20);
    esp8266_send_cmd((u8 *)"AT+CWMODE=1", (u8 *)"OK", 50);

    if (wifista_ssid[0] == '\0') {
        printf("WiFi credentials not configured; skipping join\r\n");
        return;
    }

    snprintf(command, sizeof(command), "AT+CWJAP_DEF=\"%s\",\"%s\"", wifista_ssid, wifista_password);
    while (esp8266_send_cmd((u8 *)command, (u8 *)"WIFI GOT IP", 1000)) {
        printf("ESP8266 WiFi join retry\r\n");
    }
    printf("ESP8266 WiFi connected\r\n");
}

TimeStruct esp8266_gettime(void)
{
    TimeStruct t = {0};
    char command[96];
    const char *request =
        "HEAD / HTTP/1.1\r\n"
        "Host: www.hko.gov.hk\r\n"
        "Connection: close\r\n"
        "\r\n";

    esp8266_send_cmd((u8 *)"AT+CIPMODE=0", (u8 *)"OK", 100);
    esp8266_send_cmd((u8 *)"AT+CIPCLOSE", NULL, 0);

    snprintf(command, sizeof(command), "AT+CIPSTART=\"TCP\",\"%s\",%s", time_host, time_port);
    if (esp8266_send_cmd((u8 *)command, (u8 *)"CONNECT", 500)) {
        printf("Time sync TCP connect failed\r\n");
        return t;
    }

    snprintf(command, sizeof(command), "AT+CIPSEND=%u", (unsigned int)strlen(request));
    if (esp8266_send_cmd((u8 *)command, (u8 *)">", 200)) {
        printf("Time sync CIPSEND failed\r\n");
        esp8266_send_cmd((u8 *)"AT+CIPCLOSE", NULL, 0);
        return t;
    }

    if (esp8266_send_data((u8 *)request, (u8 *)"Date:", 500)) {
        printf("Time sync Date header missing\r\n");
        esp8266_send_cmd((u8 *)"AT+CIPCLOSE", NULL, 0);
        return t;
    }

    if (parse_http_date_hkt((const char *)USART2_RX_BUF, &t) == 0U) {
        printf("Time sync parse failed\r\n");
        memset(&t, 0, sizeof(t));
    } else {
        printf("HKT time: %04u-%02u-%02u %02u:%02u:%02u week %u\r\n",
               t.year, t.month, t.day, t.hour, t.minute, t.second, t.week);
    }

    esp8266_send_cmd((u8 *)"AT+CIPCLOSE", NULL, 0);
    return t;
}

static uint8_t parse_http_date_hkt(const char *rx, TimeStruct *out)
{
    const char *date = strstr(rx, "Date:");
    char weekday[4] = {0};
    char month[4] = {0};
    unsigned int day = 0;
    unsigned int year = 0;
    unsigned int hour = 0;
    unsigned int minute = 0;
    unsigned int second = 0;
    uint8_t dim;

    if (date == NULL || out == NULL) {
        return 0;
    }

    if (sscanf(date, "Date: %3[^,], %u %3s %u %u:%u:%u GMT",
               weekday, &day, month, &year, &hour, &minute, &second) != 7) {
        return 0;
    }

    out->week = parse_weekday(weekday);
    out->month = parse_month(month);
    out->day = (u8)day;
    out->year = (u16)year;
    out->hour = (u8)hour;
    out->minute = (u8)minute;
    out->second = (u8)second;

    if (out->week == 0U || out->month == 0U || out->day == 0U ||
        out->hour > 23U || out->minute > 59U || out->second > 59U) {
        return 0;
    }

    out->hour = (u8)(out->hour + 8U);
    if (out->hour >= 24U) {
        out->hour = (u8)(out->hour - 24U);
        out->week++;
        if (out->week > 7U) {
            out->week = 1U;
        }

        out->day++;
        dim = days_in_month(out->year, out->month);
        if (out->day > dim) {
            out->day = 1U;
            out->month++;
            if (out->month > 12U) {
                out->month = 1U;
                out->year++;
            }
        }
    }

    return 1;
}

static uint8_t parse_month(const char *month)
{
    static const char names[12][4] = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };

    for (uint8_t i = 0; i < 12U; i++) {
        if (strncmp(month, names[i], 3U) == 0) {
            return (uint8_t)(i + 1U);
        }
    }
    return 0;
}

static uint8_t parse_weekday(const char *weekday)
{
    static const char names[7][4] = {
        "Mon", "Tue", "Wed", "Thu", "Fri", "Sat", "Sun"
    };

    for (uint8_t i = 0; i < 7U; i++) {
        if (strncmp(weekday, names[i], 3U) == 0) {
            return (uint8_t)(i + 1U);
        }
    }
    return 0;
}

static uint8_t days_in_month(uint16_t year, uint8_t month)
{
    static const uint8_t days[12] = {
        31U, 28U, 31U, 30U, 31U, 30U,
        31U, 31U, 30U, 31U, 30U, 31U
    };

    if (month == 2U && is_leap_year(year)) {
        return 29U;
    }
    if (month < 1U || month > 12U) {
        return 31U;
    }
    return days[month - 1U];
}

static uint8_t is_leap_year(uint16_t year)
{
    return (uint8_t)(((year % 4U) == 0U && (year % 100U) != 0U) || ((year % 400U) == 0U));
}

WeatherStruct ESP8266_GetWeather(char *city)
{
    WeatherStruct weather = {0};
    printf("Weather fetch pending for city: %s\r\n", city ? city : "");
    return weather;
}

UserInfoStruct ESP8266_GetUserInfo(char *username)
{
    UserInfoStruct user = {0};
    strncpy((char *)user.username, username ? username : "user", sizeof(user.username) - 1);
    user.coins = 100;
    user.hunger = 100;
    user.exp = 0;
    user.energy = 100;
    user.emotion = 100;
    user.status = 'i';
    user.action = ACTION_NONE;
    printf("User API pending; using local defaults for %s\r\n", user.username);
    return user;
}
