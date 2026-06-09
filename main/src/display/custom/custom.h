/*
* Copyright 2024 NXP
* NXP Proprietary. This software is owned or controlled by NXP and may only be used strictly in
* accordance with the applicable license terms. By expressly accepting such terms or by downloading, installing,
* activating and/or otherwise using the software, you are agreeing that you have read, and that you agree to
* comply with and are bound by, such license terms.  If you do not agree to be bound by the applicable license
* terms, then you may not retain, install, activate or otherwise use the software.
*/

#ifndef __CUSTOM_H_
#define __CUSTOM_H_
#ifdef __cplusplus
extern "C" {
#endif

#include "gui_guider.h"

extern lv_ui guider_ui;

void custom_init_desktop(lv_ui *ui);
void custom_ing_desktop(lv_ui *ui);
void custom_deinit_desktop(lv_ui *ui);
void setup_ui_desktop(lv_ui *ui);
void custom_init_status_list(lv_ui *ui);
void custom_ing_status_list(lv_ui *ui);
void custom_deinit_status_list(lv_ui *ui);
void setup_ui_status_list(lv_ui *ui);

#ifdef __cplusplus
}
#endif
#endif /* EVENT_CB_H_ */
