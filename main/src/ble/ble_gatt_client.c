#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_gattc_api.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "esp_gatt_common_api.h"

#include "sdkconfig.h"
#include "gw_log.h"

#define GATTC_TAG "GATTC_CLIENT"
static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);
static void gattc_profile_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param);
// ==========================================================
// 1. UUID 定义 (必须与服务端 ble_gatt_server.c 保持一致)
// ==========================================================

// Service UUID: 81416d20-c9cd-492e-9d8b-6eeccf8a5622
static const esp_bt_uuid_t remote_service_uuid = {
    .len = ESP_UUID_LEN_128,
    .uuid = {
        .uuid128 = {0x81,0x41,0x6d,0x20,0xc9,0xcd,0x49,0x2e,0x9d,0x8b,0x6e,0xec,0xcf,0x8a,0x56,0x22}
    }
};

// RX Characteristic UUID (Client Write): 81416d21-c9cd-492e-9d8b-6eeccf8a5622
static const esp_bt_uuid_t remote_char_rx_uuid = {
    .len = ESP_UUID_LEN_128,
    .uuid = {
        .uuid128 = {0x81,0x41,0x6d,0x21,0xc9,0xcd,0x49,0x2e,0x9d,0x8b,0x6e,0xec,0xcf,0x8a,0x56,0x22}
    }
};

// TX Characteristic UUID (Client Notify): 81416d22-c9cd-492e-9d8b-6eeccf8a5622
static const esp_bt_uuid_t remote_char_tx_uuid = {
    .len = ESP_UUID_LEN_128,
    .uuid = {
        .uuid128 = {0x81,0x41,0x6d,0x22,0xc9,0xcd,0x49,0x2e,0x9d,0x8b,0x6e,0xec,0xcf,0x8a,0x56,0x22}
    }
};

// CCCD UUID (Standard): 00002902-0000-1000-8000-00805F9B34FB
static const uint16_t char_cccd_uuid = 0x2902;

// ==========================================================
// 2. 全局变量与状态管理
// ==========================================================

static esp_gattc_char_elem_t *char_elem_result   = NULL;
static esp_gattc_descr_elem_t *descr_elem_result = NULL;

// 保存发现的句柄
static uint16_t conn_id = 0xFFFF;
static uint16_t srv_start_handle = 0;
static uint16_t srv_end_handle = 0;
static uint16_t char_rx_handle = 0;
static uint16_t char_tx_handle = 0;
static uint16_t char_tx_cccd_handle = 0;

// 目标设备名称 (根据服务端代码 "ESP_GATEWAY")
static const char *target_device_name = "ESP_GATEWAY";
static esp_bd_addr_t remote_bda = {0};
static bool connect_done = false;
static bool get_service_done = false;
static bool get_char_done = false;

// ==========================================================
// 5. 初始化与辅助函数
// ==========================================================

// 扫描参数配置
static esp_ble_scan_params_t ble_scan_params = {
    .scan_type              = BLE_SCAN_TYPE_ACTIVE,
    .own_addr_type          = BLE_ADDR_TYPE_PUBLIC,
    .scan_filter_policy     = BLE_SCAN_FILTER_ALLOW_ALL,
    .scan_interval          = 0x50,
    .scan_window            = 0x30,
    .scan_duplicate         = BLE_SCAN_DUPLICATE_DISABLE
};

enum {
    PROFILE_APP_ID,
    PROFILE_NUM
};

struct gattc_profile_inst {
    esp_gattc_cb_t gattc_cb;
    uint16_t gattc_if;
    uint16_t app_id;
    uint16_t conn_id;
};

static struct gattc_profile_inst gl_profile_tab[PROFILE_NUM] = {
    [PROFILE_APP_ID] = {
        .gattc_cb = gattc_profile_event_handler,
        .gattc_if = ESP_GATT_IF_NONE,
    },
};

