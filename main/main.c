#include <stdio.h>
#include "nvs_flash.h"

#include "lvgl_lcd.h"
#include "wifi.h"
#include "mqtts.h"
#include "ble.h"
#include "ui.h"
#include "key_scan.h"
#include "ota.h"

void sys_monitor_task()
{
    uint32_t free_heap = esp_get_free_heap_size();
    uint32_t min_free_heap = esp_get_minimum_free_heap_size();
    
    // 计算已用内存百分比 (假设总堆约为 300KB-500KB，具体视芯片型号而定)
    // ESP32-WROOM 通常约有 300KB+ 可用堆
    LOGI("SYS_MON", "=== Memory Status ===");
    LOGI("SYS_MON", "Free Heap: %lu bytes", free_heap);
    LOGI("SYS_MON", "Min Free Heap: %lu bytes", min_free_heap);
    
    // 警告：如果剩余内存低于阈值 (例如 20KB)
    if (free_heap < 20 * 1024) {
        LOGW("SYS_MON", "WARNING: Low Memory!");
    }

    // 可选：打印各个任务的栈水位线 (Stack High Water Mark)
    // 注意：这需要知道任务句柄，或者使用 vTaskList (需要 configUSE_TRACE_FACILITY=1)
    // 这里简单演示获取当前任务（即监控任务自己）的水位
    UBaseType_t stack_watermark = uxTaskGetStackHighWaterMark(NULL);
    LOGI("SYS_MON", "Monitor Task Stack Watermark: %u words (%u bytes)", 
         stack_watermark, stack_watermark * 4);
}

void app_main(void)
{
    esp_err_t ret;
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }
    
    ota_verify_and_check_integrity();
    // printf(" OTA test app2 sucesse\n");
    lv_lcd_init();
    gateway_event_bus_init();

    // 创建任务，可以将队列句柄作为参数传递，这里简化为全局变量演示
    xTaskCreatePinnedToCore(ui_task, "ui_task", 8192, NULL, 2, NULL,1);
    xTaskCreatePinnedToCore(key_scan_task, "key_scan", 2048, NULL, 3, NULL, 0);
    xTaskCreatePinnedToCore(wifi_task, "wifi_scan", 4096, NULL, 5, NULL, 0);
    xTaskCreatePinnedToCore(ble_task, "ble_task", 4096, NULL, 5, NULL, 0);
    xTaskCreatePinnedToCore(mqtt_task, "mqtt_task", 8192, NULL, 5, NULL, 0);

    while(1) {
        // sys_monitor_task();
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}