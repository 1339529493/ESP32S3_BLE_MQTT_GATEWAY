#ifndef __MENU_H
#define __MENU_H

#include "smf.h"
// 引入 GUI Guider 生成的头文件
#include "desktop/generated/gui_guider.h"
#include "desktop/generated/events_init.h"
#include "desktop/custom/custom.h"

// 定义用户对象结构，用于在状态机中传递数据
struct menu_data {
    struct smf_ctx ctx;
    lv_ui *guider_ui; // 存放 GUI 状态
};

void menu_entry(void *obj);
enum smf_state_result menu_run(void *obj);
void menu_exit(void *obj);

#endif