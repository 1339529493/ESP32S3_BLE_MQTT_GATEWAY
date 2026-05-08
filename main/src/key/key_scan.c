#include "xl9555.h"
#include "key_scan.h"
#include "gw_log.h"

#define KEY_SCAN_TAG "KEY_SCAN"
QueueHandle_t key_scan_q;
void key_scan_task(void *pvParameter)
{
    key_scan_q = xQueueCreate(10, sizeof(key_scan_msg_t));
    key_scan_msg_t key_scan_msg;
    uint8_t key;
    while (1)
    {
        // switch (xl9555_key_scan(0))
        // {
        // case KEY0_PRES:
        // {
        //     key_scan_msg.key_code = KEY0_PRES;
        //     break;
        // }
        // case KEY1_PRES:
        // {
        //     key_scan_msg.key_code = KEY1_PRES; 
        //     break;
        // }
        // case KEY2_PRES:
        // {
        //     key_scan_msg.key_code = KEY2_PRES;
        //     break;
        // }
        // case KEY3_PRES:
        // {
        //     key_scan_msg.key_code = KEY3_PRES;
        //     break;
        // }
        // default:
        // {
        //     break;
        // }
        // }
        key = xl9555_key_scan(0);
        if (key && xQueueSend(key_scan_q, &key_scan_msg, pdMS_TO_TICKS(100)) != pdTRUE) 
        {
            LOGW(KEY_SCAN_TAG, "key scan Queue Full!");
        }
        vTaskDelay(pdMS_TO_TICKS(10)); 
    }
}