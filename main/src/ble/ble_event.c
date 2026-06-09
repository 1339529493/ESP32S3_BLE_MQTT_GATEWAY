#include "ble.h"

void ble_task(void *pvParameter)
{
    LOGI(GATTC_TAG, "BLE Task Started");
    ble_init(); // 初始化 BLE GATT Server

    gateway_event_t msg;
    while(1) {
        // 1. 等待来自 MQTT 的下发指令
        if (gateway_event_receive(MODULE_ID_BLE, &msg, portMAX_DELAY)) {
            LOGI(GATTC_TAG, "Received from MQTT, sending to BLE Notify");
            // 调用 BLE 接口发送 Notify 给手机
            ble_send_notify(msg.data, msg.data_len);
            gateway_event_free(&msg);
        }
        
        // 注意：BLE 接收数据是在 GATT 回调中发生的，需要在回调中发送队列
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}