#ifndef __KEY_SCAN_H
#define __KEY_SCAN_H

#include "gateway_cmd.h"

typedef struct {
    int key_code;
} key_scan_msg_t;

extern QueueHandle_t key_scan_q;
void key_scan_task(void *pvParameter);

#endif