#include "gateway_event.h"
#include "gw_log.h"
#include <string.h>
#include <stdlib.h>
#include "memory_pool.h"

static const char *TAG = "GW_BUS";

// 为每个模块创建一个队列
static QueueHandle_t s_module_queues[MODULE_ID_MAX];
memory_pool *data_pool = NULL;
#define DATAPOOL_LEN 512
#define DATAPOOL_NUM 10
void gateway_event_bus_init(void)
{
    int ret;
    for (int i = 0; i < MODULE_ID_MAX; i++) {
        // 每个模块队列深度 10，存储的是 gateway_event_t 结构体（包含指针）
        s_module_queues[i] = xQueueCreate(10, sizeof(gateway_event_t));
        if (s_module_queues[i] == NULL) {
            LOGE(TAG, "Failed to create queue for module %d", i);
        }
    }
    data_pool = malloc(sizeof(memory_pool));
    ret = mp_init(data_pool, DATAPOOL_LEN, DATAPOOL_NUM);
    if (ret)
    {
        LOGE(TAG, "Failed to init data pool, error : %d",ret);
    }
    LOGI(TAG, "Event Bus Initialized");
}

QueueHandle_t gateway_event_get_module_queue(module_id_t id)
{
    if (id >= MODULE_ID_MAX) return NULL;
    return s_module_queues[id];
}

BaseType_t gateway_event_send(module_id_t dst_id, gateway_event_t *event, TickType_t timeout)
{
    if (dst_id >= MODULE_ID_MAX || s_module_queues[dst_id] == NULL) {
        LOGE(TAG, "Invalid destination module %d", dst_id);
        return pdFALSE;
    }
    
    // 发送事件到目标模块队列
    return xQueueSend(s_module_queues[dst_id], event, timeout);
    //命令类不可丢弃使用xQueueOverwrite()剔除最旧消息不要使用xQueueSend
}

BaseType_t gateway_event_receive(module_id_t src_id, gateway_event_t *event, TickType_t timeout)
{
    if (src_id >= MODULE_ID_MAX || s_module_queues[src_id] == NULL) {
        LOGE(TAG, "Invalid source module %d", src_id);
        return pdFALSE;
    }

    return xQueueReceive(s_module_queues[src_id], event, timeout);
}

static BaseType_t gateway_event_create_mode(gateway_event_t *evt, module_id_t src_id, module_id_t dst_id, uint32_t cmd_id, void *data, uint16_t data_len, data_mode_t data_mode)
{
    evt->src_id = src_id;
    evt->dst_id = dst_id;
    evt->cmd_id = cmd_id;
    if (data_mode) {
        evt->data = data;
    } else {
        evt->data = mp_alloc(data_pool, data_len);
        if (evt->data == NULL) {
            LOGE(TAG, "Failed to allocate memory for event data");
            return pdFALSE;
        }
        memcpy(evt->data, data, data_len);
    }
    evt->data_len = data_len;
    return pdTRUE;
}

BaseType_t gateway_event_create_ref(gateway_event_t *evt, module_id_t src_id, module_id_t dst_id, uint32_t cmd_id, void *data, uint16_t data_len)
{
    return gateway_event_create_mode(evt, src_id, dst_id, cmd_id, data, data_len, DATA_REF);
}

BaseType_t gateway_event_create(gateway_event_t *evt, module_id_t src_id, module_id_t dst_id, uint32_t cmd_id, void *data, uint16_t data_len)
{
    return gateway_event_create_mode(evt, src_id, dst_id, cmd_id, data, data_len, DATA_MALLOC);
}

void gateway_event_free(gateway_event_t *evt)
{
    if (!evt->data_mode) {
        mp_free(data_pool, evt->data);
        evt->data = NULL;
        evt->data_len = 0;
    }
}
