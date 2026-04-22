#ifndef __GATEWAY_MSG_H
#define __GATEWAY_MSG_H

#include <stdint.h>
#include <stddef.h>

#define MAX_PAYLOAD_LEN 512

typedef enum {
    MSG_TYPE_BLE_DATA,
    MSG_TYPE_MQTT_CMD,
    MSG_TYPE_SYS_STATUS
} gw_msg_type_e;

typedef struct {
    gw_msg_type_e type;
    uint8_t payload[MAX_PAYLOAD_LEN];
    size_t len;
    uint32_t timestamp; // 可选：用于调试或时序分析
} gateway_msg_t;

#endif