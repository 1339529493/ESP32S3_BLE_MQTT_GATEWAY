#include "smf.h"
#include "ui.h"
#include "desktop.h"

#include "key_scan.h"
#include "ui.h"
/**
 * @brief 桌面状态入口：初始化 GUI
 */
void desktop_entry(void *obj)
{
    struct desktop_data *u_obj = (struct desktop_data *)obj;
    
    u_obj->guider_ui = &guider_ui;
    // // 1. 初始化 GUI 结构体中的屏幕删除标志等
    // init_scr_del_flag(u_obj->guider_ui);
    
    // 设置并加载主屏幕
    // setup_ui 会调用 setup_scr_screen 并加载屏幕
    setup_ui_desktop(u_obj->guider_ui);
    
    // 初始化自定义部分（如果有字体或额外样式）
    custom_init(u_obj->guider_ui);
    
    // 初始化事件绑定
    events_init(u_obj->guider_ui);
}

/**
 * @brief 桌面状态运行：处理 LVGL 任务
 * @note 如果你的主循环已经在其他地方调用了 lv_timer_handler()，这里可以留空或仅返回 SMF_CONTINUE
 */
enum smf_state_result desktop_run(void *obj)
{
    // 保持状态继续运行
    // key_scan_msg_t key_scan_msg;
    // if (xQueueReceive(key_scan_q,&key_scan_msg,0) == pdTRUE)
    // {
    //     smf_set_state(SMF_CTX(obj), &menu);
    // }
    return 0;
}

/**
 * @brief 桌面状态退出：清理资源（可选）
 */
void desktop_exit(void *obj)
{
    // 如果需要切换屏幕，可以在这里清理当前屏幕
    lv_obj_clean(lv_screen_active());
}