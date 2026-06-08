#ifndef DESKTOP_H
#define DESKTOP_H

#include "smf.h"
// 引入 GUI Guider 生成的头文件
#include "gui_guider.h"
#include "events_init.h"
#include "custom.h"

// 定义用户对象结构，用于在状态机中传递数据
struct desktop_data {
    struct smf_ctx ctx;
    lv_ui *guider_ui; // 存放 GUI 状态
};

void desktop_entry(void *obj);
enum smf_state_result desktop_run(void *obj);
void desktop_exit(void *obj);

#endif