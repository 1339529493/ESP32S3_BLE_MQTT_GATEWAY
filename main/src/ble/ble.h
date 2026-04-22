#ifndef __BLE_H
#define __BLE_H

#include "gateway_msg.h"

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

#endif
