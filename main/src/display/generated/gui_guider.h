/*
* Copyright 2026 NXP
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/

#ifndef GUI_GUIDER_H
#define GUI_GUIDER_H
#ifdef __cplusplus
extern "C" {
#endif

#include "lvgl.h"


typedef struct
{
  
	lv_obj_t *desktop;
	bool desktop_del;
	lv_obj_t *desktop_cont_1;
	lv_obj_t *desktop_desktop_1;
	lv_obj_t *desktop_datetext_1;
	lv_obj_t *desktop_analog_clock_1;
	lv_obj_t *desktop_analog_clock_1_hour_needle;
	lv_obj_t *desktop_analog_clock_1_min_needle;
	lv_obj_t *desktop_analog_clock_1_sec_needle;
	lv_obj_t *desktop_digital_clock_1;
	lv_obj_t *status_list;
	bool status_list_del;
	lv_obj_t *status_list_cont_6;
	lv_obj_t *status_list_img_1;
	lv_obj_t *status_list_wifi;
	lv_obj_t *status_list_img_wifi_0;
	lv_obj_t *status_list_spangroup_1;
	lv_span_t *status_list_spangroup_1_span;
	lv_obj_t *status_list_label_status_wifi;
	lv_obj_t *status_list_spangroup_wifi;
	lv_span_t *status_list_spangroup_wifi_span;
	lv_obj_t *status_list_cont_2;
	lv_obj_t *status_list_mqtt;
	lv_obj_t *status_list_img_mqtt_0;
	lv_obj_t *status_list_spangroup_3;
	lv_span_t *status_list_spangroup_3_span;
	lv_obj_t *status_list_label_status_mqtt;
	lv_obj_t *status_list_spangroup_mqtt;
	lv_span_t *status_list_spangroup_mqtt_span;
	lv_obj_t *status_list_ble;
	lv_obj_t *status_list_img_ble_0;
	lv_obj_t *status_list_spangroup_2;
	lv_span_t *status_list_spangroup_2_span;
	lv_obj_t *status_list_label_status_ble;
	lv_obj_t *status_list_spangroup_ble;
	lv_span_t *status_list_spangroup_ble_span;
}lv_ui;

typedef void (*ui_setup_scr_t)(lv_ui * ui);

void ui_init_style(lv_style_t * style);

void ui_load_scr_animation(lv_ui *ui, lv_obj_t ** new_scr, bool new_scr_del, bool * old_scr_del, ui_setup_scr_t setup_scr,
                           lv_screen_load_anim_t anim_type, uint32_t time, uint32_t delay, bool is_clean, bool auto_del);

void ui_animation(void * var, uint32_t duration, int32_t delay, int32_t start_value, int32_t end_value, lv_anim_path_cb_t path_cb,
                  uint32_t repeat_cnt, uint32_t repeat_delay, uint32_t playback_time, uint32_t playback_delay,
                  lv_anim_exec_xcb_t exec_cb, lv_anim_start_cb_t start_cb, lv_anim_completed_cb_t ready_cb, lv_anim_deleted_cb_t deleted_cb);


void init_scr_del_flag(lv_ui *ui);

void setup_bottom_layer(void);

void setup_ui(lv_ui *ui);

void video_play(lv_ui *ui);

void init_keyboard(lv_ui *ui);

extern lv_ui guider_ui;


void setup_scr_desktop(lv_ui *ui);
void setup_scr_status_list(lv_ui *ui);
LV_IMAGE_DECLARE(_a_RGB565A8_240x320);
LV_IMAGE_DECLARE(_wifi_green_RGB565A8_32x32);
LV_IMAGE_DECLARE(_mqtt_RGB565A8_32x32);
LV_IMAGE_DECLARE(_ble_RGB565A8_32x32);

LV_FONT_DECLARE(lv_font_AlexBrush_Regular_16)
LV_FONT_DECLARE(lv_font_montserratMedium_14)
LV_FONT_DECLARE(lv_font_Alatsi_Regular_25)
LV_FONT_DECLARE(lv_font_montserratMedium_24)
LV_FONT_DECLARE(lv_font_montserratMedium_12)


#ifdef __cplusplus
}
#endif
#endif
