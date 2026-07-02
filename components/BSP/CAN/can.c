/*
 * SPDX-FileCopyrightText: 2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <string.h>
#include <sys/param.h>
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "can.h"

#define TWAI_TX_GPIO            GPIO_NUM_4
#define TWAI_RX_GPIO            GPIO_NUM_5
#define TWAI_QUEUE_DEPTH        10
#define TWAI_BITRATE            1000000


const char *TWAI_TAG = "twai_node";
twai_node_handle_t can_gateway_node = NULL;  // TWAI 节点句柄

can_ring_buffer_t ring_buffer;
TaskHandle_t xCanTaskHandle;

/**
 * @brief 初始化环形缓冲区
 */
static void init_ring_buffer(void) 
{
    for (int i = 0; i < RX_RING_SIZE; i++) {
        ring_buffer.frames[i].buffer = ring_buffer.data_buffers[i];
        ring_buffer.frames[i].buffer_len = TWAI_FRAME_MAX_LEN;
    }
    ring_buffer.head = 0;
    ring_buffer.tail = 0;
    ring_buffer.drop_count = 0;
    ring_buffer.total_count = 0;
}

// 发送完成回调
static IRAM_ATTR bool twai_sender_tx_done_callback(twai_node_handle_t handle, const twai_tx_done_event_data_t *edata, void *user_ctx)
{
    if (!edata->is_tx_success) {
        ESP_EARLY_LOGW(TWAI_TAG, "发送消息失败, ID: 0x%X", edata->done_tx_frame->header.id);
    }
    // else
    // {
    //     ESP_EARLY_LOGI(TWAI_TAG, "发送消息成功, ID: 0x%X, len: %d", edata->done_tx_frame->header.id, edata->done_tx_frame->buffer_len);
    // }
    return false; // 无需唤醒任务
}

// 总线错误回调
static IRAM_ATTR bool twai_sender_on_error_callback(twai_node_handle_t handle, const twai_error_event_data_t *edata, void *user_ctx)
{
    ESP_EARLY_LOGW(TWAI_TAG, "TWAI 节点错误: 0x%x", edata->err_flags.val);
    return false;
}

// 节点状态变化回调
static bool IRAM_ATTR twai_listener_on_state_change_callback(twai_node_handle_t handle, const twai_state_change_event_data_t *edata, void *user_ctx)
{
    const char *twai_state_name[] = {"error_active", "error_warning", "error_passive", "bus_off"};
    ESP_EARLY_LOGI(TWAI_TAG, "状态改变: %s -> %s", twai_state_name[edata->old_sta], twai_state_name[edata->new_sta]);
    return false;
}

static bool IRAM_ATTR twai_listener_rx_callback(twai_node_handle_t handle, const twai_rx_done_event_data_t *edata, void *user_ctx)
{
    // ESP_EARLY_LOGI(TWAI_TAG, "收到数据");
    can_ring_buffer_t *rx_ring = (can_ring_buffer_t *)user_ctx;
    BaseType_t woken = pdFALSE;
    int next_head = (rx_ring->head + 1) % RX_RING_SIZE;
    
    // 检查是否满了
    if (next_head == rx_ring->tail) {
        rx_ring->drop_count++;  // 丢弃该帧
        return false;
    }

    // 从硬件接收数据
    esp_err_t ret = twai_node_receive_from_isr(handle, &rx_ring->frames[rx_ring->head]);
    if (ret == ESP_OK) {
        // ESP_EARLY_LOGI(TWAI_TAG, "收到数据, len %d",rx_ring->frames[rx_ring->head].header.dlc);
        rx_ring->head = next_head;
        rx_ring->total_count++;
        vTaskNotifyGiveFromISR(xCanTaskHandle, &woken);
    }
    return (woken == pdTRUE);   //pdTRUE使得更高优先级任务解除阻塞，申请调度器上下文切换
}

void can_bus_init(void)
{
    // 配置 TWAI 节点
    twai_onchip_node_config_t node_config = {
        .io_cfg = {
            .tx = TWAI_TX_GPIO,
            .rx = TWAI_RX_GPIO,
            .quanta_clk_out = -1,
            .bus_off_indicator = -1,
        },
        .bit_timing = {
            .bitrate = TWAI_BITRATE,    // 波特率
        },
        .fail_retry_cnt = 3,            // 错误重试次数
        .tx_queue_depth = TWAI_QUEUE_DEPTH,     // 发送队列深度
        .flags.enable_self_test = true, // 允许自测模式
        .flags.enable_loopback = true, // 允许回环模式
    };
    
    // 创建 TWAI 节点
    ESP_ERROR_CHECK(twai_new_node_onchip(&node_config, &can_gateway_node));

    // 配置接受过滤器
    twai_mask_filter_config_t data_filter = {
        .id = TWAI_DATA_ID,
        .mask = 0x7F0,      // 匹配 ID 的高 7 位，忽略低 4 位
        .is_ext = false,    // 仅接收标准 ID
    };
    ESP_ERROR_CHECK(twai_node_config_mask_filter(can_gateway_node, 0, &data_filter));
    ESP_LOGI(TWAI_TAG, "已启用过滤器 ID: 0x%03X 掩码: 0x%03X", data_filter.id, data_filter.mask);
    init_ring_buffer();

    // 注册发送完成回调
    twai_event_callbacks_t callbacks = {
        .on_state_change = twai_listener_on_state_change_callback,
        .on_rx_done = twai_listener_rx_callback,
        .on_tx_done = twai_sender_tx_done_callback,
        .on_error = twai_sender_on_error_callback,
    };
    ESP_ERROR_CHECK(twai_node_register_event_callbacks(can_gateway_node, &callbacks, &ring_buffer));
    // 启用 TWAI 节点
    ESP_ERROR_CHECK(twai_node_enable(can_gateway_node));
    ESP_LOGI(TWAI_TAG, "TWAI 启动成功");
    ESP_LOGI(TWAI_TAG, "发送消息 ID: 0x%03X (数据), 0x%03X (心跳)",  TWAI_DATA_ID, TWAI_HEARTBEAT_ID);
}

/**
 * @brief 接收数据
 * 
 * @param frame 数据帧
 * @return int 接收数量
 */
int twai_node_receive(twai_frame_t *frame)
{
    if (frame == NULL) {
        return -1;
    }
    if (ring_buffer.head == ring_buffer.tail) {
        return 0;
    }
    memcpy((void *)frame, &ring_buffer.frames[ring_buffer.tail], sizeof(twai_frame_t));
    ring_buffer.tail = (ring_buffer.tail + 1) % RX_RING_SIZE;
    return 1;
}
