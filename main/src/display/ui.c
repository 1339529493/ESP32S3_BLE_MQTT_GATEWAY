#include <stdio.h>
#include "ui.h"

#define UI_TAG "UI"
struct user_object ui_obj;

const struct smf_state desktop = SMF_CREATE_STATE(desktop_entry, desktop_run, desktop_exit, NULL, NULL);
const struct smf_state status_list = SMF_CREATE_STATE(status_list_entry, status_list_run, status_list_exit, NULL, NULL);
const struct smf_state menu = SMF_CREATE_STATE(menu_entry, menu_run, menu_exit, NULL, NULL);
const struct smf_state menu_list[MENU_COUNT] = {
    [MENU_MAIN] = SMF_CREATE_STATE(NULL, NULL, NULL, NULL, NULL),
    [MENU_SETTINGS] = SMF_CREATE_STATE(NULL, NULL, NULL, NULL, NULL),
    [MENU_ABOUT] = SMF_CREATE_STATE(NULL, NULL, NULL, NULL, NULL),
};
void ui_task(void *pvParameter)
{
    struct user_object *obj = &ui_obj;
    int32_t ret;
    gateway_event_t msg;
    // 初始化 GUI 指针
    obj->guider_ui = &guider_ui;
    smf_set_initial(SMF_CTX(obj), &status_list);

    while(1) {
        lv_timer_handler();
        ret = smf_run_state(SMF_CTX(obj));
        if (ret) {
            printf("State Machine Terminated with code: %ld\n", ret);
            break;
        }

        if (gateway_event_receive(MODULE_ID_UI, &msg, 0) == pdTRUE) {
            LOGD(UI_TAG, "Received from %d, len: %d", msg.src_id, msg.data_len);
            switch (msg.cmd_id) {                
                case CMD_UI_NONE:
                    if (msg.src_id == MODULE_ID_KEY)
                    {
                        SET_KEY_FLAG(obj,msg.short_msg); 
                    }
                    break;
                case CMD_UI_UPDATE_STATUS:
                    lv_update_connection_icons(obj, msg.src_id);
                    break;
                default:
                    break;
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10));         /* 短暂等待，让系统喘息 */
    }
}