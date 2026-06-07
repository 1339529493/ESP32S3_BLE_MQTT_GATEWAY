/*
* Copyright 2024 NXP
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/


/*********************
 *      INCLUDES
 *********************/
#include <stdio.h>
#include "lvgl.h"
#include "custom.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/**
 * Create a demo application
 */

#include "time.h"
#include "gw_log.h"
#define TAG "DISPLAY"

lv_ui guider_ui;
extern int desktop_digital_clock_1_min_value;
extern int desktop_digital_clock_1_hour_value;
extern int desktop_digital_clock_1_sec_value;
extern int desktop_analog_clock_1_hour_value;
extern int desktop_analog_clock_1_min_value;
extern int desktop_analog_clock_1_sec_value;

void custom_init(lv_ui *ui)
{
    time_t now;
    struct tm timeinfo;
    char strftime_buf[64];
    
    time(&now);
    localtime_r(&now, &timeinfo);
    desktop_digital_clock_1_min_value = desktop_analog_clock_1_hour_value = timeinfo.tm_min;
    desktop_digital_clock_1_hour_value = desktop_analog_clock_1_hour_value = timeinfo.tm_hour;
    desktop_digital_clock_1_sec_value = desktop_analog_clock_1_sec_value = timeinfo.tm_sec;
    strftime(strftime_buf, sizeof(strftime_buf), "%Y/%m/%d", &timeinfo);
    LOGI(TAG, "Current time: %s", strftime_buf);
    if (ui->desktop_datetext_1) {
        lv_label_set_text(ui->desktop_datetext_1, strftime_buf);
    }
}

void custom_ing(lv_ui *ui)
{
    if (desktop_digital_clock_1_hour_value == 0 && desktop_digital_clock_1_min_value == 0 && desktop_digital_clock_1_sec_value == 0) {

    }
}
void custom_deinit(lv_ui *ui)
{

}

void setup_ui_desktop(lv_ui *ui)
{
    setup_bottom_layer();
    init_scr_del_flag(ui);
    init_keyboard(ui);
    setup_scr_desktop(ui);
    lv_screen_load(ui->desktop);
}

void setup_ui_status_list(lv_ui *ui)
{
    setup_bottom_layer();
    init_scr_del_flag(ui);
    init_keyboard(ui);
    setup_scr_status_list(ui);
    lv_screen_load(ui->status_list);
}