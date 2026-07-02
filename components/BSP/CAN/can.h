#ifndef __CAN_H
#define __CAN_H

#include "freertos/FreeRTOS.h"
#include "esp_twai.h"
#include "esp_twai_onchip.h"
#include "driver/gpio.h"
#include "esp_log.h"

// 消息 ID
#define TWAI_DATA_ID            0x100
#define TWAI_HEARTBEAT_ID       0x7FF

#define RX_RING_SIZE  64
typedef struct {
    twai_frame_t frames[RX_RING_SIZE];
    uint8_t data_buffers[RX_RING_SIZE][TWAI_FRAME_MAX_LEN];
    volatile int head;  // 写指针
    volatile int tail;  // 读指针
    volatile uint32_t drop_count;  // 丢帧统计
    volatile uint32_t total_count; // 总接收统计
} can_ring_buffer_t;

extern can_ring_buffer_t ring_buffer;
extern const char *TWAI_TAG;
extern twai_node_handle_t can_gateway_node;

void can_bus_init(void);
int twai_node_receive(twai_frame_t *frame);

#endif
