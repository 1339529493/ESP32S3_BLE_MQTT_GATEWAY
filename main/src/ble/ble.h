#ifndef __BLE_H
#define __BLE_H

#include "gateway_cmd.h"
#include "esp_gatts_api.h"
#include "gw_log.h"

extern const char *GATTS_TAG;

enum {
    GW_SVC_IDX,
    GW_RX_CHAR_IDX,
    GW_RX_VAL_IDX,
    GW_TX_CHAR_IDX,
    GW_TX_VAL_IDX,
    GW_TX_CCCD_IDX,
    GW_IDX_NB,
};

void ble_init(void);
int ble_send_notify(uint8_t *data, int len);
void ble_task(void *pvParameter);
void gateway_control_channel_hander(esp_gatts_cb_event_t event, esp_ble_gatts_cb_param_t *param);

#endif
