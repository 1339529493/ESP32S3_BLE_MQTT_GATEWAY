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



int desktop_analog_clock_1_hour_value = 12;
int desktop_analog_clock_1_min_value = 30;
int desktop_analog_clock_1_sec_value = 0;
int desktop_digital_clock_1_min_value = 25;
int desktop_digital_clock_1_hour_value = 11;
int desktop_digital_clock_1_sec_value = 50;
char desktop_digital_clock_1_meridiem[] = "AM";
void setup_scr_desktop(lv_ui *ui)
{
    //Write codes desktop
    ui->desktop = lv_obj_create(NULL);
    lv_obj_set_size(ui->desktop, 240, 320);
    lv_obj_set_scrollbar_mode(ui->desktop, LV_SCROLLBAR_MODE_OFF);

    //Write style for desktop, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->desktop, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes desktop_cont_1
    ui->desktop_cont_1 = lv_obj_create(ui->desktop);
    lv_obj_set_pos(ui->desktop_cont_1, 0, 0);
    lv_obj_set_size(ui->desktop_cont_1, 240, 320);
    lv_obj_set_scrollbar_mode(ui->desktop_cont_1, LV_SCROLLBAR_MODE_OFF);

    //Write style for desktop_cont_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_border_width(ui->desktop_cont_1, 2, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_opa(ui->desktop_cont_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_color(ui->desktop_cont_1, lv_color_hex(0x2195f6), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_side(ui->desktop_cont_1, LV_BORDER_SIDE_FULL, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->desktop_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->desktop_cont_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(ui->desktop_cont_1, lv_color_hex(0xffffff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_grad_dir(ui->desktop_cont_1, LV_GRAD_DIR_NONE, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->desktop_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->desktop_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->desktop_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->desktop_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->desktop_cont_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes desktop_desktop_1
    ui->desktop_desktop_1 = lv_image_create(ui->desktop_cont_1);
    lv_obj_set_pos(ui->desktop_desktop_1, 0, 0);
    lv_obj_set_size(ui->desktop_desktop_1, 240, 320);
    lv_obj_add_flag(ui->desktop_desktop_1, LV_OBJ_FLAG_CLICKABLE);
    lv_image_set_src(ui->desktop_desktop_1, &_a_RGB565A8_240x320);
    lv_image_set_pivot(ui->desktop_desktop_1, 50,50);
    lv_image_set_rotation(ui->desktop_desktop_1, 0);

    //Write style for desktop_desktop_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_image_recolor_opa(ui->desktop_desktop_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_image_opa(ui->desktop_desktop_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes desktop_datetext_1
    ui->desktop_datetext_1 = lv_label_create(ui->desktop_cont_1);
    lv_obj_set_pos(ui->desktop_datetext_1, 59, 23);
    lv_obj_set_size(ui->desktop_datetext_1, 130, 36);
    lv_label_set_text(ui->desktop_datetext_1, "2024/04/23");
    lv_obj_set_style_text_align(ui->desktop_datetext_1, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_add_flag(ui->desktop_datetext_1, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(ui->desktop_datetext_1, desktop_datetext_1_event_handler, LV_EVENT_ALL, NULL);

    //Write style for desktop_datetext_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_text_color(ui->desktop_datetext_1, lv_color_hex(0x009bff), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->desktop_datetext_1, &lv_font_AlexBrush_Regular_16, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->desktop_datetext_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->desktop_datetext_1, 1, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->desktop_datetext_1, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->desktop_datetext_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(ui->desktop_datetext_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_radius(ui->desktop_datetext_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->desktop_datetext_1, 7, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->desktop_datetext_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->desktop_datetext_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->desktop_datetext_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write codes desktop_analog_clock_1
    static bool desktop_analog_clock_1_timer_enabled = false;
    static const char * desktop_analog_clock_1_hour_ticks[] = {"12", "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", NULL};
    ui->desktop_analog_clock_1 = lv_scale_create(ui->desktop_cont_1);
    lv_obj_set_pos(ui->desktop_analog_clock_1, 59, 93);
    lv_obj_set_size(ui->desktop_analog_clock_1, 123, 123);
    lv_scale_set_mode(ui->desktop_analog_clock_1, LV_SCALE_MODE_ROUND_INNER);
    lv_scale_set_angle_range(ui->desktop_analog_clock_1, 360U);
    lv_scale_set_range(ui->desktop_analog_clock_1, 0U, 60U);
    lv_scale_set_rotation(ui->desktop_analog_clock_1, 270U);
    lv_obj_set_style_radius(ui->desktop_analog_clock_1, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_style_clip_corner(ui->desktop_analog_clock_1, true, LV_PART_MAIN);
    lv_obj_set_style_arc_width(ui->desktop_analog_clock_1, 0, LV_PART_MAIN);

    lv_scale_set_total_tick_count(ui->desktop_analog_clock_1, 61);

    lv_scale_set_major_tick_every(ui->desktop_analog_clock_1, 5);
    lv_scale_set_text_src(ui->desktop_analog_clock_1, desktop_analog_clock_1_hour_ticks);
    lv_obj_update_layout(ui->desktop_analog_clock_1);
    ui->desktop_analog_clock_1_hour_needle = lv_line_create(ui->desktop_analog_clock_1);
    lv_obj_set_style_line_width(ui->desktop_analog_clock_1_hour_needle, 4, LV_PART_MAIN);
    lv_obj_set_style_line_color(ui->desktop_analog_clock_1_hour_needle, lv_color_hex(0x1f1d21), LV_PART_MAIN);
    lv_obj_set_style_line_rounded(ui->desktop_analog_clock_1_hour_needle, true, LV_PART_MAIN);
    lv_scale_set_line_needle_value(ui->desktop_analog_clock_1, ui->desktop_analog_clock_1_hour_needle, 30, desktop_analog_clock_1_hour_value * 5);
    ui->desktop_analog_clock_1_min_needle = lv_line_create(ui->desktop_analog_clock_1);
    lv_obj_set_style_line_width(ui->desktop_analog_clock_1_min_needle, 3, LV_PART_MAIN);
    lv_obj_set_style_line_color(ui->desktop_analog_clock_1_min_needle, lv_color_hex(0x474840), LV_PART_MAIN);
    lv_obj_set_style_line_rounded(ui->desktop_analog_clock_1_min_needle, true, LV_PART_MAIN);
    lv_scale_set_line_needle_value(ui->desktop_analog_clock_1, ui->desktop_analog_clock_1_min_needle, 40, desktop_analog_clock_1_min_value);
    ui->desktop_analog_clock_1_sec_needle = lv_line_create(ui->desktop_analog_clock_1);
    lv_obj_set_style_line_width(ui->desktop_analog_clock_1_sec_needle, 2, LV_PART_MAIN);
    lv_obj_set_style_line_color(ui->desktop_analog_clock_1_sec_needle, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_line_rounded(ui->desktop_analog_clock_1_sec_needle, true, LV_PART_MAIN);
    lv_scale_set_line_needle_value(ui->desktop_analog_clock_1, ui->desktop_analog_clock_1_sec_needle, 60, desktop_analog_clock_1_sec_value);
    // create timer
    if (!desktop_analog_clock_1_timer_enabled) {
        lv_timer_create(desktop_analog_clock_1_timer, 1000, NULL);
        desktop_analog_clock_1_timer_enabled = true;
    }

    //Write style for desktop_analog_clock_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_bg_opa(ui->desktop_analog_clock_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->desktop_analog_clock_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //Write style for desktop_analog_clock_1, Part: LV_PART_INDICATOR, State: LV_STATE_DEFAULT.
    lv_obj_set_style_text_color(ui->desktop_analog_clock_1, lv_color_hex(0x000000), LV_PART_INDICATOR|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->desktop_analog_clock_1, &lv_font_montserratMedium_14, LV_PART_INDICATOR|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->desktop_analog_clock_1, 255, LV_PART_INDICATOR|LV_STATE_DEFAULT);
    lv_obj_set_style_line_width(ui->desktop_analog_clock_1, 2, LV_PART_INDICATOR|LV_STATE_DEFAULT);
    lv_obj_set_style_line_color(ui->desktop_analog_clock_1, lv_color_hex(0x000000), LV_PART_INDICATOR|LV_STATE_DEFAULT);
    lv_obj_set_style_line_opa(ui->desktop_analog_clock_1, 255, LV_PART_INDICATOR|LV_STATE_DEFAULT);
    lv_obj_set_style_line_rounded(ui->desktop_analog_clock_1, true, LV_PART_INDICATOR|LV_STATE_DEFAULT);
    lv_obj_set_style_length(ui->desktop_analog_clock_1, 10, LV_PART_INDICATOR|LV_STATE_DEFAULT);

    //Write style for desktop_analog_clock_1, Part: LV_PART_ITEMS, State: LV_STATE_DEFAULT.
    lv_obj_set_style_line_width(ui->desktop_analog_clock_1, 2, LV_PART_ITEMS|LV_STATE_DEFAULT);
    lv_obj_set_style_line_color(ui->desktop_analog_clock_1, lv_color_hex(0x000000), LV_PART_ITEMS|LV_STATE_DEFAULT);
    lv_obj_set_style_line_opa(ui->desktop_analog_clock_1, 255, LV_PART_ITEMS|LV_STATE_DEFAULT);
    lv_obj_set_style_line_rounded(ui->desktop_analog_clock_1, true, LV_PART_ITEMS|LV_STATE_DEFAULT);
    lv_obj_set_style_length(ui->desktop_analog_clock_1, 6, LV_PART_ITEMS|LV_STATE_DEFAULT);

    //Write codes desktop_digital_clock_1
    static bool desktop_digital_clock_1_timer_enabled = false;
    ui->desktop_digital_clock_1 = lv_label_create(ui->desktop_cont_1);
    lv_obj_set_pos(ui->desktop_digital_clock_1, 59, 258);
    lv_obj_set_size(ui->desktop_digital_clock_1, 130, 36);
    lv_label_set_text(ui->desktop_digital_clock_1, "11:25:50 AM");
    if (!desktop_digital_clock_1_timer_enabled) {
        lv_timer_create(desktop_digital_clock_1_timer, 1000, NULL);
        desktop_digital_clock_1_timer_enabled = true;
    }

    //Write style for desktop_digital_clock_1, Part: LV_PART_MAIN, State: LV_STATE_DEFAULT.
    lv_obj_set_style_radius(ui->desktop_digital_clock_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_color(ui->desktop_digital_clock_1, lv_color_hex(0x1dfff7), LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(ui->desktop_digital_clock_1, &lv_font_Alatsi_Regular_25, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_opa(ui->desktop_digital_clock_1, 255, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_letter_space(ui->desktop_digital_clock_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_text_align(ui->desktop_digital_clock_1, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(ui->desktop_digital_clock_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_top(ui->desktop_digital_clock_1, 7, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_right(ui->desktop_digital_clock_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_bottom(ui->desktop_digital_clock_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_pad_left(ui->desktop_digital_clock_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);
    lv_obj_set_style_shadow_width(ui->desktop_digital_clock_1, 0, LV_PART_MAIN|LV_STATE_DEFAULT);

    //The custom code of desktop.


    //Update current screen layout.
    lv_obj_update_layout(ui->desktop);

}
