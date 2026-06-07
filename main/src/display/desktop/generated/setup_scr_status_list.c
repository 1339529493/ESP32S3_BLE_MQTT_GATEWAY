/*
* Copyright 2026 NXP
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/

#include "lvgl.h"
#include <stdio.h>
#include "gui_guider.h"
#include "events_init.h"
#include "widgets_init.h"
#include "custom.h"



void setup_scr_status_list(lv_ui *ui)
{
    //Write codes status_list
    ui->status_list = lv_obj_create(NULL);
    lv_obj_set_size(ui->status_list, 240, 320);
    lv_obj_set_scrollbar_mode(ui->status_list, LV_SCROLLBAR_MODE_OFF);

    //Write style for status_list, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->status_list, 4, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->status_list, lv_color_hex(0x1A1C23), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->status_list, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes status_list_cont_6
    ui->status_list_cont_6 = lv_obj_create(ui->status_list);
    lv_obj_set_pos(ui->status_list_cont_6, 0, 0);
    lv_obj_set_size(ui->status_list_cont_6, 240, 320);
    lv_obj_set_scrollbar_mode(ui->status_list_cont_6, LV_SCROLLBAR_MODE_OFF);

    //Write style for status_list_cont_6, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->status_list_cont_6, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->status_list_cont_6, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->status_list_cont_6, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->status_list_cont_6, LV_BORDER_SIDE_FULL, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->status_list_cont_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->status_list_cont_6, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->status_list_cont_6, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->status_list_cont_6, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->status_list_cont_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->status_list_cont_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->status_list_cont_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->status_list_cont_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->status_list_cont_6, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes status_list_img_1
    ui->status_list_img_1 = lv_image_create(ui->status_list_cont_6);
    lv_obj_set_pos(ui->status_list_img_1, 0, 0);
    lv_obj_set_size(ui->status_list_img_1, 240, 320);
    lv_obj_add_flag(ui->status_list_img_1, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->status_list_img_1, &_a_RGB565A8_240x320);
    lv_image_set_pivot(ui->status_list_img_1, 50,50);
    lv_image_set_rotation(ui->status_list_img_1, 0);

    //Write style for status_list_img_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->status_list_img_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->status_list_img_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes status_list_wifi
    ui->status_list_wifi = lv_obj_create(ui->status_list_cont_6);
    lv_obj_set_pos(ui->status_list_wifi, 20, 60);
    lv_obj_set_size(ui->status_list_wifi, 200, 70);
    lv_obj_set_scrollbar_mode(ui->status_list_wifi, LV_SCROLLBAR_MODE_OFF);

    //Write style for status_list_wifi, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->status_list_wifi, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->status_list_wifi, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->status_list_wifi, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->status_list_wifi, LV_BORDER_SIDE_FULL, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->status_list_wifi, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->status_list_wifi, 129, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->status_list_wifi, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->status_list_wifi, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->status_list_wifi, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->status_list_wifi, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->status_list_wifi, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->status_list_wifi, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->status_list_wifi, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes status_list_img_wifi_0
    ui->status_list_img_wifi_0 = lv_image_create(ui->status_list_wifi);
    lv_obj_set_pos(ui->status_list_img_wifi_0, 10, 10);
    lv_obj_set_size(ui->status_list_img_wifi_0, 32, 32);
    lv_obj_add_flag(ui->status_list_img_wifi_0, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->status_list_img_wifi_0, &_wifi_green_RGB565A8_32x32);
    lv_image_set_pivot(ui->status_list_img_wifi_0, 50,50);
    lv_image_set_rotation(ui->status_list_img_wifi_0, 0);

    //Write style for status_list_img_wifi_0, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->status_list_img_wifi_0, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->status_list_img_wifi_0, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes status_list_spangroup_1
    ui->status_list_spangroup_1 = lv_spangroup_create(ui->status_list_wifi);
    lv_obj_set_pos(ui->status_list_spangroup_1, 50, 10);
    lv_obj_set_size(ui->status_list_spangroup_1, 100, 25);
    lv_spangroup_set_align(ui->status_list_spangroup_1, LV_TEXT_ALIGN_LEFT);
    lv_spangroup_set_overflow(ui->status_list_spangroup_1, LV_SPAN_OVERFLOW_CLIP);
    lv_spangroup_set_mode(ui->status_list_spangroup_1, LV_SPAN_MODE_BREAK);
    //create span
    ui->status_list_spangroup_1_span = lv_spangroup_new_span(ui->status_list_spangroup_1);
    lv_span_set_text(ui->status_list_spangroup_1_span, "wifi");
    lv_style_set_text_color(lv_span_get_style(ui->status_list_spangroup_1_span), lv_color_hex(0x000000));
    lv_style_set_text_decor(lv_span_get_style(ui->status_list_spangroup_1_span), LV_TEXT_DECOR_NONE);
    lv_style_set_text_font(lv_span_get_style(ui->status_list_spangroup_1_span), &lv_font_montserratMedium_24);

    //Write style state: LV_STATE_DEFAULT for &style_status_list_spangroup_1_main_main_default
    static lv_style_t style_status_list_spangroup_1_main_main_default;
    ui_init_style(&style_status_list_spangroup_1_main_main_default);

    lv_style_set_border_width(&style_status_list_spangroup_1_main_main_default, 0);
    lv_style_set_radius(&style_status_list_spangroup_1_main_main_default, 0);
    lv_style_set_bg_opa(&style_status_list_spangroup_1_main_main_default, 0);
    lv_style_set_pad_top(&style_status_list_spangroup_1_main_main_default, 0);
    lv_style_set_pad_right(&style_status_list_spangroup_1_main_main_default, 0);
    lv_style_set_pad_bottom(&style_status_list_spangroup_1_main_main_default, 0);
    lv_style_set_pad_left(&style_status_list_spangroup_1_main_main_default, 0);
    lv_style_set_shadow_width(&style_status_list_spangroup_1_main_main_default, 0);
    lv_obj_add_style(ui->status_list_spangroup_1, &style_status_list_spangroup_1_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_spangroup_refr_mode(ui->status_list_spangroup_1);

    //Write codes status_list_label_status_wifi
    ui->status_list_label_status_wifi = lv_label_create(ui->status_list_wifi);
    lv_obj_set_pos(ui->status_list_label_status_wifi, 50, 35);
    lv_obj_set_size(ui->status_list_label_status_wifi, 45, 20);
    lv_label_set_text(ui->status_list_label_status_wifi, "status :");
    lv_label_set_long_mode(ui->status_list_label_status_wifi, LV_LABEL_LONG_WRAP);

    //Write style for status_list_label_status_wifi, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->status_list_label_status_wifi, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->status_list_label_status_wifi, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->status_list_label_status_wifi, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->status_list_label_status_wifi, &lv_font_montserratMedium_12, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->status_list_label_status_wifi, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->status_list_label_status_wifi, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->status_list_label_status_wifi, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->status_list_label_status_wifi, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->status_list_label_status_wifi, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->status_list_label_status_wifi, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->status_list_label_status_wifi, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->status_list_label_status_wifi, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->status_list_label_status_wifi, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->status_list_label_status_wifi, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes status_list_spangroup_wifi
    ui->status_list_spangroup_wifi = lv_spangroup_create(ui->status_list_wifi);
    lv_obj_set_pos(ui->status_list_spangroup_wifi, 100, 35);
    lv_obj_set_size(ui->status_list_spangroup_wifi, 70, 20);
    lv_spangroup_set_align(ui->status_list_spangroup_wifi, LV_TEXT_ALIGN_LEFT);
    lv_spangroup_set_overflow(ui->status_list_spangroup_wifi, LV_SPAN_OVERFLOW_CLIP);
    lv_spangroup_set_mode(ui->status_list_spangroup_wifi, LV_SPAN_MODE_BREAK);
    //create span
    ui->status_list_spangroup_wifi_span = lv_spangroup_new_span(ui->status_list_spangroup_wifi);
    lv_span_set_text(ui->status_list_spangroup_wifi_span, "disconnect");
    lv_style_set_text_color(lv_span_get_style(ui->status_list_spangroup_wifi_span), lv_color_hex(0x000000));
    lv_style_set_text_decor(lv_span_get_style(ui->status_list_spangroup_wifi_span), LV_TEXT_DECOR_NONE);
    lv_style_set_text_font(lv_span_get_style(ui->status_list_spangroup_wifi_span), &lv_font_montserratMedium_12);

    //Write style state: LV_STATE_DEFAULT for &style_status_list_spangroup_wifi_main_main_default
    static lv_style_t style_status_list_spangroup_wifi_main_main_default;
    ui_init_style(&style_status_list_spangroup_wifi_main_main_default);

    lv_style_set_border_width(&style_status_list_spangroup_wifi_main_main_default, 0);
    lv_style_set_radius(&style_status_list_spangroup_wifi_main_main_default, 0);
    lv_style_set_bg_opa(&style_status_list_spangroup_wifi_main_main_default, 0);
    lv_style_set_pad_top(&style_status_list_spangroup_wifi_main_main_default, 0);
    lv_style_set_pad_right(&style_status_list_spangroup_wifi_main_main_default, 0);
    lv_style_set_pad_bottom(&style_status_list_spangroup_wifi_main_main_default, 0);
    lv_style_set_pad_left(&style_status_list_spangroup_wifi_main_main_default, 0);
    lv_style_set_shadow_width(&style_status_list_spangroup_wifi_main_main_default, 0);
    lv_obj_add_style(ui->status_list_spangroup_wifi, &style_status_list_spangroup_wifi_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_spangroup_refr_mode(ui->status_list_spangroup_wifi);

    //Write codes status_list_cont_2
    ui->status_list_cont_2 = lv_obj_create(ui->status_list_cont_6);
    lv_obj_set_pos(ui->status_list_cont_2, 0, 0);
    lv_obj_set_size(ui->status_list_cont_2, 240, 50);
    lv_obj_set_scrollbar_mode(ui->status_list_cont_2, LV_SCROLLBAR_MODE_OFF);

    //Write style for status_list_cont_2, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->status_list_cont_2, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->status_list_cont_2, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->status_list_cont_2, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->status_list_cont_2, LV_BORDER_SIDE_FULL, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->status_list_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->status_list_cont_2, 129, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->status_list_cont_2, lv_color_hex(0x3B82F6), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->status_list_cont_2, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->status_list_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->status_list_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->status_list_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->status_list_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->status_list_cont_2, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes status_list_mqtt
    ui->status_list_mqtt = lv_obj_create(ui->status_list_cont_6);
    lv_obj_set_pos(ui->status_list_mqtt, 20, 220);
    lv_obj_set_size(ui->status_list_mqtt, 200, 70);
    lv_obj_set_scrollbar_mode(ui->status_list_mqtt, LV_SCROLLBAR_MODE_OFF);

    //Write style for status_list_mqtt, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->status_list_mqtt, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->status_list_mqtt, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->status_list_mqtt, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->status_list_mqtt, LV_BORDER_SIDE_FULL, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->status_list_mqtt, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->status_list_mqtt, 129, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->status_list_mqtt, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->status_list_mqtt, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->status_list_mqtt, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->status_list_mqtt, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->status_list_mqtt, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->status_list_mqtt, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->status_list_mqtt, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes status_list_img_mqtt_0
    ui->status_list_img_mqtt_0 = lv_image_create(ui->status_list_mqtt);
    lv_obj_set_pos(ui->status_list_img_mqtt_0, 10, 10);
    lv_obj_set_size(ui->status_list_img_mqtt_0, 32, 32);
    lv_obj_add_flag(ui->status_list_img_mqtt_0, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->status_list_img_mqtt_0, &_mqtt_RGB565A8_32x32);
    lv_image_set_pivot(ui->status_list_img_mqtt_0, 50,50);
    lv_image_set_rotation(ui->status_list_img_mqtt_0, 0);

    //Write style for status_list_img_mqtt_0, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->status_list_img_mqtt_0, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->status_list_img_mqtt_0, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes status_list_spangroup_3
    ui->status_list_spangroup_3 = lv_spangroup_create(ui->status_list_mqtt);
    lv_obj_set_pos(ui->status_list_spangroup_3, 50, 10);
    lv_obj_set_size(ui->status_list_spangroup_3, 100, 25);
    lv_spangroup_set_align(ui->status_list_spangroup_3, LV_TEXT_ALIGN_LEFT);
    lv_spangroup_set_overflow(ui->status_list_spangroup_3, LV_SPAN_OVERFLOW_CLIP);
    lv_spangroup_set_mode(ui->status_list_spangroup_3, LV_SPAN_MODE_BREAK);
    //create span
    ui->status_list_spangroup_3_span = lv_spangroup_new_span(ui->status_list_spangroup_3);
    lv_span_set_text(ui->status_list_spangroup_3_span, "mqtt\n");
    lv_style_set_text_color(lv_span_get_style(ui->status_list_spangroup_3_span), lv_color_hex(0x000000));
    lv_style_set_text_decor(lv_span_get_style(ui->status_list_spangroup_3_span), LV_TEXT_DECOR_NONE);
    lv_style_set_text_font(lv_span_get_style(ui->status_list_spangroup_3_span), &lv_font_montserratMedium_24);

    //Write style state: LV_STATE_DEFAULT for &style_status_list_spangroup_3_main_main_default
    static lv_style_t style_status_list_spangroup_3_main_main_default;
    ui_init_style(&style_status_list_spangroup_3_main_main_default);

    lv_style_set_border_width(&style_status_list_spangroup_3_main_main_default, 0);
    lv_style_set_radius(&style_status_list_spangroup_3_main_main_default, 0);
    lv_style_set_bg_opa(&style_status_list_spangroup_3_main_main_default, 0);
    lv_style_set_pad_top(&style_status_list_spangroup_3_main_main_default, 0);
    lv_style_set_pad_right(&style_status_list_spangroup_3_main_main_default, 0);
    lv_style_set_pad_bottom(&style_status_list_spangroup_3_main_main_default, 0);
    lv_style_set_pad_left(&style_status_list_spangroup_3_main_main_default, 0);
    lv_style_set_shadow_width(&style_status_list_spangroup_3_main_main_default, 0);
    lv_obj_add_style(ui->status_list_spangroup_3, &style_status_list_spangroup_3_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_spangroup_refr_mode(ui->status_list_spangroup_3);

    //Write codes status_list_label_status_mqtt
    ui->status_list_label_status_mqtt = lv_label_create(ui->status_list_mqtt);
    lv_obj_set_pos(ui->status_list_label_status_mqtt, 50, 35);
    lv_obj_set_size(ui->status_list_label_status_mqtt, 45, 20);
    lv_label_set_text(ui->status_list_label_status_mqtt, "status :");
    lv_label_set_long_mode(ui->status_list_label_status_mqtt, LV_LABEL_LONG_WRAP);

    //Write style for status_list_label_status_mqtt, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->status_list_label_status_mqtt, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->status_list_label_status_mqtt, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->status_list_label_status_mqtt, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->status_list_label_status_mqtt, &lv_font_montserratMedium_12, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->status_list_label_status_mqtt, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->status_list_label_status_mqtt, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->status_list_label_status_mqtt, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->status_list_label_status_mqtt, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->status_list_label_status_mqtt, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->status_list_label_status_mqtt, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->status_list_label_status_mqtt, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->status_list_label_status_mqtt, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->status_list_label_status_mqtt, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->status_list_label_status_mqtt, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes status_list_spangroup_mqtt
    ui->status_list_spangroup_mqtt = lv_spangroup_create(ui->status_list_mqtt);
    lv_obj_set_pos(ui->status_list_spangroup_mqtt, 100, 35);
    lv_obj_set_size(ui->status_list_spangroup_mqtt, 70, 20);
    lv_spangroup_set_align(ui->status_list_spangroup_mqtt, LV_TEXT_ALIGN_LEFT);
    lv_spangroup_set_overflow(ui->status_list_spangroup_mqtt, LV_SPAN_OVERFLOW_CLIP);
    lv_spangroup_set_mode(ui->status_list_spangroup_mqtt, LV_SPAN_MODE_BREAK);
    //create span
    ui->status_list_spangroup_mqtt_span = lv_spangroup_new_span(ui->status_list_spangroup_mqtt);
    lv_span_set_text(ui->status_list_spangroup_mqtt_span, "disconnect");
    lv_style_set_text_color(lv_span_get_style(ui->status_list_spangroup_mqtt_span), lv_color_hex(0x000000));
    lv_style_set_text_decor(lv_span_get_style(ui->status_list_spangroup_mqtt_span), LV_TEXT_DECOR_NONE);
    lv_style_set_text_font(lv_span_get_style(ui->status_list_spangroup_mqtt_span), &lv_font_montserratMedium_12);

    //Write style state: LV_STATE_DEFAULT for &style_status_list_spangroup_mqtt_main_main_default
    static lv_style_t style_status_list_spangroup_mqtt_main_main_default;
    ui_init_style(&style_status_list_spangroup_mqtt_main_main_default);

    lv_style_set_border_width(&style_status_list_spangroup_mqtt_main_main_default, 0);
    lv_style_set_radius(&style_status_list_spangroup_mqtt_main_main_default, 0);
    lv_style_set_bg_opa(&style_status_list_spangroup_mqtt_main_main_default, 0);
    lv_style_set_pad_top(&style_status_list_spangroup_mqtt_main_main_default, 0);
    lv_style_set_pad_right(&style_status_list_spangroup_mqtt_main_main_default, 0);
    lv_style_set_pad_bottom(&style_status_list_spangroup_mqtt_main_main_default, 0);
    lv_style_set_pad_left(&style_status_list_spangroup_mqtt_main_main_default, 0);
    lv_style_set_shadow_width(&style_status_list_spangroup_mqtt_main_main_default, 0);
    lv_obj_add_style(ui->status_list_spangroup_mqtt, &style_status_list_spangroup_mqtt_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_spangroup_refr_mode(ui->status_list_spangroup_mqtt);

    //Write codes status_list_ble
    ui->status_list_ble = lv_obj_create(ui->status_list_cont_6);
    lv_obj_set_pos(ui->status_list_ble, 20, 140);
    lv_obj_set_size(ui->status_list_ble, 200, 70);
    lv_obj_set_scrollbar_mode(ui->status_list_ble, LV_SCROLLBAR_MODE_OFF);

    //Write style for status_list_ble, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->status_list_ble, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->status_list_ble, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->status_list_ble, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->status_list_ble, LV_BORDER_SIDE_FULL, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->status_list_ble, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->status_list_ble, 128, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->status_list_ble, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->status_list_ble, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->status_list_ble, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->status_list_ble, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->status_list_ble, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->status_list_ble, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->status_list_ble, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes status_list_img_ble_0
    ui->status_list_img_ble_0 = lv_image_create(ui->status_list_ble);
    lv_obj_set_pos(ui->status_list_img_ble_0, 10, 10);
    lv_obj_set_size(ui->status_list_img_ble_0, 32, 32);
    lv_obj_add_flag(ui->status_list_img_ble_0, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->status_list_img_ble_0, &_ble_RGB565A8_32x32);
    lv_image_set_pivot(ui->status_list_img_ble_0, 50,50);
    lv_image_set_rotation(ui->status_list_img_ble_0, 0);

    //Write style for status_list_img_ble_0, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->status_list_img_ble_0, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->status_list_img_ble_0, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes status_list_spangroup_2
    ui->status_list_spangroup_2 = lv_spangroup_create(ui->status_list_ble);
    lv_obj_set_pos(ui->status_list_spangroup_2, 50, 10);
    lv_obj_set_size(ui->status_list_spangroup_2, 100, 25);
    lv_spangroup_set_align(ui->status_list_spangroup_2, LV_TEXT_ALIGN_LEFT);
    lv_spangroup_set_overflow(ui->status_list_spangroup_2, LV_SPAN_OVERFLOW_CLIP);
    lv_spangroup_set_mode(ui->status_list_spangroup_2, LV_SPAN_MODE_BREAK);
    //create span
    ui->status_list_spangroup_2_span = lv_spangroup_new_span(ui->status_list_spangroup_2);
    lv_span_set_text(ui->status_list_spangroup_2_span, "ble\n");
    lv_style_set_text_color(lv_span_get_style(ui->status_list_spangroup_2_span), lv_color_hex(0x000000));
    lv_style_set_text_decor(lv_span_get_style(ui->status_list_spangroup_2_span), LV_TEXT_DECOR_NONE);
    lv_style_set_text_font(lv_span_get_style(ui->status_list_spangroup_2_span), &lv_font_montserratMedium_24);

    //Write style state: LV_STATE_DEFAULT for &style_status_list_spangroup_2_main_main_default
    static lv_style_t style_status_list_spangroup_2_main_main_default;
    ui_init_style(&style_status_list_spangroup_2_main_main_default);

    lv_style_set_border_width(&style_status_list_spangroup_2_main_main_default, 0);
    lv_style_set_radius(&style_status_list_spangroup_2_main_main_default, 0);
    lv_style_set_bg_opa(&style_status_list_spangroup_2_main_main_default, 0);
    lv_style_set_pad_top(&style_status_list_spangroup_2_main_main_default, 0);
    lv_style_set_pad_right(&style_status_list_spangroup_2_main_main_default, 0);
    lv_style_set_pad_bottom(&style_status_list_spangroup_2_main_main_default, 0);
    lv_style_set_pad_left(&style_status_list_spangroup_2_main_main_default, 0);
    lv_style_set_shadow_width(&style_status_list_spangroup_2_main_main_default, 0);
    lv_obj_add_style(ui->status_list_spangroup_2, &style_status_list_spangroup_2_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_spangroup_refr_mode(ui->status_list_spangroup_2);

    //Write codes status_list_label_status_ble
    ui->status_list_label_status_ble = lv_label_create(ui->status_list_ble);
    lv_obj_set_pos(ui->status_list_label_status_ble, 50, 34);
    lv_obj_set_size(ui->status_list_label_status_ble, 45, 20);
    lv_label_set_text(ui->status_list_label_status_ble, "status :");
    lv_label_set_long_mode(ui->status_list_label_status_ble, LV_LABEL_LONG_WRAP);

    //Write style for status_list_label_status_ble, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->status_list_label_status_ble, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->status_list_label_status_ble, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->status_list_label_status_ble, lv_color_hex(0x000000), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->status_list_label_status_ble, &lv_font_montserratMedium_12, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->status_list_label_status_ble, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->status_list_label_status_ble, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_line_space(ui->status_list_label_status_ble, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->status_list_label_status_ble, LV_TEXT_ALIGN_LEFT, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->status_list_label_status_ble, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->status_list_label_status_ble, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->status_list_label_status_ble, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->status_list_label_status_ble, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->status_list_label_status_ble, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->status_list_label_status_ble, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes status_list_spangroup_ble
    ui->status_list_spangroup_ble = lv_spangroup_create(ui->status_list_ble);
    lv_obj_set_pos(ui->status_list_spangroup_ble, 100, 35);
    lv_obj_set_size(ui->status_list_spangroup_ble, 70, 20);
    lv_spangroup_set_align(ui->status_list_spangroup_ble, LV_TEXT_ALIGN_LEFT);
    lv_spangroup_set_overflow(ui->status_list_spangroup_ble, LV_SPAN_OVERFLOW_CLIP);
    lv_spangroup_set_mode(ui->status_list_spangroup_ble, LV_SPAN_MODE_BREAK);
    //create span
    ui->status_list_spangroup_ble_span = lv_spangroup_new_span(ui->status_list_spangroup_ble);
    lv_span_set_text(ui->status_list_spangroup_ble_span, "disconnect");
    lv_style_set_text_color(lv_span_get_style(ui->status_list_spangroup_ble_span), lv_color_hex(0x000000));
    lv_style_set_text_decor(lv_span_get_style(ui->status_list_spangroup_ble_span), LV_TEXT_DECOR_NONE);
    lv_style_set_text_font(lv_span_get_style(ui->status_list_spangroup_ble_span), &lv_font_montserratMedium_12);

    //Write style state: LV_STATE_DEFAULT for &style_status_list_spangroup_ble_main_main_default
    static lv_style_t style_status_list_spangroup_ble_main_main_default;
    ui_init_style(&style_status_list_spangroup_ble_main_main_default);

    lv_style_set_border_width(&style_status_list_spangroup_ble_main_main_default, 0);
    lv_style_set_radius(&style_status_list_spangroup_ble_main_main_default, 0);
    lv_style_set_bg_opa(&style_status_list_spangroup_ble_main_main_default, 0);
    lv_style_set_pad_top(&style_status_list_spangroup_ble_main_main_default, 0);
    lv_style_set_pad_right(&style_status_list_spangroup_ble_main_main_default, 0);
    lv_style_set_pad_bottom(&style_status_list_spangroup_ble_main_main_default, 0);
    lv_style_set_pad_left(&style_status_list_spangroup_ble_main_main_default, 0);
    lv_style_set_shadow_width(&style_status_list_spangroup_ble_main_main_default, 0);
    lv_obj_add_style(ui->status_list_spangroup_ble, &style_status_list_spangroup_ble_main_main_default, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_spangroup_refr_mode(ui->status_list_spangroup_ble);

    //The custom code of status_list.


    //Update current screen layout.
    lv_obj_update_layout(ui->status_list);

}
