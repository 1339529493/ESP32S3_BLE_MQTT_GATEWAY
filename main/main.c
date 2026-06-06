#include <stdio.h>
#include "nvs_flash.h"
#include "esp_log.h"

#include "wifi.h"
#include "mqtts.h"
#include "ble.h"
#include "ui.h"
#include "key_scan.h"

static const char *TAG = "MAIN";

extern void http_test(void);
void app_main(void)
{
    esp_err_t ret;
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }
    status_mgr_init();
    lv_lcd_init();
    gateway_event_bus_init();
    wifi_sta_init();

    // 创建任务，可以将队列句柄作为参数传递，这里简化为全局变量演示
    xTaskCreatePinnedToCore(ui_task, "ui_task", 8192, NULL, 2, NULL,1);
    xTaskCreatePinnedToCore(key_scan_task, "key_scan", 2048, NULL, 3, NULL, 0);
    xTaskCreatePinnedToCore(ble_task, "ble_task", 4096, NULL, 5, NULL, 0);
    xTaskCreatePinnedToCore(mqtt_task, "mqtt_task", 8192, NULL, 5, NULL, 0);
    // http_test();
    while(1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}