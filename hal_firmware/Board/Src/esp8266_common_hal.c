#include "esp8266_common_hal.h"
#include "usart_compat_hal.h"
#include "app_debug.h"
#include <stdio.h>
#include <string.h>

static const char *wifista_ssid = VPC_WIFI_SSID;
static const char *wifista_password = VPC_WIFI_PASSWORD;

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
    t.year = 2026;
    t.month = 1;
    t.day = 1;
    t.week = 4;
    t.hour = 0;
    t.minute = 0;
    t.second = 0;
    printf("Network time parser pending; using safe default time\r\n");
    return t;
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