static void gattc_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param)
{
    if (event == ESP_GATTC_REG_EVT) {
        if (param->reg.status == ESP_GATT_OK) {
            gl_profile_tab[param->reg.app_id].gattc_if = gattc_if;
        } else {
            LOGI(GATTC_TAG, "Reg app failed, app_id %04x, status %d", param->reg.app_id, param->reg.status);
            return;
        }
    }

    for (int idx = 0; idx < PROFILE_NUM; idx++) {
        if (gattc_if == ESP_GATT_IF_NONE || gattc_if == gl_profile_tab[idx].gattc_if) {
            if (gl_profile_tab[idx].gattc_cb) {
                gl_profile_tab[idx].gattc_cb(event, gattc_if, param);
            }
        }
    }
}

// ==========================================================
// 3. GAP 事件处理 (扫描与连接)
// ==========================================================

static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    switch (event) {
    case ESP_GAP_BLE_SCAN_RESULT_EVT: {
        esp_ble_gap_cb_param_t *scan_result = (esp_ble_gap_cb_param_t *)param;
        if (scan_result->scan_rst.search_evt == ESP_GAP_SEARCH_INQ_RES_EVT) {
            // 检查广播数据中是否包含目标名称或服务UUID
            // 这里简单通过名称判断，实际项目中建议解析 Adv Data 中的 UUID
            if (scan_result->scan_rst.adv_data_len > 0) {
                 // 简单的名称匹配逻辑可能需要解析 AD Structure，此处简化为已知地址或后续连接尝试
                 // 为了演示，我们假设扫描到任意设备都打印，实际应过滤
                 LOGI(GATTC_TAG, "Found device: %s", scan_result->scan_rst.bda);
                 
                 // 如果知道具体 MAC 地址，可以直接在这里连接
                 // if (memcmp(scan_result->scan_rst.bda, target_bda, 6) == 0) ...
            }
        } else if (scan_result->scan_rst.search_evt == ESP_GAP_SEARCH_INQ_CMPL_EVT) {
            LOGI(GATTC_TAG, "Scan complete");
            // 可以在这里发起连接，如果之前记录了目标设备的 BDA
            if (!connect_done && remote_bda[0] != 0) {
                 esp_ble_gap_stop_scanning();
                 esp_ble_gattc_open(gl_profile_tab[0].gattc_if, remote_bda, BLE_ADDR_TYPE_PUBLIC, true);
            }
        }
        break;
    }
    case ESP_GAP_BLE_SCAN_START_COMPLETE_EVT:
        if (param->scan_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
            LOGE(GATTC_TAG, "Scan start failed");
        } else {
            LOGI(GATTC_TAG, "Scan started");
        }
        break;
    default:
        break;
    }
}

// ==========================================================
// 4. GATTC 事件处理 (核心业务逻辑)
// ==========================================================

