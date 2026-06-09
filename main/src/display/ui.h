#ifndef __UI_H
#define __UI_H

#include <stdio.h>
#include "gateway_cmd.h"
#include "smf.h"
#include "gw_log.h"

#include "gui_guider.h"
#include "events_init.h"
#include "custom.h"

struct user_object {
    struct smf_ctx ctx;      /* 状态机上下文 */
    lv_ui *guider_ui;        /* GUI 状态指针 */
    uint8_t key_flag;        /* 按键标志位 */
};

/* 设置标志位 */
#define SET_KEY_FLAG(obj,key)  (obj->key_flag |= (1 << (key)))
/* 清除标志位 (推荐命名: CLR_KEY_FLAG) */
#define CLR_KEY_FLAG(obj,key)  (obj->key_flag &= ~(1 << (key)))
/* 检查标志位是否被设置 (返回非0表示已设置) */
#define CHECK_KEY_FLAG(obj,key) ((obj->key_flag >> (key)) & 1)

/* --- 状态枚举 --- */
enum menu_state {
    MENU_MAIN,
    MENU_SETTINGS,
    MENU_ABOUT,
    MENU_COUNT
};

/* --- 状态表 --- */
extern const struct smf_state desktop;
extern const struct smf_state status_list;
extern const struct smf_state menu;
extern const struct smf_state menu_list[MENU_COUNT];
#define STATE(id) (&menu_list[id])

void ui_task(void *pvParameter);

void desktop_entry(void *obj);
enum smf_state_result desktop_run(void *obj);
void desktop_exit(void *obj);
void menu_entry(void *obj);
enum smf_state_result menu_run(void *obj);
void menu_exit(void *obj);
void status_list_entry(void *obj);
enum smf_state_result status_list_run(void *obj);
void status_list_exit(void *obj);
void lv_update_connection_icons(void *obj, module_id_t target);

#endif