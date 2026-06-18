#include "ble.h"
#include "cjson.h"

#ifdef __LWIP_DEF_H__          // lwIP 的 def.h 会定义这个宏
    // 使用 lwIP 提供的函数
    #include <lwip/def.h>
#else
static inline uint16_t ntohs(uint16_t net_short) {
    return ((net_short >> 8) & 0x00FF) | 
           ((net_short << 8) & 0xFF00);
}
#endif

void ble_task(void *pvParameter)
{
    LOGI(GATTS_TAG, "BLE Task Started");
    ble_init(); // 初始化 BLE GATT Server

    gateway_event_t msg;
    while (1)
    {
        if (gateway_event_receive(MODULE_ID_BLE, &msg, portMAX_DELAY))
        {
            LOGI(GATTS_TAG, "BLE EVT : Received from %d, len: %d", msg.src_id, msg.data_len);
            ble_send_notify(msg.data, msg.data_len);
            gateway_event_free(&msg);
        }
    }
}
#define TAG_JSON 0x3031
#define TAG_HEX 0x3032
char ble_test_cmd[][200] = {
    "\x30\x31\x00\x40{\"cmd\":\"wifi_config\",\"ssid\":\"LinksField\",\"pwd\":\"linksfield2023\"}",
    "\x30\x31\x00\x5a{\"cmd\":\"ota_download\",\"url\":\"http://81.70.100.183/ota/esp_ble_mqtt_gateway_test_ota1.bin\"}",
    "\x30\x31\x00\x5a{\"cmd\":\"ota_download\",\"url\":\"http://81.70.100.183/ota/esp_ble_mqtt_gateway_test_ota2.bin\"}",
    "\x30\x31\x00\x16{\"cmd\":\"ota_rollback\"}"};
static void process_json_command(char *json_str)
{
    // 解析 JSON
    cJSON *root = cJSON_Parse(json_str);
    if (root == NULL)
    {
        LOGE(GATTS_TAG, "JSON Parse Error");
        return;
    }

    // 获取 cmd 字段
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
                    LOGE(GATTS_TAG, "BLE to MQTT Queue Full! cmd : wifi_config");
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
}

static uint8_t ble_buffer[1024];

typedef struct {
    uint16_t tag;
    uint16_t payload_len;   //当前接收数据长度
    uint8_t *data; 
    uint16_t data_len;      //数据总长度
} gw_ble_state_t;

static void gw_ble_states_data_reset(gw_ble_state_t *states)
{
    states->tag = 0;
    states->payload_len = 0;
    states->data = ble_buffer;
    states->data_len = 0;
}
void ble_assemble_and_parse(uint8_t *data, uint16_t len, gw_ble_state_t *states)
{
    // int len = strlen(ble_test_cmd[1] + 4) + 4;
    // uint8_t data[200] = {0};
    // memcpy(data, ble_test_cmd[1], len);

    LOGW(GATTS_TAG, "TLV Received: Tag=0x%04X, Len=%d, TotalPktLen=%d", states->tag, states->payload_len, states->data_len);
    if (states->tag == TAG_JSON) {
        LOGW(GATTS_TAG, "JSON Received: %s", states->data);
    }
    if (states->payload_len) {
        memcpy(states->data + states->payload_len, data, len);
        states->payload_len += len;
        if (states->payload_len >= states->data_len) {
            LOGD(GATTS_TAG, "TLV Received: Tag=0x%04X, Len=%d, TotalPktLen=%d", states->tag, states->payload_len, states->data_len);
            states->data[states->payload_len] = '\0';
        }
        else
        {
            return;
        }
    }
    else
    {
        // 检查最小长度 (Header 4 bytes)
        if (len < 4) {
            LOGE(GATTS_TAG, "Packet too short for TLV header: %d", len);
            return;
        }
        states->tag = ntohs(*(uint16_t *)(data));
        states->data_len = ntohs(*(uint16_t *)(data + 2));
        states->payload_len = len - 4;
        LOGD(GATTS_TAG, "TLV Received: Tag=0x%04X, Len=%d, TotalPktLen=%d", states->tag, states->payload_len, states->data_len);

        if (states->tag == TAG_JSON && states->data_len > states->payload_len) {
            LOGD(GATTS_TAG, "JSON 数据未收全，继续接收");
            memcpy(states->data, data + 4, states->payload_len);
            return;
        }
        memcpy(states->data, data + 4, states->payload_len);
        states->data[states->payload_len] = '\0';
    }
    switch (states->tag) {
        case TAG_JSON:
            LOGD(GATTS_TAG, "data len : %d, JSON: %s", states->data_len, states->data);
            process_json_command((char *)(states->data));
            gw_ble_states_data_reset(states);
            break;
        default:
            LOGW(GATTS_TAG, "Unknown TLV Tag: 0x%04X", states->tag);
            gw_ble_states_data_reset(states);
            break;
    }
}

void gateway_control_channel_hander(esp_gatts_cb_event_t event, esp_ble_gatts_cb_param_t *param)
{
    static gw_ble_state_t gateway_ble_states;
    switch (event) {
        case ESP_GATTS_WRITE_EVT:
            ble_assemble_and_parse(param->write.value, param->write.len, &gateway_ble_states);
            break;
        case ESP_GATTS_CONNECT_EVT:
            gw_ble_states_data_reset(&gateway_ble_states);
            break;
        default:
            break;
    }
}