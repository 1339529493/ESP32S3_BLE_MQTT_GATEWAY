#include "status_list.h"

void status_list_entry(void *obj)
{
    struct status_list_data *u_obj = (struct status_list_data *)obj;
    
    u_obj->guider_ui = &guider_ui;
    // 1. 初始化 GUI 结构体中的屏幕删除标志等
    init_scr_del_flag(u_obj->guider_ui);
    
    // 2. 设置并加载主屏幕
    // setup_ui 会调用 setup_scr_screen 并加载屏幕
    setup_ui_status_list(u_obj->guider_ui);
    
    // 3. 初始化自定义部分（如果有字体或额外样式）
    custom_init(u_obj->guider_ui);
    
    // 4. 初始化事件绑定
    events_init(u_obj->guider_ui);
}

enum smf_state_result status_list_run(void *obj)
{
    return 0;
}

void status_list_exit(void *obj)
{
    // 如果需要切换屏幕，可以在这里清理当前屏幕
    lv_obj_clean(lv_screen_active());
}