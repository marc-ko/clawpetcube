/*
 * Copyright 2022 NXP
 * SPDX-License-Identifier: MIT
 */

#include "lvgl.h"
#include <stdio.h>
#include "gui_guider.h"
#include "events_init.h"
#include "string.h"
#include "lcd.h"
// #include "flash.h"
// #include "systick.h"
#include "stdlib.h"
#include "esp8266_common.h"
#include "base_function.h"
// #include "dance_pic_data.h"
#include "weather_pic_data.h"

extern RTC_TimeTypeDef RTC_TimeStruct;
extern RTC_DateTypeDef RTC_DateStruct;
extern WeatherStruct esp8266_weather;
extern UserInfoStruct esp8266_userInfo;

extern u8 cityNum;
extern char cityNameArrPinYin[][10];
extern char cityNameArr[][10];
// lv_mem_monitor_t mem_mon;
// extern CityWifiInfo_t cityWifiInfo;
void setup_scr_deskop(lv_ui_deskop *ui)
{

	// Write codes screen
	ui->screen = lv_obj_create(NULL);
	initVPetArray();
	// Write style state: LV_STATE_DEFAULT for style_screen_main_main_default
	static lv_style_t style_screen_main_main_default;
	lv_style_init(&style_screen_main_main_default);
	lv_style_set_bg_color(&style_screen_main_main_default, lv_color_make(0x00, 0x00, 0x00));
	lv_style_set_bg_opa(&style_screen_main_main_default, 255);
	lv_obj_add_style(ui->screen, &style_screen_main_main_default, LV_PART_MAIN | LV_STATE_DEFAULT);

	// Write codes screen_weather
	ui->screen_weather = lv_img_create(ui->screen);
	lv_obj_set_pos(ui->screen_weather, 150, 40);
	lv_obj_set_size(ui->screen_weather, 48, 48);
	// lv_img_set_zoom(ui->screen_weather, 128);
	// Write style state: LV_STATE_DEFAULT for style_screen_weather_main_main_default
	static lv_style_t style_screen_weather_main_main_default;
	lv_style_init(&style_screen_weather_main_main_default);
	lv_style_set_img_recolor(&style_screen_weather_main_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_img_recolor_opa(&style_screen_weather_main_main_default, 0);
	lv_style_set_img_opa(&style_screen_weather_main_main_default, 255);
	lv_obj_add_style(ui->screen_weather, &style_screen_weather_main_main_default, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_add_flag(ui->screen_weather, LV_OBJ_FLAG_CLICKABLE);
	lv_img_set_src(ui->screen_weather, &weatherPatternDefault);
	updateDeskopWeatherPattern(esp8266_weather.present_code);
	lv_img_set_pivot(ui->screen_weather, 0, 0);
	lv_img_set_angle(ui->screen_weather, 0);

	ui->screen_coins = lv_label_create(ui->screen);
	char coins_str[20];
	sprintf(coins_str, "$$: %d", esp8266_userInfo.coins);
	lv_label_set_text(ui->screen_coins, coins_str);
	lv_obj_set_pos(ui->screen_coins, 130, 143);
	lv_obj_set_size(ui->screen_coins, 130, 17);
	lv_label_set_long_mode(ui->screen_coins, LV_LABEL_LONG_WRAP);
	lv_obj_set_style_text_align(ui->screen_coins, LV_TEXT_ALIGN_LEFT, 0);

	ui->screen_hunger = lv_label_create(ui->screen);
	char hunger_str[20];
	sprintf(hunger_str, "Hunger: %d", esp8266_userInfo.hunger);
	lv_label_set_text(ui->screen_hunger, hunger_str);
	lv_obj_set_pos(ui->screen_hunger, 130, 164);
	lv_obj_set_size(ui->screen_hunger, 130, 17);
	lv_label_set_long_mode(ui->screen_hunger, LV_LABEL_LONG_WRAP);
	lv_obj_set_style_text_align(ui->screen_hunger, LV_TEXT_ALIGN_LEFT, 0);

	ui->screen_exp = lv_label_create(ui->screen);
	char exp_str[20];
	sprintf(exp_str, "XP: %d", esp8266_userInfo.exp);
	lv_label_set_text(ui->screen_exp, exp_str);
	lv_obj_set_pos(ui->screen_exp, 130, 184);
	lv_obj_set_size(ui->screen_exp, 130, 17);
	lv_label_set_long_mode(ui->screen_exp, LV_LABEL_LONG_WRAP);
	lv_obj_set_style_text_align(ui->screen_exp, LV_TEXT_ALIGN_LEFT, 0);

	ui->screen_energy = lv_label_create(ui->screen);
	char energy_str[20];
	sprintf(energy_str, "Energy: %d", esp8266_userInfo.energy);
	lv_label_set_text(ui->screen_energy, energy_str);
	lv_obj_set_pos(ui->screen_energy, 130, 204);
	lv_obj_set_size(ui->screen_energy, 130, 17);
	lv_label_set_long_mode(ui->screen_energy, LV_LABEL_LONG_WRAP);
	lv_obj_set_style_text_align(ui->screen_energy, LV_TEXT_ALIGN_LEFT, 0);

	// Write codes screen_gif
	ui->screen_gif = lv_img_create(ui->screen);
	lv_obj_set_pos(ui->screen_gif, 50, 40);
	lv_obj_set_size(ui->screen_gif, LV_SIZE_CONTENT, LV_SIZE_CONTENT);

	ui->screen_hour = lv_label_create(ui->screen);
	lv_obj_set_pos(ui->screen_hour, 0, 20);
	lv_obj_set_size(ui->screen_hour, 30, 20);
	char tempStr[15];
	numToString(RTC_TimeStruct.RTC_Hours, tempStr);
	lv_label_set_text(ui->screen_hour, tempStr);
	lv_label_set_long_mode(ui->screen_hour, LV_LABEL_LONG_WRAP);
	lv_obj_set_style_text_align(ui->screen_hour, LV_TEXT_ALIGN_CENTER, 0);

	// Write style state: LV_STATE_DEFAULT for style_screen_hour_main_main_default
	static lv_style_t style_screen_hour_main_main_default;
	lv_style_init(&style_screen_hour_main_main_default);
	lv_style_set_radius(&style_screen_hour_main_main_default, 0);
	lv_style_set_bg_color(&style_screen_hour_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_color(&style_screen_hour_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_screen_hour_main_main_default, LV_GRAD_DIR_VER);
	lv_style_set_bg_opa(&style_screen_hour_main_main_default, 0);
	lv_style_set_text_color(&style_screen_hour_main_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_text_font(&style_screen_hour_main_main_default, &lv_font_montserratMedium_20);
	lv_style_set_text_letter_space(&style_screen_hour_main_main_default, 2);
	lv_style_set_pad_left(&style_screen_hour_main_main_default, 0);
	lv_style_set_pad_right(&style_screen_hour_main_main_default, 0);
	lv_style_set_pad_top(&style_screen_hour_main_main_default, 0);
	lv_style_set_pad_bottom(&style_screen_hour_main_main_default, 0);
	lv_obj_add_style(ui->screen_hour, &style_screen_hour_main_main_default, LV_PART_MAIN | LV_STATE_DEFAULT);

	// Write codes screen_min
	ui->screen_min = lv_label_create(ui->screen);
	lv_obj_set_pos(ui->screen_min, 30, 20);
	lv_obj_set_size(ui->screen_min, 30, 20);
	numToString(RTC_TimeStruct.RTC_Minutes, tempStr);
	lv_label_set_text(ui->screen_min, tempStr);
	lv_label_set_long_mode(ui->screen_min, LV_LABEL_LONG_WRAP);
	lv_obj_set_style_text_align(ui->screen_min, LV_TEXT_ALIGN_CENTER, 0);

	// Write style state: LV_STATE_DEFAULT for style_screen_min_main_main_default
	static lv_style_t style_screen_min_main_main_default;
	lv_style_init(&style_screen_min_main_main_default);
	lv_style_set_radius(&style_screen_min_main_main_default, 0);
	lv_style_set_bg_color(&style_screen_min_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_color(&style_screen_min_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_screen_min_main_main_default, LV_GRAD_DIR_VER);
	lv_style_set_bg_opa(&style_screen_min_main_main_default, 0);
	lv_style_set_text_color(&style_screen_min_main_main_default, lv_color_make(0xff, 0xd5, 0x00));
	lv_style_set_text_font(&style_screen_min_main_main_default, &lv_font_montserratMedium_20);
	lv_style_set_text_letter_space(&style_screen_min_main_main_default, 2);
	lv_style_set_pad_left(&style_screen_min_main_main_default, 0);
	lv_style_set_pad_right(&style_screen_min_main_main_default, 0);
	lv_style_set_pad_top(&style_screen_min_main_main_default, 0);
	lv_style_set_pad_bottom(&style_screen_min_main_main_default, 0);
	lv_obj_add_style(ui->screen_min, &style_screen_min_main_main_default, LV_PART_MAIN | LV_STATE_DEFAULT);

	// Write codes screen_temperature
	ui->screen_temperature = lv_label_create(ui->screen);
	lv_obj_set_pos(ui->screen_temperature, 185, 20);
	lv_obj_set_size(ui->screen_temperature, 55, 20);
	updateDeskopTemperature(esp8266_weather.present_temp);
	lv_label_set_long_mode(ui->screen_temperature, LV_LABEL_LONG_WRAP);
	lv_obj_set_style_text_align(ui->screen_temperature, LV_TEXT_ALIGN_CENTER, 0);

	// Write style state: LV_STATE_DEFAULT for style_screen_temperature_main_main_default
	static lv_style_t style_screen_temperature_main_main_default;
	lv_style_init(&style_screen_temperature_main_main_default);
	lv_style_set_radius(&style_screen_temperature_main_main_default, 0);
	lv_style_set_bg_color(&style_screen_temperature_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_color(&style_screen_temperature_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_screen_temperature_main_main_default, LV_GRAD_DIR_VER);
	lv_style_set_bg_opa(&style_screen_temperature_main_main_default, 0);
	lv_style_set_text_color(&style_screen_temperature_main_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_text_font(&style_screen_temperature_main_main_default, &lv_font_simsun_20);
	lv_style_set_text_letter_space(&style_screen_temperature_main_main_default, 2);
	lv_style_set_pad_left(&style_screen_temperature_main_main_default, 0);
	lv_style_set_pad_right(&style_screen_temperature_main_main_default, 0);
	lv_style_set_pad_top(&style_screen_temperature_main_main_default, 0);
	lv_style_set_pad_bottom(&style_screen_temperature_main_main_default, 0);
	lv_obj_add_style(ui->screen_temperature, &style_screen_temperature_main_main_default, LV_PART_MAIN | LV_STATE_DEFAULT);

	// Write codes screen_user_info

	static lv_style_t style_screen_coins_main_main_default;
	lv_style_init(&style_screen_coins_main_main_default);
	lv_style_set_radius(&style_screen_coins_main_main_default, 0);
	lv_style_set_bg_color(&style_screen_coins_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_color(&style_screen_coins_main_main_default, lv_color_make(0x21, 0x95, 0xf6));
	lv_style_set_bg_grad_dir(&style_screen_coins_main_main_default, LV_GRAD_DIR_VER);
	lv_style_set_bg_opa(&style_screen_coins_main_main_default, 0);
	lv_style_set_text_color(&style_screen_coins_main_main_default, lv_color_make(0xff, 0xff, 0xff));
	lv_style_set_text_font(&style_screen_coins_main_main_default, &lv_font_montserrat_14);
	lv_style_set_text_letter_space(&style_screen_coins_main_main_default, 2);
	lv_style_set_pad_left(&style_screen_coins_main_main_default, 0);
	lv_style_set_pad_right(&style_screen_coins_main_main_default, 0);
	lv_style_set_pad_top(&style_screen_coins_main_main_default, 0);
	lv_style_set_pad_bottom(&style_screen_coins_main_main_default, 0);

	lv_obj_add_style(ui->screen_coins, &style_screen_coins_main_main_default, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_add_style(ui->screen_hunger, &style_screen_coins_main_main_default, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_add_style(ui->screen_exp, &style_screen_coins_main_main_default, LV_PART_MAIN | LV_STATE_DEFAULT);
	lv_obj_add_style(ui->screen_energy, &style_screen_coins_main_main_default, LV_PART_MAIN | LV_STATE_DEFAULT);
}

extern lv_ui_deskop lv_gui_deskop;
void updateDeskopTime(u8 hour, u8 minute, u8 sec)
{
	char hourStr[10];
	char mintuesStr[10];
	// char secondStr[10];
	char temp[10];

	memset(hourStr, 0, sizeof(hourStr));
	numToString(hour, temp);
	strcat(hourStr, temp);
	lv_label_set_text(lv_gui_deskop.screen_hour, hourStr);

	memset(mintuesStr, 0, sizeof(mintuesStr));
	numToString(minute, temp);
	strcat(mintuesStr, temp);
	lv_label_set_text(lv_gui_deskop.screen_min, mintuesStr);

	// memset(secondStr, 0, sizeof(secondStr));
	// numToString(sec, temp);
	// strcat(secondStr, temp);
	// lv_label_set_text(lv_gui_deskop.screen_second, secondStr);
}

void updateDeskopDate(u8 month, u8 day, u8 week)
{
	char dateStr[15];
	char weekStr[15];
	char temp[10];

	memset(dateStr, 0, sizeof(dateStr));
	numToString(month, temp);
	strcat(dateStr, temp);
	strcat(dateStr, "Getsu");
	numToString(day, temp);
	strcat(dateStr, temp);
	strcat(dateStr, "Hi");
	lv_label_set_text(lv_gui_deskop.screen_date, dateStr);

	memset(weekStr, 0, sizeof(weekStr));
	// char weekFullname[8][3] = {" ", "Mon", "Tue", "Wed", "Thur", "Fri", "Sat", "Sun"};
	// strcat(weekStr, weekFullname[week]);
	lv_label_set_text(lv_gui_deskop.screen_week, weekStr);
}

void updateDeskopCity(u8 cityNum)
{
	char cityNameArr[][10] = {"Namking", "Wuxing", "Hong Kong", "HK", "Beijing", "Shanghai", "Canton"};
	lv_label_set_text(lv_gui_deskop.screen_city, cityNameArr[cityNum]);
}

void updateDeskopWeatherText(u8 weatherCode)
{
	u8 weatherArrPos = 0;
	char weatherArr[][15] = {"Sunny", "Cloudy", "Cloud", "Ame", "Snowy", "Dusty", "Foggy", "Windy", "Typhoon", "Cold", "Hot"};
	char str[25];
	memset(str, 0, sizeof(str));
	if (weatherCode <= 3)
		weatherArrPos = 0;
	else if (weatherCode >= 4 && weatherCode <= 8)
		weatherArrPos = 1;
	else if (weatherCode == 9)
		weatherArrPos = 2;
	else if (weatherCode >= 10 && weatherCode <= 19)
		weatherArrPos = 3;
	else if (weatherCode >= 20 && weatherCode <= 25)
		weatherArrPos = 4;
	else if (weatherCode >= 26 && weatherCode <= 29)
		weatherArrPos = 5;
	else if (weatherCode == 30 || weatherCode == 31)
		weatherArrPos = 6;
	else if (weatherCode == 32 || weatherCode == 33)
		weatherArrPos = 7;
	else if (weatherCode >= 34 && weatherCode <= 36)
		weatherArrPos = 8;
	else if (weatherCode == 37)
		weatherArrPos = 9;
	else
		weatherArrPos = 10;
	strcat(str, "Weather:");
	strcat(str, weatherArr[weatherArrPos]);
	lv_label_set_text(lv_gui_deskop.screen_weather_text, str);
}

void updateDeskopTemperature(int temperature)
{
	char str[25];
	memset(str, 0, sizeof(str));
	char temp[10];
	numToString(temperature, temp);
	strcat(str, temp);
	strcat(str, "^C");
	lv_label_set_text(lv_gui_deskop.screen_temperature, str);
}

void updateDesktopDialogue(char *output)
{
	char str[150];
	memset(str, 0, sizeof(str));

	strcat(str, output);
	// lv_label_set_text(lv_gui_deskop.screen_dialogue, str);
}

void updateDeskopWeatherInfo(int temperatureHigh, int temperatureLow, u8 precipitation, u8 humidityPercent)
{
	char str[150];
	memset(str, 0, sizeof(str));
	char temp[10];
	strcat(str, "Highest Temp:");
	numToString(temperatureHigh, temp);
	strcat(str, temp);
	strcat(str, "*C, Lowest Temp:");
	numToString(temperatureLow, temp);
	strcat(str, temp);
	strcat(str, "*C, FuMi:");
	numToString(precipitation, temp);
	strcat(str, temp);
	strcat(str, "mm, Humidity");
	numToString(humidityPercent, temp);
	strcat(str, temp);
	strcat(str, "%");
	lv_label_set_text(lv_gui_deskop.screen_weather_info, str);
}

void updateDesktopUserInfo(u8 coins, u8 hunger, u8 exp, u8 energy)
{
	char str1[20];
	char str2[20];
	char str3[20];
	char str4[20];
	lv_obj_add_flag(lv_gui_deskop.screen_coins,LV_OBJ_FLAG_HIDDEN);
	lv_obj_add_flag(lv_gui_deskop.screen_hunger,LV_OBJ_FLAG_HIDDEN);
	lv_obj_add_flag(lv_gui_deskop.screen_exp,LV_OBJ_FLAG_HIDDEN);
	lv_obj_add_flag(lv_gui_deskop.screen_energy,LV_OBJ_FLAG_HIDDEN);
	sprintf(str1, "$$: %d", coins);
	lv_label_set_text(lv_gui_deskop.screen_coins, str1);

	sprintf(str2, "Hunger: %d", hunger);
	lv_label_set_text(lv_gui_deskop.screen_hunger, str2);

	sprintf(str3, "XP: %d", exp);
	lv_label_set_text(lv_gui_deskop.screen_exp, str3);

	sprintf(str4, "Energy: %d", energy);
	lv_label_set_text(lv_gui_deskop.screen_energy, str4);
	
	
}

extern u8 weatherPatternBmp[6912];
void updateDeskopWeatherPattern(u8 weatherCode)
{
	if (weatherCode >= WEATHER_PIC_NUM)
	{
		return;
	}

	u16 i = 0;
	for (i = 0; i < 6912; i++)
	{
		weatherPatternBmp[i] = weather_pic_Bmp[weatherCode][i];
	}
}

// extern uint8_t dancePatternBmp[12690];
const int vpetdefaultsize = 13;
const int vpetsleepdownsize = 4;
const int vpetsleepingsize = 6;
const int vpetsleepupsize = 7;
const int vpetsickarraysize = 20;
const int vpetwalkleftarraysize = 14;
const int vpetwalkrightkarraysize = 14;

#define vpetdefaultsize 13
#define vpetsleepdownsize 4
#define vpetsleepingsize 6
#define vpetsleepupsize 7
#define vpetsickarraysize 20
#define vpetwalkleftarraysize 14
#define vpetwalkrightkarraysize 14
char vpetDefaultsizeArray[vpetdefaultsize][30];
char vpetDefaultArray[vpetdefaultsize][30];
char vpetDrinkingArray[20][30];
char vpetEatingArray[19][30];
char vpetSleepdownArray[vpetsleepdownsize][30];
char vpetSleepingArray[vpetsleepingsize][30];
char vpetSleepupArray[vpetsleepupsize][30];
char vpetPlayingArray[20][30];
char vpetWalkleftArray[vpetwalkleftarraysize][30];
char vpetWalkrightArray[vpetwalkrightkarraysize][30];
char vpetSickArray[vpetsickarraysize][30];
char vpetDefault72Array[vpetdefaultsize][30];
char vpetDefault64Array[vpetdefaultsize][30];
char vpetDefault56Array[vpetdefaultsize][30];
char vpetDefault48Array[vpetdefaultsize][30];
char vpetDefault40Array[vpetdefaultsize][30];
char writingArray[75][30];
char patArray[17][30];
char bodyArray[22][30];
char liftArray[35][30];
char glassArray[41][30];
void initVPetArray()
{
	for (int i = 0; i < vpetdefaultsize; i++)
	{
		// snprinft to avoid overflow
		snprintf(vpetDefaultArray[i], sizeof(vpetDefaultArray[i]), "0:default/vpet%02d.bin", i + 1);
		snprintf(vpetDefault72Array[i], sizeof(vpetDefault72Array[i]), "0:default_72/vpet%02d.bin", i + 1);
		snprintf(vpetDefault64Array[i], sizeof(vpetDefault64Array[i]), "0:default_64/vpet%02d.bin", i + 1);
		snprintf(vpetDefault56Array[i], sizeof(vpetDefault56Array[i]), "0:default_56/vpet%02d.bin", i + 1);
		snprintf(vpetDefault48Array[i], sizeof(vpetDefault48Array[i]), "0:default_48/vpet%02d.bin", i + 1);
		snprintf(vpetDefault40Array[i], sizeof(vpetDefault40Array[i]), "0:default_40/vpet%02d.bin", i + 1);
		
	}
	for (int i = 0; i < vpetsickarraysize; i++)
	{

		snprintf(vpetSickArray[i], sizeof(vpetSickArray[i]), "0:ill/vpet%02d.bin", i + 1);
	}
	for (int i = 0; i < vpetsleepdownsize; i++)
	{
		snprintf(vpetSleepdownArray[i], sizeof(vpetSleepdownArray[i]), "0:sleepdown/vpet%02d.bin", i + 1);
	}
	for (int i = 0; i < vpetsleepingsize; i++)
	{
		snprintf(vpetSleepingArray[i], sizeof(vpetSleepingArray[i]), "0:sleeping/vpet%02d.bin", i + 1);
	}
	for (int i = 0; i < vpetsleepupsize; i++)
	{
		snprintf(vpetSleepupArray[i], sizeof(vpetSleepupArray[i]), "0:sleepup/vpet%02d.bin", i + 1);
	}
	for(int i = 0; i < 74; i++){
		snprintf(writingArray[i], sizeof(writingArray[i]), "0:writing/vpet%02d.bin", i + 1);
	}
	for (int i = 0; i < 20; i++)
	{
		snprintf(patArray[i], sizeof(patArray[i]), "0:pat/vpet%02d.bin", i + 1);
	}
	for(int i = 0; i < vpetwalkleftarraysize; i++){
		snprintf(vpetWalkleftArray[i], sizeof(vpetWalkleftArray[i]), "0:walkleft/vpet%02d.bin", i + 1);
	}
	for(int i = 0; i < vpetwalkrightkarraysize; i++){
		snprintf(vpetWalkrightArray[i],sizeof(vpetWalkrightArray[i]),"0:walkr/vpet%02d.bin", i + 1);
	}
	for(int i = 0; i <22; i++){
		snprintf(bodyArray[i],sizeof(bodyArray[i]),"0:body/vpet%02d.bin", i + 1);
	}
	for(int i = 0; i <35; i++){
		snprintf(liftArray[i],sizeof(liftArray[i]),"0:lift/vpet%02d.bin", i + 1);
	}
	for(int i = 0; i<19; i++){
		snprintf(vpetEatingArray[i],sizeof(vpetEatingArray[i]),"0:eat/vpet%02d.bin", i + 1);
	}
	for(int i = 0; i<41; i++){
		snprintf(glassArray[i],sizeof(glassArray[i]),"0:glass/vpet%02d.bin", i + 1);
	}
	
}

int beforeArrayNum = 0;
u8 display_finished = 1;
int xpos = 30;
int ypos = 40;
u8 sleeping = 0;
u8 writing = 0;
u8 eatfinished = 0;
u8 cleaning = 0;
extern UserInfoStruct esp8266_userInfo;
int updateVPetPattern(u8 *num, u8 arraynum)
{

	if (beforeArrayNum != arraynum)
	{
		if (display_finished)
		{
			beforeArrayNum = arraynum;
			display_finished = 0;
		}
	}
	if(esp8266_userInfo.action == ACTION_SLEEP){
		if(!sleeping){
			sleeping = 1;
			beforeArrayNum = 1;
		}
		else{
			beforeArrayNum = 2;
		}
	}
	else if(esp8266_userInfo.action == ACTION_WRITE){
		if(!writing){
			writing = 1;
			beforeArrayNum = 4;
		}
		else{
			beforeArrayNum = 4;
			if(*num >= 50){
				*num = 15;
			}
		}
	}
	else if(esp8266_userInfo.action == ACTION_EAT){
		if(!eatfinished){
		beforeArrayNum = 10;
		}
		if(*num == 19){
			eatfinished = 1;
		}
	}
	else if(esp8266_userInfo.action == ACTION_CLEAN){
		if(!cleaning){
			cleaning = 1;
			beforeArrayNum = 11;
		}
		else{
			beforeArrayNum = 11;
			if(*num >= 30){
				*num = 10;
			}
		}
	}
	else if(esp8266_userInfo.action == ACTION_NONE){
		if(sleeping){
			beforeArrayNum = 3;
			sleeping = 0;
		}
		else if(writing){
			beforeArrayNum = 4;
			writing = 0;
		}
		else if(eatfinished){
			beforeArrayNum = 0;
			eatfinished = 0;
		}
		
	}

	switch(beforeArrayNum){
		
		case 0:
				if (*num < 13)
				{	
					lv_img_cache_invalidate_src(vpetDefaultArray[*num - 1]);
					lv_img_set_src(lv_gui_deskop.screen_gif, vpetDefaultArray[*num]);
					
					//printf("lv_img_set_src success! File: %s\r\n", vpetDefaultsizeArray[*num]);
					// lv_mem_monitor(&mem_mon);
					// printf("used: %d, total: %d, frag: %d\r\n", mem_mon.total_size - mem_mon.free_size, mem_mon.total_size, mem_mon.frag_pct);
					display_finished = 1;
					return 2;
				}
					else if(*num == 13)
				{
					display_finished = 1;
					return 1;
				}
				break;
		case 1:
				if(*num < vpetsleepdownsize){
					lv_img_cache_invalidate_src(vpetSleepdownArray[*num - 1]);
					lv_img_set_src(lv_gui_deskop.screen_gif, vpetSleepdownArray[*num]);
					printf("lv_img_set_src success! File: %s\r\n", vpetSleepdownArray[*num]);
					display_finished = 0;
					return 0;
				}
				else if(*num == vpetsleepdownsize){
					display_finished = 1;
					return 1;
				}
				break;
		case 2:
				if(*num < vpetsleepingsize){
					lv_img_cache_invalidate_src(vpetSleepingArray[*num - 1]);
					lv_img_set_src(lv_gui_deskop.screen_gif, vpetSleepingArray[*num]);
					printf("lv_img_set_src success! File: %s\r\n", vpetSleepingArray[*num]);
					display_finished = 0;
					return 0;
				}
				else if(*num == vpetsleepingsize){
					display_finished = 1;
					return 1;
				}
				break;
		case 3:	
				if(*num < vpetsleepupsize){
					lv_img_cache_invalidate_src(vpetSleepupArray[*num - 1]);
					lv_img_set_src(lv_gui_deskop.screen_gif, vpetSleepupArray[*num]);
					printf("lv_img_set_src success! File: %s\r\n", vpetSleepupArray[*num]);
					display_finished = 0;
					return 0;
				}
				else if(*num == vpetsleepupsize){
					display_finished = 1;
					return 1;
				}
				break;
		case 4:
		 			if(*num < 70){
					lv_img_cache_invalidate_src(writingArray[*num - 1]);
					lv_img_set_src(lv_gui_deskop.screen_gif, writingArray[*num]);
					printf("lv_img_set_src success! File: %s\r\n", writingArray[*num]);
					display_finished = 0;
					return 0;
					}else if(*num == 70){
						display_finished = 1;
						return 1;
					}
		 			break;
		case 5:
				if(*num < 17){
					lv_img_cache_invalidate_src(patArray[*num - 1]);
					lv_img_set_src(lv_gui_deskop.screen_gif, patArray[*num]);
					printf("lv_img_set_src success! File: %s\r\n", patArray[*num]);
					display_finished = 0;
					return 0;
				}
				else if(*num == 17){
					display_finished = 1;
					return 1;
				}
				break;
	
		case 6:
				if(*num < vpetwalkleftarraysize){
					lv_obj_set_x(lv_gui_deskop.screen_gif, xpos-=2);
					lv_img_cache_invalidate_src(vpetWalkleftArray[*num - 1]);
					lv_img_set_src(lv_gui_deskop.screen_gif, vpetWalkleftArray[*num]);
					printf("lv_img_set_src success! File: %s\r\n", vpetWalkleftArray[*num]);
					display_finished = 0;
					return 0;
				}
				else if(*num == vpetwalkleftarraysize){
					display_finished = 1;
					return 1;
				}
		case 7:
				if(*num < vpetwalkrightkarraysize){
					lv_obj_set_x(lv_gui_deskop.screen_gif, xpos+=2);
					lv_img_cache_invalidate_src(vpetWalkrightArray[*num - 1]);
					lv_img_set_src(lv_gui_deskop.screen_gif, vpetWalkrightArray[*num]);
					printf("lv_img_set_src success! File: %d\r\n",lv_gui_deskop.screen_gif->h_layout );
					printf("lv_img_set_src success! File: %s\r\n", vpetWalkrightArray[*num]);
					display_finished = 0;
					return 0;
				}
				else if(*num == vpetwalkrightkarraysize){
					display_finished = 1;
					return 1;

				}
		case 8:
				if(*num < 22){
					lv_img_cache_invalidate_src(bodyArray[*num - 1]);
					lv_img_set_src(lv_gui_deskop.screen_gif, bodyArray[*num]);
					printf("lv_img_set_src success! File: %s\r\n", bodyArray[*num]);
					display_finished = 0;
					return 0;
				}
				else if(*num == 22){
					display_finished = 1;
					return 1;
				}
		case 9:
				if(*num == 0){
					lv_obj_set_y(lv_gui_deskop.screen_gif,  200 );
				}
				if(*num < 20){
					lv_obj_set_y(lv_gui_deskop.screen_gif,  ypos += 3 );

					lv_img_cache_invalidate_src( liftArray[*num - 1] );
					lv_img_set_src(lv_gui_deskop.screen_gif, liftArray[*num]);
					printf("lv_img_set_src success! File: %s\r\n", liftArray[*num]);
					display_finished = 0;
					return 0;
				}
				else if(*num <35){
					lv_obj_set_y(lv_gui_deskop.screen_gif, ypos-=3);
					lv_img_cache_invalidate_src( liftArray[*num - 1] );
					lv_img_set_src(lv_gui_deskop.screen_gif, liftArray[*num]);
					printf("lv_img_set_src success! File: %s\r\n", liftArray[*num]);
					display_finished = 0;
					return 0;

				}
				else if(*num == 35){
					lv_obj_set_y(lv_gui_deskop.screen_gif, 40);
					display_finished = 1;
					return 1;
				}
				break;
		case 10:
				if(*num < 19){
					lv_img_cache_invalidate_src(vpetEatingArray[*num - 1]);
					lv_img_set_src(lv_gui_deskop.screen_gif, vpetEatingArray[*num]);
					printf("lv_img_set_src success! File: %s\r\n", vpetEatingArray[*num]);
					display_finished = 0;
					return 0;
				}
				else if(*num == 19){
					display_finished = 1;
					return 1;
				}
				break;
		case 11:
				if(*num < 41){
					lv_img_cache_invalidate_src(glassArray[*num - 1]);
					lv_img_set_src(lv_gui_deskop.screen_gif, glassArray[*num]);
					printf("lv_img_set_src success! File: %s\r\n", glassArray[*num]);
					display_finished = 0;
					return 0;
				}
				else if(*num == 41){
					display_finished = 1;
					return 1;
				}
				break;
		
}
	return 0;
}

void ZoomVPetPattern(uint16_t factor)
{
	lv_img_set_zoom(lv_gui_deskop.screen_gif, factor);
}
