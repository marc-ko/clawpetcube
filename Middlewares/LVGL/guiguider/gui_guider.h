/*
 * Copyright 2022 NXP
 * SPDX-License-Identifier: MIT
 */

#ifndef GUI_GUIDER_H
#define GUI_GUIDER_H
#ifdef __cplusplus
extern "C"
{
#endif

#include "sys.h"
#include "lvgl.h"
#include "guider_fonts.h"

	typedef struct
	{
		lv_obj_t *screen;
		lv_obj_t *screen_gif;
		lv_obj_t *screen_hour;
		lv_obj_t *screen_min;
		lv_obj_t *screen_second;
		lv_obj_t *screen_city;
		lv_obj_t *screen_weather;
		lv_obj_t *screen_weather_info;
		lv_obj_t *screen_date;
		lv_obj_t *screen_week;
		lv_obj_t *screen_weather_text;
		lv_obj_t *screen_temperature;
		lv_obj_t *screen_coins;
		lv_obj_t *screen_emotion;
		lv_obj_t *screen_hunger;
		lv_obj_t *screen_exp;
		lv_obj_t *screen_energy;
	} lv_ui_deskop;

	typedef struct
	{
		lv_obj_t *screen;
	} lv_ui_alarm;

	typedef struct
	{
		lv_obj_t *selectFunction;
		lv_obj_t *selectFunction_clock_pattern;
		lv_obj_t *selectFunction_picture_pattern;
		lv_obj_t *selectFunction_projection_pattern;
		lv_obj_t *selectFunction_set_pattern;
		lv_obj_t *selectFunction_clock_label;
		lv_obj_t *selectFunction_picture_label;
		lv_obj_t *selectFunction_projection_label;
		lv_obj_t *selectFunction_set_label;
	} lv_ui_selectFunction;

	typedef struct
	{
		lv_obj_t *screen;
		lv_obj_t *screen_init_progress_bar;
		lv_obj_t *screen_init_progress_label;
	} lv_ui_init;

	typedef struct
	{
		lv_obj_t *screen;
		lv_obj_t *screen_date_label;
		lv_obj_t *screen_date_year_dropdown;
		lv_obj_t *screen_date_month_dropdown;
		lv_obj_t *screen_date_day_dropdown;
		lv_obj_t *screen_time_label;
		lv_obj_t *screen_am_pm_dropdown;
		lv_obj_t *screen_hour_dropdown;
		lv_obj_t *screen_min_dropdown;
		lv_obj_t *screen_switch_on_off;
		lv_obj_t *screen_switch_label;
	} lv_ui_functionClock;

	typedef struct
	{
		lv_obj_t *screen;
		lv_obj_t *screen_cityNameLabel;
		lv_obj_t *screen_wifiNameLabel;
		lv_obj_t *screen_passwordLabel;
		lv_obj_t *screen_wifiNameText;
		lv_obj_t *screen_passwordText;
		lv_obj_t *screen_cityLabel;
	} lv_ui_functionSet;

	typedef struct
	{
		lv_obj_t *screen;
		lv_obj_t *screen_loading_bar;
		lv_obj_t *screen_saving_label;
	} lv_ui_functionSetLoading;

	void updateDeskopTime(u8 hour, u8 minute, u8 sec);
	void updateDeskopDate(u8 month, u8 day, u8 week);
	void updateDeskopCity(u8 cityNum);
	void updateDeskopWeatherText(u8 weatherCode);
	void updateDeskopTemperature(int temperature);
	void updateDeskopWeatherPattern(u8 weatherCode);
	void updateDeskopUserInfo(u8 coins, u8 hunger, u8 exp, u8 energy);
	int updateVPetPattern(u8 *num, u8 arraynum);

	void setup_ui_deskop(lv_ui_deskop *ui);
	void setup_scr_deskop(lv_ui_deskop *ui);

	void setup_scr_init(lv_ui_init *ui);
	void setup_ui_init(lv_ui_init *ui);

	void updateInitBar(u8 num);
	void updateInitLabel(char str[]);

	LV_IMG_DECLARE(weatherPatternDefault);
	// LV_IMG_DECLARE(dancePatternDefault);
	LV_IMG_DECLARE(weatherPatternRead);
	// LV_IMG_DECLARE(dancePatternRead);

	LV_IMG_DECLARE(clockFuntionPattern);
	LV_IMG_DECLARE(pictureFuntionPattern);
	LV_IMG_DECLARE(projectionFuntionPattern);
	LV_IMG_DECLARE(setFuntionPattern);

#ifdef __cplusplus
}
#endif
#endif
