#include "xl9555.h"
#include "key_scan.h"
#include "gw_log.h"

#define KEY_SCAN_TAG "KEY_SCAN"
void key_scan_task(void *pvParameter)
{
    gateway_event_t msg;
    uint32_t key_code;
    while (1)
    {
        key_code = xl9555_key_scan(0);
        if (key_code)
        {
            GATEWAY_EVENT_INIT_CMD(&msg, MODULE_ID_KEY, MODULE_ID_UI, CMD_UI_NONE, key_code);
            if (gateway_event_send(MODULE_ID_UI, &msg, pdMS_TO_TICKS(100)) != pdTRUE) 
            {
                LOGE(KEY_SCAN_TAG, "KEY to UI Queue Full! key : %d",key_code);
            }
        }
        vTaskDelay(pdMS_TO_TICKS(10)); 
    }
}