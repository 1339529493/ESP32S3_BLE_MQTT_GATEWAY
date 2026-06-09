#ifndef __GATEWAY_EVENT_H
#define __GATEWAY_EVENT_H

#include <stdint.h>
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

#ifdef __cplusplus
extern "C" {
#endif

// 定义模块 ID
typedef enum {
    MODULE_ID_BLE = 0,
    MODULE_ID_MQTT,
    MODULE_ID_UI,
    MODULE_ID_KEY,
    MODULE_ID_WIFI,
    MODULE_ID_MAX
} module_id_t;

// 通用事件包
typedef struct {
    module_id_t src_id;       // 来源模块
    module_id_t dst_id;       // 目标模块 (MODULE_ID_MAX 表示广播或网关处理)
    uint32_t cmd_id;          // 具体命令字 (由各个模块自己定义)
    // void *data;               // 数据指针 (注意内存管理，通常由发送者 malloc，接收者 free)
    union {
        void *data;
        uint32_t short_msg;        // 按键码/状态值
    };
    uint16_t data_len;        // 数据长度

    uint32_t timestamp; // 可选：用于调试或时序分析
} gateway_event_t;

/**
 * @brief 初始化事件总线
 */
void gateway_event_bus_init(void);

/**
 * @brief 向指定模块发送事件
 * @param dst_id 目标模块
 * @param event 事件指针
 * @param timeout 超时时间
 * @return pdTRUE if successful
 */
BaseType_t gateway_event_send(module_id_t dst_id, gateway_event_t *event, TickType_t timeout);

/**
 * @brief 获取当前模块的事件队列句柄 (供模块内部任务使用)
 */
BaseType_t gateway_event_receive(module_id_t src_id, gateway_event_t *event, TickType_t timeout);

/**
 * @brief 创建事件包
 */
BaseType_t gateway_event_create(gateway_event_t *evt, module_id_t src_id, module_id_t dst_id, uint32_t cmd_id, void *data, uint16_t data_len);

/**
 * @brief 创建事件包 (零拷贝接口)
 */
BaseType_t gateway_event_create_ref(gateway_event_t *evt, module_id_t src_id, module_id_t dst_id, uint32_t cmd_id, void *data, uint16_t data_len);

void gateway_event_free(gateway_event_t *evt);

// 创建事件包
#define GATEWAY_EVENT_INIT_CMD(evt, src, dst, cmd, val) \
    do { \
        (evt)->src_id = (src); \
        (evt)->dst_id = (dst); \
        (evt)->cmd_id = (cmd); \
        (evt)->short_msg = (val); \
        (evt)->data_len = 0; \
        (evt)->timestamp = xTaskGetTickCount(); \
    } while(0)

/**
 * @brief 获取指定模块的事件队列句柄 (供模块内部任务使用)
 */
QueueHandle_t gateway_event_get_module_queue(module_id_t id);

#ifdef __cplusplus
}
#endif

#endif /* __GATEWAY_EVENT_H */