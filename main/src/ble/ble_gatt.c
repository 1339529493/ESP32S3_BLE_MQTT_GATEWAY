#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "nvs_flash.h"
#include "esp_bt.h"

#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_defs.h"
#include "esp_bt_main.h"
#include "esp_bt_device.h"
#include "esp_gatt_common_api.h"

#include "sdkconfig.h"
#include "gw_log.h"
#include "ble.h"
extern QueueHandle_t ble_to_mqtt_q;

#define GATTS_TAG "GATTS_GATEWAY"
static void gatts_profile_gateway_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
// uuid : 81416d20c9cd492e9d8b6eeccf8a5622
#define CHAR_DECLARATION_SIZE   (sizeof(uint8_t))
#define GW_BUFF_LEN  (512)
static const uint16_t primary_service_uuid = ESP_GATT_UUID_PRI_SERVICE;
static const uint16_t character_declaration_uuid = ESP_GATT_UUID_CHAR_DECLARE;
static const uint16_t character_tx_cccd_uuid = ESP_GATT_UUID_CHAR_CLIENT_CONFIG;
static const uint8_t tx_cccd_enable_notify[2] = {0x00, 0x00};
static const uint8_t gw_service_adv_uuid[] =  {0x81,0x41,0x6d,0x20,0xc9,0xcd,0x49,0x2e,0x9d,0x8b,0x6e,0xec,0xcf,0x8a,0x56,0x22};
static const uint8_t gw_character_rx_uuid[] = {0x81,0x41,0x6d,0x21,0xc9,0xcd,0x49,0x2e,0x9d,0x8b,0x6e,0xec,0xcf,0x8a,0x56,0x22};
static const uint8_t gw_character_tx_uuid[] = {0x81,0x41,0x6d,0x22,0xc9,0xcd,0x49,0x2e,0x9d,0x8b,0x6e,0xec,0xcf,0x8a,0x56,0x22};
static const uint8_t rx_function = ESP_GATT_CHAR_PROP_BIT_WRITE;
static const uint8_t tx_function = ESP_GATT_CHAR_PROP_BIT_NOTIFY;
static uint8_t gw_rx_val[GW_BUFF_LEN] = {0};
static uint8_t gw_tx_val[GW_BUFF_LEN] = {0};

static const esp_gatts_attr_db_t gw_gatt_db[GW_IDX_NB] =
{
    // 网关自定义服务声明
    [GW_SVC_IDX]                    =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&primary_service_uuid, ESP_GATT_PERM_READ,
      sizeof(gw_service_adv_uuid), sizeof(gw_service_adv_uuid), (uint8_t *)gw_service_adv_uuid}},

    // 读特征属性声明
    [GW_RX_CHAR_IDX]            =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
       CHAR_DECLARATION_SIZE, CHAR_DECLARATION_SIZE, (uint8_t *)&rx_function}},

    // 网关自定义读
    [GW_RX_VAL_IDX]             =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_128, (uint8_t *)gw_character_rx_uuid, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
      GW_BUFF_LEN,0, gw_rx_val}},

    // 写特征属性声明
    [GW_TX_CHAR_IDX]        =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_declaration_uuid, ESP_GATT_PERM_READ,
      CHAR_DECLARATION_SIZE,CHAR_DECLARATION_SIZE, (uint8_t *)&tx_function}},

    // 网关自定义写
    [GW_TX_VAL_IDX]  =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_128, (uint8_t *)gw_character_tx_uuid, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
      GW_BUFF_LEN,0, gw_tx_val}},

    // 写特征描述符打开notify
    [GW_TX_CCCD_IDX]   =
    {{ESP_GATT_AUTO_RSP}, {ESP_UUID_LEN_16, (uint8_t *)&character_tx_cccd_uuid, ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
      sizeof(tx_cccd_enable_notify), sizeof(tx_cccd_enable_notify), (uint8_t *)tx_cccd_enable_notify}},
};

static char device_name[ESP_BLE_ADV_NAME_LEN_MAX] = "ESP_GATEWAY";
static uint16_t local_mtu = 256;        //当前mtu活跃值

