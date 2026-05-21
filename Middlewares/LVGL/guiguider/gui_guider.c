/*
 * Copyright 2022 NXP
 * SPDX-License-Identifier: MIT
 */

#include "lvgl.h"
#include "gui_guider.h"




void setup_ui_init(lv_ui_init *ui){
	setup_scr_init(ui);
	lv_scr_load(ui->screen);				// Initialising the UI no need to delete the old one
}

void setup_ui_deskop(lv_ui_deskop *ui){
	setup_scr_deskop(ui);
	
	lv_scr_load_anim(ui->screen,LV_SCR_LOAD_ANIM_FADE_ON,350,0,true);
	printf("loading \n");
}

