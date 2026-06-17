#include "ble.h"
#include "cjson.h"
void ble_task(void *pvParameter)
{
    LOGI(GATTS_TAG, "BLE Task Started");
    ble_init(); // 初始化 BLE GATT Server

    gateway_event_t msg;
    while (1)
    {
        // 1. 等待来自 MQTT 的下发指令
        if (gateway_event_receive(MODULE_ID_BLE, &msg, portMAX_DELAY))
        {
            LOGI(GATTS_TAG, "BLE EVT : Received from %d, len: %d", msg.src_id, msg.data_len);
            // 调用 BLE 接口发送 Notify 给手机
            ble_send_notify(msg.data, msg.data_len);
            gateway_event_free(&msg);
        }
    }
}

char ble_test_cmd[][200] = {
    "{\"cmd\":\"wifi_config\",\"ssid\":\"LinksField\",\"pwd\":\"linksfield2023\"}",
    "{\"cmd\":\"ota_download\",\"url\":\"http://81.70.100.183/ota/esp_ble_mqtt_gateway_test_ota1.bin\"}",
    "{\"cmd\":\"ota_rollback\"}"};
    
void gateway_control_channel_hander(esp_gatts_cb_event_t event, esp_ble_gatts_cb_param_t *param)
{
    // LOGD(GATTS_TAG, "Characteristic write, conn_id %d, trans_id %" PRIu32 ", handle %d", param->write.conn_id, param->write.trans_id, param->write.handle);
    // gateway_event_t msg;
    // // 发送队列，阻塞等待确保数据不丢失，或者使用 xQueueSendWithTimeout
    // if (gateway_event_create(&msg, MODULE_ID_BLE, MODULE_ID_MQTT, CMD_BLE_TO_MQTT_PUBLISH, param->write.value, param->write.len) != pdTRUE ||
    //     gateway_event_send(MODULE_ID_MQTT, &msg, pdMS_TO_TICKS(100)) != pdTRUE) {
    //     LOGE(GATTS_TAG, "BLE to MQTT Queue Full! data: %*.s",param->write.value, param->write.len);
    // }

    // char *json_str = malloc(param->write.len + 1);
    // memcpy(json_str, param->write.value, param->write.len);
    // json_str[param->write.len] = '\0';
    char *json_str = malloc(200);
    int len = strlen(ble_test_cmd[2]);
    memcpy(json_str, ble_test_cmd[2], len);
    json_str[len + 1] = '\0';

    // 2. 解析 JSON
    cJSON *root = cJSON_Parse(json_str);
    if (root == NULL)
    {
        LOGE(GATTS_TAG, "JSON Parse Error");
        free(json_str);
        return;
    }

    // 3. 获取 cmd 字段
    cJSON *cmd_obj = cJSON_GetObjectItemCaseSensitive(root, "cmd");
    if (cJSON_IsString(cmd_obj) && (cmd_obj->valuestring != NULL))
    {
        if (strcmp(cmd_obj->valuestring, "wifi_config") == 0)
        {
            // --- 处理配网 ---
            cJSON *ssid_obj = cJSON_GetObjectItemCaseSensitive(root, "ssid");
            cJSON *pwd_obj = cJSON_GetObjectItemCaseSensitive(root, "pwd");

            if (ssid_obj && pwd_obj)
            {
                LOGI(GATTS_TAG, "Provisioning: SSID=%s", ssid_obj->valuestring);
                gateway_event_t msg;
                wifi_provision_info_t info = {0};
                memcpy(info.ssid, ssid_obj->valuestring, strlen(ssid_obj->valuestring));
                memcpy(info.pwd, pwd_obj->valuestring, strlen(pwd_obj->valuestring));
                if (gateway_event_create(&msg, MODULE_ID_BLE, MODULE_ID_WIFI, CMD_BLE_TO_WIFI_PROVISION, &info, sizeof(wifi_provision_info_t)) != pdTRUE ||
                    gateway_event_send(MODULE_ID_WIFI, &msg, 0) != pdTRUE)
                {
                    // LOGE(GATTS_TAG, "BLE to MQTT Queue Full! data: %*.s",param->write.value, param->write.len);
                    LOGE(GATTS_TAG, "BLE to MQTT Queue Full! data:");
                    // 立即回复手机：收到指令
                    char *ack = "{\"cmd\":\"recv_ok\",\"msg\":\"message lose\"}";
                    ble_send_notify((uint8_t *)ack, strlen(ack));
                }
                else
                {
                    // 立即回复手机：收到指令
                    char *ack = "{\"cmd\":\"recv_ok\",\"msg\":\"provisioning started\"}";
                    ble_send_notify((uint8_t *)ack, strlen(ack));
                }
            }
        }
        else if (strcmp(cmd_obj->valuestring, "ota_download") == 0)
        {
            cJSON *url_obj = cJSON_GetObjectItemCaseSensitive(root, "url");
            if (url_obj && url_obj->valuestring)
            {
                gateway_event_t msg;
                // 发送事件给 OTA 模块
                gateway_event_create(&msg, MODULE_ID_BLE, MODULE_ID_WIFI, CMD_BLE_TO_WIFI_OTA_DOWNLOAD,
                                     (void *)url_obj->valuestring, strlen(url_obj->valuestring) + 1);
                gateway_event_send(MODULE_ID_WIFI, &msg, 0);

                char *ack = "{\"cmd\":\"recv_ok\",\"msg\":\"ota started\"}";
                ble_send_notify((uint8_t *)ack, strlen(ack));
            }
        }
        else if (strcmp(cmd_obj->valuestring, "ota_rollback") == 0)
        {
            gateway_event_t msg;
            // 创建事件
            GATEWAY_EVENT_INIT_CMD(&msg, MODULE_ID_BLE, MODULE_ID_WIFI, CMD_BLE_TO_WIFI_OTA_ROLLBACK, 0);
            gateway_event_send(MODULE_ID_WIFI, &msg, 0);
            char *ack = "{\"cmd\":\"recv_ok\",\"msg\":\"ota rollback\"}";
            ble_send_notify((uint8_t *)ack, strlen(ack));
        }
    }

    cJSON_Delete(root);
    free(json_str);
}