#include <stdio.h>
#include "nvs_flash.h"
#include "esp_log.h"

#include "wifi.h"
#include "mqtts.h"
#include "ble.h"
#include "gateway_msg.h" // 引入新定义
#include "ui.h"
static const char *TAG = "MAIN";

// 声明全局队列句柄，或者通过参数传递
QueueHandle_t ble_to_mqtt_q;
QueueHandle_t mqtt_to_ble_q;

void mqtt_task(void *pvParameter)
{
    ESP_LOGI(TAG, "MQTT Task Started");
    // 1. 初始化 MQTT (内部会连接并订阅)
    mqtts_init(); 
    
    gateway_msg_t msg;
    while(1) {
        // 2. 等待来自 BLE 的数据并上报
        if (xQueueReceive(ble_to_mqtt_q, &msg, portMAX_DELAY) == pdTRUE) {
            ESP_LOGI(TAG, "Received from BLE, len: %d", msg.len);
            
            // 【预留解析点】：如果需要在此处解析 BLE 数据再上报
            // parse_ble_data_and_publish(&msg); 
            // 直接发布原始数据或处理后数据
            mqtts_publish((char *)msg.payload, msg.len, 0, 0);
        }
    }
}

void ble_task(void *pvParameter)
{
    ESP_LOGI(TAG, "BLE Task Started");
    ble_init(); // 初始化 BLE GATT Server

    gateway_msg_t msg;
    while(1) {
        // 1. 等待来自 MQTT 的下发指令
        if (xQueueReceive(mqtt_to_ble_q, &msg, portMAX_DELAY) == pdTRUE) {
            ESP_LOGI(TAG, "Received from MQTT, sending to BLE Notify");
            // 调用 BLE 接口发送 Notify 给手机
            ble_send_notify(msg.payload, msg.len);
        }
        
        // 注意：BLE 接收数据是在 GATT 回调中发生的，需要在回调中发送队列
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

void app_main(void)
{
    esp_err_t ret;
    ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ESP_ERROR_CHECK(nvs_flash_init());
    }

    wifi_sta_init();

    // 创建队列
    ble_to_mqtt_q = xQueueCreate(10, sizeof(gateway_msg_t));
    mqtt_to_ble_q = xQueueCreate(10, sizeof(gateway_msg_t));

    // 创建任务，可以将队列句柄作为参数传递，这里简化为全局变量演示
    xTaskCreatePinnedToCore(ui_task, "ui_task", 8192, NULL, 2, NULL,1);
    vTaskDelay(pdMS_TO_TICKS(1000));
    xTaskCreatePinnedToCore(ble_task, "ble_task", 4096, NULL, 5, NULL, 0);
    xTaskCreatePinnedToCore(mqtt_task, "mqtt_task", 8192, NULL, 5, NULL, 0);
    while(1) {
        vTaskDelay(pdMS_TO_TICKS(1000));
    }
}