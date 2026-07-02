#ifndef __CAN_H
#define __CAN_H

#include "freertos/FreeRTOS.h"
#include "esp_twai.h"
#include "esp_twai_onchip.h"
#include "driver/gpio.h"
#include "esp_log.h"

#define RX_RING_SIZE  64
typedef struct {
    twai_frame_t frames[RX_RING_SIZE];
    uint8_t data_buffers[RX_RING_SIZE][TWAI_FRAME_MAX_LEN];
    volatile int head;  // 写指针
    volatile int tail;  // 读指针
    volatile uint32_t drop_count;  // 丢帧统计
    volatile uint32_t total_count; // 总接收统计
} can_ring_buffer_t;
#define CAN_ID_DATA_LEN 512
typedef struct {
    uint32_t can_id;
    uint8_t data_buffers[CAN_ID_DATA_LEN];
    volatile int head;
    volatile int tail;
} can_id_rvce_buffer_t;

extern can_ring_buffer_t ring_buffer;
extern const char *TWAI_TAG;
extern twai_node_handle_t can_gateway_node;

void can_bus_init(void);
esp_err_t can_send_data_private(uint32_t can_id, uint8_t *data, int len);

#endif