static uint8_t adv_config_done = 0;
#define adv_config_flag      (1 << 0)
#define scan_rsp_config_flag (1 << 1)

static uint8_t raw_adv_data[] = {
    /* Flags */
    0x02, ESP_BLE_AD_TYPE_FLAG, 0x06,               // Length 2, Data Type ESP_BLE_AD_TYPE_FLAG, Data 1 (LE General Discoverable Mode, BR/EDR Not Supported)
    /* TX Power Level */
    0x02, ESP_BLE_AD_TYPE_TX_PWR, 0xEB,             // Length 2, Data Type ESP_BLE_AD_TYPE_TX_PWR, Data 2 (-21)
    /* Complete 16-bit Service UUIDs */
    0x03, ESP_BLE_AD_TYPE_16SRV_CMPL, 0xAB, 0xCD,    // Length 3, Data Type ESP_BLE_AD_TYPE_16SRV_CMPL, Data 3 (UUID)

    0x11,ESP_BLE_AD_TYPE_128SRV_PART,0x81,0x41,0x6d,0x20,0xc9,0xcd,0x49,0x2e,0x9d,0x8b,0x6e,0xec,0xcf,0x8a,0x56,0x22,
};

static uint8_t raw_scan_rsp_data[] = {
    /* Complete Local Name */
    0x0C, ESP_BLE_AD_TYPE_NAME_CMPL, 'E', 'S', 'P', '_', 'G', 'A', 'T', 'E', 'W', 'A', 'Y'   // Length 12, Data Type ESP_BLE_AD_TYPE_NAME_CMPL, Data (ESP_GATTS_DEMO)
};


static esp_ble_adv_params_t adv_params = {
    .adv_int_min        = ESP_BLE_GAP_ADV_ITVL_MS(20),
    .adv_int_max        = ESP_BLE_GAP_ADV_ITVL_MS(40),
    .adv_type           = ADV_TYPE_IND,     //广播 + 可连接模式
    .own_addr_type      = BLE_ADDR_TYPE_PUBLIC,     //公共设备地址,永久固定的设备地址
    //.peer_addr            =   //高速定向广播模式下的目标设备地址
    //.peer_addr_type       =
    .channel_map        = ADV_CHNL_ALL,     //在全部的三个信道下依次广播
    .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY, //无白名单，任何人可以扫描连接
};

enum{
    PROFILE_GATEWAY_APP_ID,
    PROFILE_NUM,
};
enum{
    GATEWAY_SVC_INST_ID,
    GW_NUM,
}gw_app_id_e;

// typedef struct {
//     uint16_t conn_id;
//     uint16_t service_handle;
//     esp_gatt_srvc_id_t service_id;
//     uint16_t char_rx_handle;
//     esp_bt_uuid_t char_rx_uuid;
//     esp_gatt_perm_t perm;
//     esp_gatt_char_prop_t property;
//     uint16_t char_tx_handle;
//     esp_bt_uuid_t char_tx_uuid;
//     esp_gatt_perm_t perm;
//     esp_gatt_char_prop_t property;
//     uint16_t descr_handle;
//     esp_bt_uuid_t descr_uuid;
// } gw_service_t;

// typedef struct {
//     union{
//         gw_service_t gw_service;
//     };
// } gatts_service_handles_t;

// struct gatts_profile_inst {
//     esp_gatts_cb_t gatts_cb;
//     uint16_t gatts_if;
//     uint16_t app_id;

//     gatts_service_handles_t *services;    // 服务数组指针
//     uint8_t service_count;                // 服务数量
//     // 当前连接信息（一个 App 可能只有一个连接）
//     uint16_t conn_id;
//     esp_bd_addr_t remote_bda;
// };
//当前为可扩展app,单链接，单服务，先做数据通路，后面再做更改
struct gatts_profile_inst {
    esp_gatts_cb_t gatts_cb;
    uint16_t gatts_if;
    uint16_t app_id;
    uint16_t conn_id;
    
    uint16_t service_handle;
    uint16_t char_handle_rx;
    uint16_t char_handle_tx;
};

