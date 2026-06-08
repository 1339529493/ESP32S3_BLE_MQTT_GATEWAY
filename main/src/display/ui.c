#include <stdio.h>

#include "gw_log.h"
#include "ui.h"
#include "desktop.h"
#include "menu.h"
#include "status_list.h"

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
    struct user_object obj = {0};
    int32_t ret;
    // 初始化 GUI 指针
    obj.guider_ui = &guider_ui;
    smf_set_initial(SMF_CTX(&obj), &desktop);

    while(1) {
        lv_timer_handler();
        ret = smf_run_state(SMF_CTX(&obj));
        if (ret) {
            printf("State Machine Terminated with code: %ld\n", ret);
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(5));         /* 短暂等待，让系统喘息 */
    }
}