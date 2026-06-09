#include "ui.h"

/**
 * @brief 桌面状态入口：初始化 GUI
 */
void desktop_entry(void *obj)
{
    struct user_object *u_obj = (struct user_object *)obj;
    
    // 设置并加载主屏幕
    // setup_ui 会调用 setup_scr_screen 并加载屏幕
    setup_ui_desktop(u_obj->guider_ui);
    
    // 初始化自定义部分（如果有字体或额外样式）
    custom_init_desktop(u_obj->guider_ui);
    
    // 初始化事件绑定
    events_init(u_obj->guider_ui);
}

/**
 * @brief 桌面状态运行：处理 LVGL 任务
 * @note 如果你的主循环已经在其他地方调用了 lv_timer_handler()，这里可以留空或仅返回 SMF_CONTINUE
 */
enum smf_state_result desktop_run(void *obj)
{
    struct user_object *u_obj = (struct user_object *)obj;

    // 2. 检查是否有切换按键被按下
    if (CHECK_KEY_FLAG(u_obj, KEY_1))
    {
        // 3. 清除按键标志，防止重复触发
        CLR_KEY_FLAG(u_obj, KEY_1);
        smf_set_state(SMF_CTX(u_obj), &status_list);
        return SMF_EVENT_HANDLED; 
    }

    return SMF_EVENT_PROPAGATE;
}

/**
 * @brief 桌面状态退出：清理资源（可选）
 */
void desktop_exit(void *obj)
{
    struct user_object *u_obj = (struct user_object *)obj;
    
    // 1. 清理自定义资源（如删除时钟定时器）
    custom_deinit_desktop(u_obj->guider_ui);

    // 2. 清理 LVGL 屏幕对象
    lv_obj_clean(lv_screen_active());
}