/* One gatt-based profile one app_id and one gatts_if, this array will store the gatts_if returned by ESP_GATTS_REG_EVT */
static struct gatts_profile_inst gl_profile_tab[PROFILE_NUM] = {
    [PROFILE_GATEWAY_APP_ID] = {
        .gatts_cb = gatts_profile_gateway_event_handler,
        .gatts_if = ESP_GATT_IF_NONE,       /* Not get the gatt_if, so initial is ESP_GATT_IF_NONE */
    },
};

static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param)
{
    switch (event) {
    case ESP_GAP_BLE_ADV_DATA_RAW_SET_COMPLETE_EVT:
        adv_config_done &= (~adv_config_flag);
        if (adv_config_done==0){
            esp_ble_gap_start_advertising(&adv_params);
        }
        break;
    case ESP_GAP_BLE_SCAN_RSP_DATA_RAW_SET_COMPLETE_EVT:
        adv_config_done &= (~scan_rsp_config_flag);
        if (adv_config_done==0){
            esp_ble_gap_start_advertising(&adv_params);
        }
        break;
    case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:    // 广播开始完成
        //advertising start complete event to indicate advertising start successfully or failed
        if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
            LOGE(GATTS_TAG, "Advertising start failed, status %d", param->adv_start_cmpl.status);
            break;
        }
        LOGI(GATTS_TAG, "Advertising start successfully");
        break;
    case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT: // 停止广播完成
        if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
            LOGE(GATTS_TAG, "Advertising stop failed, status %d", param->adv_stop_cmpl.status);
            break;
        }
        LOGI(GATTS_TAG, "Advertising stop successfully");
        break;
    case ESP_GAP_BLE_UPDATE_CONN_PARAMS_EVT:    // 更新连接参数完成(影响功耗)
        LOGI(GATTS_TAG, "Connection params update, status %d, conn_int %d, latency %d, timeout %d",
                  param->update_conn_params.status,
                  param->update_conn_params.conn_int,
                  param->update_conn_params.latency,
                  param->update_conn_params.timeout);
        // if (param->update_conn_params.conn_int < 40) {  // 小于50ms
        //     esp_ble_conn_update_params_t conn_id;   // 更新连接参数
        //     esp_ble_gap_update_conn_params(&conn_id);
        // }
        break;
    case ESP_GAP_BLE_SET_PKT_LENGTH_COMPLETE_EVT:   // 设置数据包长度完成(影响功耗) (DLE链路层传输，默认27，这个值受到硬件限制?在ble4.2/5.0中支持扩展至257，可能该值略大于MTU可发挥效益最高?)
        LOGI(GATTS_TAG, "Packet length update, status %d, rx %d, tx %d",
                  param->pkt_data_length_cmpl.status,
                  param->pkt_data_length_cmpl.params.rx_len,
                  param->pkt_data_length_cmpl.params.tx_len);
        break;
    case ESP_GAP_BLE_SET_LOCAL_PRIVACY_COMPLETE_EVT: // 开启或关闭 ESP32 蓝牙隐私功能的操作完成后触发
        break;
    default:
        break;
    }
}

