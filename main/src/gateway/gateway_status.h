#ifndef __GATEWAY_STATUS_H
#define __GATEWAY_STATUS_H

#include <stdint.h>
#include <stddef.h>
#include <stdbool.h>

// 连接状态枚举
typedef enum {
    STATUS_DISCONNECTED = 0,
    STATUS_CONNECTING,
    STATUS_CONNECTED,
    STATUS_RECONNECT,
    STATUS_ERROR
} conn_status_t;

// 系统全局状态结构体
typedef struct {
    conn_status_t wifi_status;
    conn_status_t ble_status;   // BLE Server 是否被手机连接
    conn_status_t mqtt_status;
    
    // 可选：添加信号强度或错误码
    int8_t wifi_rssi;
    bool is_time_synced;
} system_status_t;

// 获取全局状态实例的接口
system_status_t* get_system_status(void);

// 更新状态的接口 (线程安全)
void update_wifi_status(conn_status_t status);
void update_ble_status(conn_status_t status);
void update_mqtt_status(conn_status_t status);

#endif