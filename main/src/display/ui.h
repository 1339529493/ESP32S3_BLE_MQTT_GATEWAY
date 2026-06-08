#ifndef __UI_H
#define __UI_H

#include <stdio.h>
#include "lvgl_lcd.h"
#include "smf.h"
#include "gui_guider.h"

struct user_object {
    struct smf_ctx ctx;      /* 状态机上下文 */
    lv_ui *guider_ui; // 存放 GUI 状态
    int counter[100];             /* 用户自定义数据，用于演示状态间数据共享 */
};

/* --- 状态枚举 --- */
enum menu_state {
    MENU_MAIN,
    MENU_SETTINGS,
    MENU_ABOUT,
    MENU_COUNT
};

/* --- 状态表 --- */
extern const struct smf_state desktop;
extern const struct smf_state menu;
extern const struct smf_state menu_list[MENU_COUNT];
#define STATE(id) (&menu_list[id])

void ui_task(void *pvParameter);

#endif