static void gatts_profile_gateway_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
    switch (event) {
    case ESP_GATTS_REG_EVT:     // 注册服务
        LOGI(GATTS_TAG, "GATT server register, status %d, app_id %d, gatts_if %d", param->reg.status, param->reg.app_id, gatts_if);
        gl_profile_tab[PROFILE_GATEWAY_APP_ID].app_id = param->reg.app_id;
        esp_ble_gap_set_device_name(device_name);
        esp_err_t raw_adv_ret = esp_ble_gap_config_adv_data_raw(raw_adv_data, sizeof(raw_adv_data));
        if (raw_adv_ret){
            LOGE(GATTS_TAG, "config raw adv data failed, error code = %x ", raw_adv_ret);
        }
        adv_config_done |= adv_config_flag;
        esp_err_t raw_scan_ret = esp_ble_gap_config_scan_rsp_data_raw(raw_scan_rsp_data, sizeof(raw_scan_rsp_data));
        if (raw_scan_ret){
            LOGE(GATTS_TAG, "config raw scan rsp data failed, error code = %x", raw_scan_ret);
        }
        adv_config_done |= scan_rsp_config_flag;
        //generate a resolvable random address
        esp_ble_gap_config_local_privacy(true);
        esp_ble_gatts_create_attr_tab(gw_gatt_db, gatts_if,
                                    GW_IDX_NB, GATEWAY_SVC_INST_ID);
        break;
    case ESP_GATTS_READ_EVT: {      //读
        break;
        }
    case ESP_GATTS_WRITE_EVT: {     //写
        LOGD(GATTS_TAG, "Characteristic write, conn_id %d, trans_id %" PRIu32 ", handle %d", param->write.conn_id, param->write.trans_id, param->write.handle);
        if (ble_to_mqtt_q != NULL) {
            gateway_msg_t msg;
            msg.type = MSG_TYPE_BLE_DATA;
            msg.len = param->write.len > MAX_PAYLOAD_LEN ? MAX_PAYLOAD_LEN : param->write.len;
            memcpy(msg.payload, param->write.value, msg.len);
            msg.timestamp = xTaskGetTickCount();

            // 发送队列，阻塞等待确保数据不丢失，或者使用 xQueueSendWithTimeout
            if (xQueueSend(ble_to_mqtt_q, &msg, pdMS_TO_TICKS(100)) != pdTRUE) {
                LOGE(GATTS_TAG, "BLE to MQTT Queue Full!");
            }
        }
        break;
    }
    case ESP_GATTS_EXEC_WRITE_EVT:      //服务端发出
        break;
    case ESP_GATTS_MTU_EVT:     //MTU事件，获取最终与服务端协商的mtu，客户端mtu的最大接受通过esp_ble_gatt_set_local_mtu设置
        LOGD(GATTS_TAG, "MTU exchange, MTU %d", param->mtu.mtu);
        local_mtu = param->mtu.mtu;
        break;
    case ESP_GATTS_UNREG_EVT:   //注销服务事件(esp_ble_gatts_app_unregister触发)
        break;
    case ESP_GATTS_CREATE_EVT:  //创建服务事件(esp_ble_gatts_create_service触发)
        break;
    case ESP_GATTS_ADD_INCL_SRVC_EVT:   //添加服务(esp_ble_gatts_add_included_service触发)
        break;
    case ESP_GATTS_ADD_CHAR_EVT: {  //添加特征(esp_ble_gatts_add_char触发)
        break;
    }
    case ESP_GATTS_ADD_CHAR_DESCR_EVT:  //添加描述符(esp_ble_gatts_add_char_descr触发)
        break;
    case ESP_GATTS_DELETE_EVT:  //删除服务(esp_ble_gatts_delete_service触发)
        break;
    case ESP_GATTS_START_EVT:   //启动服务(esp_ble_gatts_start_service触发)
        LOGI(GATTS_TAG, "Service start, status %d, service_handle %d",
                 param->start.status, param->start.service_handle);
        break;
    case ESP_GATTS_STOP_EVT:    //停止服务(esp_ble_gatts_stop_service触发)
        break;
    case ESP_GATTS_CONNECT_EVT: {   //连接(esp_ble_gap_start_advertising触发)
        esp_ble_conn_update_params_t conn_params = {0};
        memcpy(conn_params.bda, param->connect.remote_bda, sizeof(esp_bd_addr_t));
        /* For the IOS system, please reference the apple official documents about the ble connection parameters restrictions. */
        conn_params.latency = 0;
        conn_params.max_int = 0x20;    // max_int = 0x20*1.25ms = 40ms
        conn_params.min_int = 0x10;    // min_int = 0x10*1.25ms = 20ms
        conn_params.timeout = 400;    // timeout = 400*10ms = 4000ms
        LOGI(GATTS_TAG, "Connected, conn_id %u, remote "ESP_BD_ADDR_STR"",
                 param->connect.conn_id, ESP_BD_ADDR_HEX(param->connect.remote_bda));
        gl_profile_tab[PROFILE_GATEWAY_APP_ID].conn_id = param->connect.conn_id;
        //start sent the update connection parameters to the peer device.
        esp_ble_gap_update_conn_params(&conn_params);
        break;
    }
    case ESP_GATTS_DISCONNECT_EVT:  //断开连接
        LOGI(GATTS_TAG, "Disconnected, remote "ESP_BD_ADDR_STR", reason 0x%02x",
                 ESP_BD_ADDR_HEX(param->disconnect.remote_bda), param->disconnect.reason);
        esp_ble_gap_start_advertising(&adv_params);
        local_mtu = 23; // Reset MTU for a single connection
        break;
    case ESP_GATTS_CONF_EVT:    //配置完成
        LOGI(GATTS_TAG, "Confirm receive, status %d, attr_handle %d", param->conf.status, param->conf.handle);
        if (param->conf.status != ESP_GATT_OK){
            ESP_LOG_BUFFER_HEX(GATTS_TAG, param->conf.value, param->conf.len);
        }
        break;
    case ESP_GATTS_CREAT_ATTR_TAB_EVT:  //创建属性表完成(esp_ble_gatts_create_attr_tab触发)
        if (param->create.status == ESP_GATT_OK){
            if(param->add_attr_tab.num_handle == GW_IDX_NB) {
                LOGI(GATTS_TAG, "Attribute table create successfully, num_handle %x", param->add_attr_tab.num_handle);
                // memcpy(gw_handle_table, param->add_attr_tab.handles,
                // sizeof(gw_handle_table));
                gl_profile_tab[PROFILE_GATEWAY_APP_ID].service_handle = param->add_attr_tab.handles[GW_SVC_IDX];
                gl_profile_tab[PROFILE_GATEWAY_APP_ID].char_handle_rx = param->add_attr_tab.handles[GW_RX_VAL_IDX];
                gl_profile_tab[PROFILE_GATEWAY_APP_ID].char_handle_tx = param->add_attr_tab.handles[GW_TX_VAL_IDX];
                esp_ble_gatts_start_service(gl_profile_tab[PROFILE_GATEWAY_APP_ID].service_handle);
            }else{
                LOGE(GATTS_TAG, "Attribute table create abnormally, num_handle (%d) doesn't equal to HRS_IDX_NB(%d)",
                     param->add_attr_tab.num_handle, GW_IDX_NB);
            }
        }else{
            LOGE(GATTS_TAG, "Attribute table create failed, error code = %x", param->create.status);
        }
    case ESP_GATTS_OPEN_EVT:
    case ESP_GATTS_CANCEL_OPEN_EVT:
    case ESP_GATTS_CLOSE_EVT:
    case ESP_GATTS_LISTEN_EVT:
    case ESP_GATTS_CONGEST_EVT:
    default:
        break;
    }
}

