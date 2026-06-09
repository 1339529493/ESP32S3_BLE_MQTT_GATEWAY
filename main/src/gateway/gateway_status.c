#include "gateway_status.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "gw_log.h"
#include "gateway_cmd.h"
#include "gateway_event.h"
// 包含你的 UI 头文件


static const char *TAG = "STATUS_MGR";
static system_status_t g_sys_status = {0};
static SemaphoreHandle_t status_mutex = NULL;

// 初始化互斥锁
void status_mgr_init(void) {
    status_mutex = xSemaphoreCreateMutex();
}

system_status_t* get_system_status(void) {
    return &g_sys_status;
}

// 通用更新函数
static void update_status(conn_status_t *target, conn_status_t new_status, module_id_t module_name) {
    if (xSemaphoreTake(status_mutex, pdMS_TO_TICKS(100)) == pdTRUE) {
        if (*target != new_status) {
            *target = new_status;
            LOGI(TAG, "%s status changed to %d", module_name, new_status);
            gateway_event_t msg;
            GATEWAY_EVENT_INIT_CMD(&msg, module_name, MODULE_ID_UI, CMD_UI_UPDATE_STATUS, 0);
            if (gateway_event_send(MODULE_ID_UI, &msg, pdMS_TO_TICKS(100)) != pdTRUE) 
            {
                LOGE(TAG, "Device status to UI Queue Full! status : %d",new_status);
            }
        }
        xSemaphoreGive(status_mutex);
    }
}

void update_wifi_status(conn_status_t status) {
    update_status(&g_sys_status.wifi_status, status, MODULE_ID_WIFI);
}

void update_ble_status(conn_status_t status) {
    update_status(&g_sys_status.ble_status, status, MODULE_ID_BLE);
}

void update_mqtt_status(conn_status_t status) {
    update_status(&g_sys_status.mqtt_status, status, MODULE_ID_MQTT);
}
