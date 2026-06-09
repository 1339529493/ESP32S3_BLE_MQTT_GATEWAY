#include "mqtts.h"

extern esp_mqtt_client_handle_t mqtt_client;
void mqtt_task(void *pvParameter)
{
    LOGI(MQTTS_TAG, "MQTT Task Started");
    // 1. 初始化 MQTT (内部会连接并订阅)
    mqtts_init(); 
    
    gateway_event_t msg;
    while(1) {
        // 2. 等待来自 BLE 的数据并上报
        if (gateway_event_receive(MODULE_ID_MQTT, &msg, portMAX_DELAY) == pdTRUE) {
            LOGI(MQTTS_TAG, "Received from BLE, len: %d", msg.data_len);
            
            switch (msg.cmd_id) {
                
                case CMD_WIFI_TO_MQTT_START:
                    LOGI(MQTTS_TAG, "Received WiFi START command");
                    // 启动 MQTT 客户端
                    // 如果已经启动，再次调用 start 通常会返回错误或被忽略，建议加状态判断
                    if (mqtt_client) {
                        esp_mqtt_client_start(mqtt_client);
                    }
                    break;

                case CMD_WIFI_TO_MQTT_STOP:
                    LOGI(MQTTS_TAG, "Received WiFi STOP command");
                    // 停止 MQTT 客户端
                    if (mqtt_client) {
                        esp_mqtt_client_disconnect(mqtt_client); // 先断开连接
                        esp_mqtt_client_stop(mqtt_client);       // 再停止任务
                    }
                    update_mqtt_status(STATUS_DISCONNECTED);
                    break;

                case CMD_BLE_TO_MQTT_PUBLISH:
                    // 原有的 BLE 数据上报逻辑
                    if (msg.data) {
                        mqtts_publish((char *)msg.data, msg.data_len, 0, 0);
                        gateway_event_free(&msg);
                    }
                    break;
                    
                default:
                    break;
            }
        }
    }
}