static void gattc_profile_event_handler(esp_gattc_cb_event_t event, esp_gatt_if_t gattc_if, esp_ble_gattc_cb_param_t *param)
{
    esp_ble_gattc_cb_param_t *p_data = (esp_ble_gattc_cb_param_t *)param;

    switch (event) {
    case ESP_GATTC_REG_EVT:
        LOGI(GATTC_TAG, "REG_EVT, status %d", param->reg.status);
        // 注册成功后开始扫描
        esp_ble_gap_set_scan_params(&ble_scan_params); // 需定义 scan_params
        esp_ble_gap_start_scanning(30); // 扫描30秒
        break;

    case ESP_GATTC_OPEN_EVT:
        if (param->open.status != ESP_GATT_OK){
            LOGE(GATTC_TAG, "Open failed, status %d", p_data->open.status);
            break;
        }
        conn_id = p_data->open.conn_id;
        LOGI(GATTC_TAG, "Connected to server, conn_id %d", conn_id);
        connect_done = true;
        
        // 发现服务
        esp_ble_gattc_search_service(gattc_if, conn_id, &remote_service_uuid);
        break;

    case ESP_GATTC_SEARCH_RES_EVT:
        if (p_data->search_res.srvc_id.uuid.len == ESP_UUID_LEN_128 && 
            memcmp(p_data->search_res.srvc_id.uuid.uuid.uuid128, remote_service_uuid.uuid.uuid128, 16) == 0) {
            srv_start_handle = p_data->search_res.start_handle;
            srv_end_handle = p_data->search_res.end_handle;
            LOGI(GATTC_TAG, "Service found, start_handle %d, end_handle %d", srv_start_handle, srv_end_handle);
            get_service_done = true;
        }
        break;

    case ESP_GATTC_SEARCH_CMPL_EVT:
        if (p_data->search_cmpl.status != ESP_GATT_OK){
            LOGE(GATTC_TAG, "Search service failed, error status = %x", p_data->search_cmpl.status);
            break;
        }
        if(get_service_done){
            // 获取特征值
            uint16_t count = 0;
            esp_gatt_status_t status = esp_ble_gattc_get_attr_count(gattc_if, conn_id, ESP_GATT_DB_CHARACTERISTIC,
                                                                    srv_start_handle, srv_end_handle, 0, &count);
            if (status != ESP_GATT_OK) {
                LOGE(GATTC_TAG, "Get attr count failed");
                break;
            }
            if (count > 0) {
                char_elem_result = (esp_gattc_char_elem_t *)malloc(sizeof(esp_gattc_char_elem_t) * count);
                if (!char_elem_result) {
                    LOGE(GATTC_TAG, "Malloc failed");
                    break;
                }
                status = esp_ble_gattc_get_all_char(gattc_if, conn_id, srv_start_handle, srv_end_handle, char_elem_result, &count, 0);
                if (status != ESP_GATT_OK) {
                    free(char_elem_result);
                    LOGE(GATTC_TAG, "Get all char failed");
                    break;
                }

                // 遍历特征，找到 RX 和 TX
                for (int i = 0; i < count; i++) {
                    // 检查 RX (Write)
                    if (char_elem_result[i].uuid.len == ESP_UUID_LEN_128 && 
                        memcmp(char_elem_result[i].uuid.uuid.uuid128, remote_char_rx_uuid.uuid.uuid128, 16) == 0) {
                        char_rx_handle = char_elem_result[i].char_handle;
                        LOGI(GATTC_TAG, "RX Char found, handle %d", char_rx_handle);
                    }
                    // 检查 TX (Notify)
                    if (char_elem_result[i].uuid.len == ESP_UUID_LEN_128 && 
                        memcmp(char_elem_result[i].uuid.uuid.uuid128, remote_char_tx_uuid.uuid.uuid128, 16) == 0) {
                        char_tx_handle = char_elem_result[i].char_handle;
                        LOGI(GATTC_TAG, "TX Char found, handle %d", char_tx_handle);
                        
                        // 查找 TX 的 CCCD 描述符
                        uint16_t descr_count = 0;
                        esp_ble_gattc_get_attr_count(gattc_if, conn_id, ESP_GATT_DB_DESCRIPTOR,
                                                     char_elem_result[i].char_handle, char_elem_result[i].char_handle + 1, 0, &descr_count);
                        if (descr_count > 0) {
                            descr_elem_result = (esp_gattc_descr_elem_t *)malloc(sizeof(esp_gattc_descr_elem_t) * descr_count);
                            if (descr_elem_result) {
                                esp_ble_gattc_get_all_descr(gattc_if, conn_id, char_elem_result[i].char_handle, descr_elem_result, &descr_count, 0);
                                for(int j=0; j<descr_count; j++){
                                    if(descr_elem_result[j].uuid.len == ESP_UUID_LEN_16 && descr_elem_result[j].uuid.uuid.uuid16 == char_cccd_uuid){
                                        char_tx_cccd_handle = descr_elem_result[j].handle;
                                        LOGI(GATTC_TAG, "TX CCCD Handle found: %d", char_tx_cccd_handle);
                                    }
                                }
                                free(descr_elem_result);
                            }
                        }
                    }
                }
                free(char_elem_result);
                get_char_done = true;

                // 如果找到了 TX 特征和 CCCD，启用通知
                if (char_tx_handle != 0 && char_tx_cccd_handle != 0) {
                    uint8_t notify_en[] = {0x01, 0x00}; // Enable Notification
                    esp_ble_gattc_write_char_descr(gattc_if, conn_id, char_tx_cccd_handle, sizeof(notify_en), notify_en, ESP_GATT_WRITE_TYPE_RSP, ESP_GATT_AUTH_REQ_NONE);
                    LOGI(GATTC_TAG, "Enabled Notification for TX Char");
                }
            }
        }
        break;

    case ESP_GATTC_WRITE_DESCR_EVT:
        if (p_data->write.status != ESP_GATT_OK) {
            LOGE(GATTC_TAG, "Write descriptor failed, status %d", p_data->write.status);
        } else {
            LOGI(GATTC_TAG, "Write descriptor success");
            // 此时可以开始发送数据或等待通知
        }
        break;

    case ESP_GATTC_WRITE_CHAR_EVT:
        if (p_data->write.status != ESP_GATT_OK) {
            LOGE(GATTC_TAG, "Write char failed, status %d", p_data->write.status);
        } else {
            LOGI(GATTC_TAG, "Write char success");
        }
        break;

    case ESP_GATTC_NOTIFY_EVT:
        if (p_data->notify.handle == char_tx_handle) {
            LOGI(GATTC_TAG, "Received Notify, len %d", p_data->notify.value_len);
            // 处理接收到的数据
            // esp_log_buffer_hex(GATTC_TAG, p_data->notify.value, p_data->notify.value_len);
            
            // 示例：将数据转发到其他队列或处理
            // process_received_data(p_data->notify.value, p_data->notify.value_len);
        }
        break;

    case ESP_GATTC_DISCONNECT_EVT:
        LOGI(GATTC_TAG, "Disconnected, reason %d", p_data->disconnect.reason);
        connect_done = false;
        get_service_done = false;
        get_char_done = false;
        conn_id = 0xFFFF;
        // 可选：重新扫描或重连
        esp_ble_gap_start_scanning(30);
        break;

    default:
        break;
    }
}

