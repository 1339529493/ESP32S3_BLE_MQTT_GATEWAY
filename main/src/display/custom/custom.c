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
#include "gateway_status.h"
#define TAG "DISPLAY"

lv_ui guider_ui;
extern int desktop_digital_clock_1_min_value;
extern int desktop_digital_clock_1_hour_value;
extern int desktop_digital_clock_1_sec_value;
extern int desktop_analog_clock_1_hour_value;
extern int desktop_analog_clock_1_min_value;
extern int desktop_analog_clock_1_sec_value;

// 定义一个静态定时器指针
static lv_timer_t *clock_timer = NULL;

/**
 * @brief LVGL 定时器回调函数：每秒更新一次时间
 */
static void clock_update_cb(lv_timer_t *timer)
{
   //更新日期
}

void custom_init_desktop(lv_ui *ui)
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

    // 2. 创建 LVGL 定时器，每 1000ms (1秒) 执行一次
    // 如果定时器已存在，先删除（防止重复创建）
    // if (clock_timer) {
    //     lv_timer_del(clock_timer);
    // }
    // clock_timer = lv_timer_create(clock_update_cb, 1000, ui);
}

void custom_ing_desktop(lv_ui *ui)
{

}
void custom_deinit_desktop(lv_ui *ui)
{
    // if (clock_timer) {
    //     lv_timer_del(clock_timer);
    // }
}

void setup_ui_desktop(lv_ui *ui)
{
    setup_bottom_layer();
    init_scr_del_flag(ui);
    init_keyboard(ui);
    setup_scr_desktop(ui);
    lv_screen_load(ui->desktop);
}

char status_str[][16] = {"disconnected", "connecting", "ok", "reconnected","error"};

void custom_init_status_list(lv_ui *ui)
{
    system_status_t *sys_status = get_system_status();
    lv_span_set_text(ui->status_list_spangroup_ble_span, status_str[sys_status->ble_status]);
    lv_span_set_text(ui->status_list_spangroup_mqtt_span, status_str[sys_status->mqtt_status]);
    lv_span_set_text(ui->status_list_spangroup_wifi_span, status_str[sys_status->wifi_status]);
}

void custom_ing_status_list(lv_ui *ui)
{

}
void custom_deinit_status_list(lv_ui *ui)
{

}
void setup_ui_status_list(lv_ui *ui)
{
    setup_bottom_layer();
    init_scr_del_flag(ui);
    init_keyboard(ui);
    setup_scr_status_list(ui);
    lv_screen_load(ui->status_list);
}