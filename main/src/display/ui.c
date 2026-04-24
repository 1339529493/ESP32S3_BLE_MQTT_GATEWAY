#include <stdio.h>

#include "gw_log.h"
#include "lvgl_lcd.h"
#include "demos/lv_demos.h"

void ui_task(void *pvParameter)
{
    lv_lcd_init();

    lv_obj_t * label = lv_label_create(lv_screen_active());
    lv_label_set_text(label, "Hello world");
    lv_obj_center(label);

    /* 在循环中执行 LVGL 相关任务 */
    while(1) {
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(5));         /* 短暂等待，让系统喘息 */
    }
}