void ble_client_init(void)
{
    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_bt_controller_init(&bt_cfg));
    ESP_ERROR_CHECK(esp_bt_controller_enable(ESP_BT_MODE_BLE));
    ESP_ERROR_CHECK(esp_bluedroid_init());
    ESP_ERROR_CHECK(esp_bluedroid_enable());

    ESP_ERROR_CHECK(esp_ble_gattc_register_callback(gattc_event_handler));
    ESP_ERROR_CHECK(esp_ble_gap_register_callback(gap_event_handler));
    
    // 注册 App
    ESP_ERROR_CHECK(esp_ble_gattc_app_register(PROFILE_APP_ID));
    
    // 设置本地 MTU (可选，协商最大传输单元)
    esp_ble_gatt_set_local_mtu(500);
}

// 示例：发送数据到服务端 RX 特征
void ble_client_send_data(uint8_t *data, uint16_t len)
{
    if (conn_id == 0xFFFF || char_rx_handle == 0) {
        LOGW(GATTC_TAG, "Not connected or RX handle not found");
        return;
    }
    
    // 写入数据到服务端的 RX Characteristic
    esp_err_t ret = esp_ble_gattc_write_char(
        gl_profile_tab[PROFILE_APP_ID].gattc_if,
        conn_id,
        char_rx_handle,
        len,
        data,
        ESP_GATT_WRITE_TYPE_NO_RSP, // 或者 ESP_GATT_WRITE_TYPE_RSP
        ESP_GATT_AUTH_REQ_NONE
    );

    if (ret) {
        LOGE(GATTC_TAG, "Send data failed, error code = %x", ret);
    }
}