/**
 * @brief GATT Server 事件回调处理函数
 *
 * 该函数作为 GATT Server 的全局事件入口，负责处理注册事件以及将其他事件分发到具体的 Profile 回调函数中。
 *
 * @param event      GATT 服务器回调事件类型
 * @param gatts_if   GATT 服务器接口标识符
 * @param param      指向包含事件具体参数的结构体指针
 */
static void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param)
{
    /* 处理注册事件：保存每个 Profile 对应的 gatts_if 或记录注册失败日志 */
    if (event == ESP_GATTS_REG_EVT) {
        if (param->reg.status == ESP_GATT_OK) {
            gl_profile_tab[param->reg.app_id].gatts_if = gatts_if;
        } else {
            LOGI(GATTS_TAG, "Reg app failed, app_id %04x, status %d",
                    param->reg.app_id,
                    param->reg.status);
            return;
        }
    }

    /* 遍历所有 Profile，根据 gatts_if 匹配并调用相应的 Profile 回调函数 */
    do {
        int idx;
        for (idx = 0; idx < PROFILE_NUM; idx++) {
            if (gatts_if == ESP_GATT_IF_NONE || /* ESP_GATT_IF_NONE, not specify a certain gatt_if, need to call every profile cb function */
                    gatts_if == gl_profile_tab[idx].gatts_if) {
                if (gl_profile_tab[idx].gatts_cb) {
                    gl_profile_tab[idx].gatts_cb(event, gatts_if, param);
                }
            }
        }
    } while (0);
}

