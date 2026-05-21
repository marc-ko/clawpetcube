#ifndef __ESP8266_COMMON_H__
#define __ESP8266_COMMON_H__
#include "sys.h"

#define WIFI_SSID "HKBN-8266"
#define WIFI_PASSWORD "(marcoko)"

#define esp8266_PROCESS_RETURN_DATA 1
// Configuration parameters
extern const u8 *portnum;			 // Port number
extern const u8 *wifista_ssid;		 // WIFI STA SSID
extern const u8 *wifista_encryption; // WIFI STA Encryption method
extern const u8 *wifista_password;	 // WIFI STA Password

typedef struct
{
	u8 week;							 // what day of the week
	u16 year;							 // year
	u8 month, day, hour, minute, second; // self-explanatory
} TimeStruct;

typedef struct
{
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

typedef enum
{
	ACTION_NONE,
	ACTION_EAT,
	ACTION_WRITE,
	ACTION_SLEEP,
	ACTION_CLEAN
} ActionEnum;

typedef struct
{
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
