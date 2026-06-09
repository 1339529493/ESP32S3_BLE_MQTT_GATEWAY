#include "ui.h"

char status_str[16];
void status_list_entry(void *obj)
{
    struct user_object *u_obj = (struct user_object *)obj;

    setup_ui_status_list(u_obj->guider_ui);

    // 3. 初始化自定义部分（如果有字体或额外样式）
    custom_init_status_list(u_obj->guider_ui);

    // 4. 初始化事件绑定
    events_init(u_obj->guider_ui);
}

enum smf_state_result status_list_run(void *obj)
{
    struct user_object *u_obj = (struct user_object *)obj;

    // 2. 检查是否有切换按键被按下
    if (CHECK_KEY_FLAG(u_obj, KEY_2))
    {
        // 3. 清除按键标志，防止重复触发
        CLR_KEY_FLAG(u_obj, KEY_2);
        smf_set_state(SMF_CTX(u_obj), &status_list);
        return SMF_EVENT_HANDLED;
    }

    return SMF_EVENT_PROPAGATE;
}

void status_list_exit(void *obj)
{
    struct user_object *u_obj = (struct user_object *)obj;

    // 1. 清理自定义资源（如删除时钟定时器）
    custom_deinit_status_list(u_obj->guider_ui);

    // 2. 清理 LVGL 屏幕对象
    lv_obj_clean(lv_screen_active());
}

char *get_system_status_string(conn_status_t status)
{
    switch (status)
    {
    case STATUS_CONNECTED:
        strcpy(status_str, "connected");
        break;
    case STATUS_CONNECTING:
        strcpy(status_str, "connecting");
        break;
    case STATUS_DISCONNECTED:
        strcpy(status_str, "disconnected");
        break;
    case STATUS_ERROR:
        strcpy(status_str, "error");
    default:
        break;
    }
    return status_str;
}
void lv_update_connection_icons(void *obj, module_id_t target)
{
    lv_ui *ui = ((struct user_object *)obj)->guider_ui;
    system_status_t *sys_status = get_system_status();
    switch (target)
    {
    case MODULE_ID_BLE:
        lv_span_set_text(ui->status_list_spangroup_ble_span, get_system_status_string(sys_status->ble_status));
        break;
    case MODULE_ID_MQTT:
        lv_span_set_text(ui->status_list_spangroup_mqtt_span, get_system_status_string(sys_status->mqtt_status));
        /* code */
        break;
    case MODULE_ID_WIFI:
        lv_span_set_text(ui->status_list_spangroup_wifi_span, get_system_status_string(sys_status->wifi_status));
        /* code */
        break;

    default:
        break;
    }
}