void ble_init(void)
{
    #if CONFIG_EXAMPLE_CI_PIPELINE_ID
    memcpy(device_name, esp_bluedroid_get_example_name(), ESP_BLE_ADV_NAME_LEN_MAX);
    #endif

    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));

    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    int ret = esp_bt_controller_init(&bt_cfg);
    if (ret) {
        LOGE(GATTS_TAG, "%s initialize controller failed: %s", __func__, esp_err_to_name(ret));
        return;
    }

    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret) {
        LOGE(GATTS_TAG, "%s enable controller failed: %s", __func__, esp_err_to_name(ret));
        return;
    }

    ret = esp_bluedroid_init();
    if (ret) {
        LOGE(GATTS_TAG, "%s init bluetooth failed: %s", __func__, esp_err_to_name(ret));
        return;
    }
    ret = esp_bluedroid_enable();
    if (ret) {
        LOGE(GATTS_TAG, "%s enable bluetooth failed: %s", __func__, esp_err_to_name(ret));
        return;
    }
    // Note: Avoid performing time-consuming operations within callback functions.
    ret = esp_ble_gatts_register_callback(gatts_event_handler);
    if (ret){
        LOGE(GATTS_TAG, "gatts register error, error code = %x", ret);
        return;
    }
    ret = esp_ble_gap_register_callback(gap_event_handler);
    if (ret){
        LOGE(GATTS_TAG, "gap register error, error code = %x", ret);
        return;
    }
    ret = esp_ble_gatts_app_register(PROFILE_GATEWAY_APP_ID);
    if (ret){
        LOGE(GATTS_TAG, "gatts app register error, error code = %x", ret);
        return;
    }
    esp_err_t local_mtu_ret = esp_ble_gatt_set_local_mtu(500);
    if (local_mtu_ret){
        LOGE(GATTS_TAG, "set local  MTU failed, error code = %x", local_mtu_ret);
    }

    return;
}

int ble_send_notify(uint8_t *data, int len)
{
    // // 1. 检查连接状态
    // // 假设未连接时 conn_id 为 0xFFFF 或无效值 (根据 ESP-IDF 惯例，通常初始化为 0xFFFF)
    // if (gl_profile_tab[PROFILE_GATEWAY_APP_ID].conn_id == 0xFFFF) {
    //     LOGW(GATTS_TAG, "BLE not connected, cannot send notify");
    //     return -1;
    // }
    uint16_t max_chunk_len = local_mtu - 3;
    LOGD(GATTS_TAG, "Sending %d bytes, MTU: %d, Chunk size: %d", len, local_mtu, max_chunk_len);

    uint16_t offset = 0;
    esp_err_t ret;
    while (offset < len) {
        // 计算当前分片的长度
        uint16_t current_chunk_len = (len - offset > max_chunk_len) ? max_chunk_len : len - offset;
        // 发送当前分片
        // 注意：最后一个参数 false 表示 Notification (不需要确认)，true 表示 Indication (需要确认)
        // 如果需要可靠传输，建议改为 true，但速度会变慢
        ret = esp_ble_gatts_send_indicate(
            gl_profile_tab[PROFILE_GATEWAY_APP_ID].gatts_if, 
            gl_profile_tab[PROFILE_GATEWAY_APP_ID].conn_id, 
            gl_profile_tab[PROFILE_GATEWAY_APP_ID].char_handle_tx, 
            current_chunk_len, 
            data + offset, 
            false
        );
        
        if (ret != ESP_OK) {
            LOGE(GATTS_TAG, "BLE notify failed at offset %d, error = 0x%x", offset, ret);
            return -1;
        }
        // 移动偏移量
        offset += current_chunk_len;
        // 可选：如果是 Indication (true)，这里可能需要等待 ESP_GATTS_CONF_EVT 事件才能发送下一包
        // 对于 Notification (false)，通常可以直接连续发送，但如果数据量极大，建议适当延时避免拥塞
        vTaskDelay(pdMS_TO_TICKS(5)); 
    }
    LOGD(GATTS_TAG, "BLE notify success, total %d bytes sent", len);
    return 0;
}