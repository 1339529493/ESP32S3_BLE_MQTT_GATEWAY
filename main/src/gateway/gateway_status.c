#include "gateway_status.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
// 包含你的 UI 头文件，假设是 desktop.h
#include "../display/desktop/desktop.h" 

static const char *TAG = "STATUS_MGR";
static system_status_t g_sys_status = {0};
static SemaphoreHandle_t status_mutex = NULL;

static void update_ui_icons(conn_status_t status, const char *module);

// 初始化互斥锁
void status_mgr_init(void) {
    status_mutex = xSemaphoreCreateMutex();
}

system_status_t* get_system_status(void) {
    return &g_sys_status;
}

// 通用更新函数
static void update_status(conn_status_t *target, conn_status_t new_status, const char *module_name) {
    if (xSemaphoreTake(status_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        if (*target != new_status) {
            *target = new_status;
            ESP_LOGI(TAG, "%s status changed to %d", module_name, new_status);
            
            // 【关键点】：在这里触发 UI 更新
            // 注意：LVGL 操作必须在 LVGL 任务上下文或使用 lvgl 锁
            // 不能太久以防其他状态更新失效导致状态不同步
            update_ui_icons(new_status, module_name); 
        }
        xSemaphoreGive(status_mutex);
    }
}

void update_wifi_status(conn_status_t status) {
    update_status(&g_sys_status.wifi_status, status, "WiFi");
}

void update_ble_status(conn_status_t status) {
    update_status(&g_sys_status.ble_status, status, "BLE");
}

void update_mqtt_status(conn_status_t status) {
    update_status(&g_sys_status.mqtt_status, status, "MQTT");
}

// 模拟 UI 更新函数 (需要在 ui.c 或 desktop.c 中实现具体逻辑)
extern void lv_update_connection_icons(system_status_t *status);

static void update_ui_icons(conn_status_t status, const char *module) {
    // 这里不能直接调用 LVGL API，除非你确定当前上下文允许
    // 更好的方式是发送一个事件给 UI 任务，或者使用 lvgl 互斥锁
    
    // 示例：如果使用 lvgl 锁
    /*
    lv_lock();
    if (strcmp(module, "WiFi") == 0) {
        if (status == STATUS_CONNECTED) lv_obj_set_style_bg_color(ui->img_wifi, lv_color_green(), 0);
        else if (status == STATUS_CONNECTING) lv_obj_set_style_bg_color(ui->img_wifi, lv_color_yellow(), 0);
        else lv_obj_set_style_bg_color(ui->img_wifi, lv_color_gray(), 0);
    }
    lv_unlock();
    */
   
    // 更推荐的方式：将状态变化放入一个队列，UI 任务从队列取并刷新
}
