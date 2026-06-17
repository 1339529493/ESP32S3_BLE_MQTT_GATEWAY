#ifndef __GATEWAY_CMD_H
#define __GATEWAY_CMD_H

#include "gateway_status.h"
#include "gateway_event.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 全局命令字定义
 * 命名规范: CMD_<源模块>_TO_<目标模块>_<动作>
 * 或者: CMD_<目标模块>_<动作> (如果来源单一)
 */

// --- BLE 相关命令 ---
typedef enum {
    CMD_BLE_NONE = 0,
    
    // MQTT -> BLE: 要求 BLE 发送 Notify 给手机
    CMD_MQTT_TO_BLE_NOTIFY = 0x1001,
    
    // UI -> BLE: 要求 BLE 修改广播名称或参数
    CMD_UI_TO_BLE_SET_NAME = 0x1002,
    
    // BLE -> MQTT: BLE 收到手机数据，请求 MQTT 上报
    CMD_BLE_TO_MQTT_PUBLISH = 0x1003,
} ble_cmd_id_t;

// --- MQTT 相关命令 ---
typedef enum {
    CMD_MQTT_NONE = 0,
    
    // BLE -> MQTT: 同上，为了对称也可以在这里定义，但通常共用一个 ID 即可
    // 如果 MQTT 内部有子状态机，可以定义内部命令
    CMD_MQTT_INTERNAL_RECONNECT = 0x2001,

    // WiFi -> MQTT: 网络已就绪，请启动/重连 MQTT
    CMD_WIFI_TO_MQTT_START = 0x2005,
    
    // WiFi -> MQTT: 网络已断开，请停止 MQTT
    CMD_WIFI_TO_MQTT_STOP = 0x2006,
} mqtt_cmd_id_t;

// --- UI 相关命令 ---
typedef enum {
    CMD_UI_NONE = 0,
    
    // 更新屏幕上的连接状态图标
    CMD_UI_UPDATE_STATUS = 0x3001,
    
    // KEY -> UI: 按键触发页面跳转
    CMD_KEY_TO_UI_SWITCH_PAGE = 0x3002,
} ui_cmd_id_t;

// --- WIFI 相关命令 --- 
typedef enum { 
    CMD_WIFI_NONE = 0,
    
    // 更新要连接WIFI用户名和密码 
    CMD_BLE_TO_WIFI_PROVISION = 0x4001,

    // 连接WIFI
    CMD_BLE_TO_WIFI_CONNECT = 0x4002,

    // ota升级
    CMD_BLE_TO_WIFI_OTA_DOWNLOAD = 0x4003,

    // ota手动回滚
    CMD_BLE_TO_WIFI_OTA_ROLLBACK = 0x4004,

} wifi_cmd_id_t;

typedef struct {
    char ssid[32];
    char pwd[64];
} wifi_provision_info_t;

#define KEY_1 0x01
#define KEY_2 0x02
#define KEY_3 0x03
#define KEY_4 0x04

#ifdef __cplusplus
}
#endif

#endif /* __GATEWAY_CMD_H */