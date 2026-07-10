#ifndef ESP8266_COMMON_HAL_H
#define ESP8266_COMMON_HAL_H

#include "rtc_hal_compat.h"

typedef struct {
    u8 present_code;
    u8 present_temp;
    u8 today_highTemp;
    u8 today_lowTemp;
    u8 today_precipitation;
    u8 today_humidity;
    u8 tomorrow_highTemp;
    u8 tomorrow_lowTemp;
    u8 tomorrow_precipitation;
    u8 tomorrow_humidity;
} WeatherStruct;

typedef enum {
    ACTION_NONE,
    ACTION_EAT,
    ACTION_WRITE,
    ACTION_SLEEP,
    ACTION_CLEAN
} ActionEnum;

typedef struct {
    u8 username[12];
    u8 coins;
    u8 hunger;
    u8 exp;
    u8 energy;
    u8 emotion;
    u8 status;
    ActionEnum action;
} UserInfoStruct;

void esp8266_at_response(u8 mode);
u8 *esp8266_check_cmd(u8 *str);
u8 esp8266_send_cmd(u8 *cmd, u8 *ack, u16 waittime);
u8 esp8266_send_data(u8 *data, u8 *ack, u16 waittime);
u8 esp8266_quit_trans(void);
u8 esp8266_consta_check(void);
void esp8266_ctr_gpio_init(void);
void esp8266_sta_connect(void);
TimeStruct esp8266_gettime(void);
WeatherStruct ESP8266_GetWeather(char *city);
UserInfoStruct ESP8266_GetUserInfo(char *username);

#endif
