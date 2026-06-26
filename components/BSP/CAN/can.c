/*
 * SPDX-FileCopyrightText: 2025 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "esp_twai.h"
#include "esp_twai_onchip.h"
#include "driver/gpio.h"

#define TWAI_TX_GPIO            GPIO_NUM_41
#define TWAI_RX_GPIO            GPIO_NUM_40
#define TWAI_QUEUE_DEPTH        10
#define TWAI_BITRATE            1000000

// 消息 ID
#define TWAI_DATA_ID            0x100
#define TWAI_HEARTBEAT_ID       0x7FF
#define TWAI_DATA_LEN           1000

static const char *TWAI_TAG = "twai_node";
static twai_node_handle_t can_gateway_node = NULL;  // TWAI 节点句柄

// 发送完成回调
static IRAM_ATTR bool twai_sender_tx_done_callback(twai_node_handle_t handle, const twai_tx_done_event_data_t *edata, void *user_ctx)
{
    if (!edata->is_tx_success) {
        ESP_EARLY_LOGW(TWAI_TAG, "发送消息失败, ID: 0x%X", edata->done_tx_frame->header.id);
    }
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

// TWAI 接收回调 - 存储数据并发出信号
static bool IRAM_ATTR twai_listener_rx_callback(twai_node_handle_t handle, const twai_rx_done_event_data_t *edata, void *user_ctx)
{
    ESP_EARLY_LOGI(TWAI_TAG, "收到数据: %s", user_ctx);
    return false;
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

    // 注册发送完成回调
    twai_event_callbacks_t callbacks = {
        .on_state_change = twai_listener_on_state_change_callback,
        .on_rx_done = twai_listener_rx_callback,
        .on_tx_done = twai_sender_tx_done_callback,
        .on_error = twai_sender_on_error_callback,
    };
    ESP_ERROR_CHECK(twai_node_register_event_callbacks(can_gateway_node, &callbacks, NULL));

    // 启用 TWAI 节点
    ESP_ERROR_CHECK(twai_node_enable(can_gateway_node));
    ESP_LOGI(TWAI_TAG, "TWAI 启动成功");
    ESP_LOGI(TWAI_TAG, "发送消息 ID: 0x%03X (数据), 0x%03X (心跳)",  TWAI_DATA_ID, TWAI_HEARTBEAT_ID);

    // ESP_ERROR_CHECK(twai_node_disable(can_gateway_node));
    // ESP_ERROR_CHECK(twai_node_delete(can_gateway_node));
}

esp_err_t can_send_data(uint32_t can_id, uint8_t *data, int len)
{
    esp_err_t err = ESP_OK;
    twai_node_status_t status;
    for (int offset = 0; offset < len; offset+= TWAI_FRAME_MAX_LEN) {
        twai_frame_t tx_frame = {
            .header.id = can_id,
            .buffer = data + offset,
            .buffer_len = len - offset > TWAI_FRAME_MAX_LEN ? TWAI_FRAME_MAX_LEN : len - offset,
        };
        err = twai_node_transmit(can_gateway_node, &tx_frame, 500);      //异步，把数据丢入传输数据队列，ID 值大仲裁失败会导致数据一直发不出去
        if (err != ESP_OK) {
            ESP_LOGE(TWAI_TAG, "CAN 发送失败 at offset %d, err: 0x%x", offset, err);
            return err;
        }
    }
    err = twai_node_transmit_wait_all_done(can_gateway_node, pdMS_TO_TICKS(1000)); // 等待队列清空，等待所有帧数据发送完毕(-1 表示永久等待)
    if (err != ESP_OK) {
        ESP_LOGW(TWAI_TAG, "等待发送完成超时");
    }
    return err;
}

esp_err_t can_rcve_data(uint32_t can_id, uint8_t *data, int len)
{
    esp_err_t err = ESP_OK;
}








//  while (1) {
//         // 发送心跳消息
//         uint64_t timestamp = esp_timer_get_time();
//         twai_frame_t tx_frame = {
//             .header.id = TWAI_HEARTBEAT_ID,
//             .buffer = (uint8_t *) &timestamp,
//             .buffer_len = sizeof(timestamp),
//         };
//         ESP_ERROR_CHECK(twai_node_transmit(can_gateway_node, &tx_frame, 500));      //异步，把数据丢入传输数据队列，ID 值大仲裁失败会导致数据一直发不出去
//         ESP_LOGI(TWAI_TAG, "发送心跳消息: %lld", timestamp);
//         ESP_ERROR_CHECK(twai_node_transmit_wait_all_done(can_gateway_node, -1)); // 等待帧数据发送完毕(-1 表示永久等待)

//         // 每 10 秒发送一次突发数据消息
//         if ((timestamp / 1000000) % 10 == 0) {
//             int num_frames = howmany(TWAI_DATA_LEN, TWAI_FRAME_MAX_LEN);
//             twai_sender_data_t *data = (twai_sender_data_t *)calloc(num_frames, sizeof(twai_sender_data_t));
//             assert(data != NULL);
//             ESP_LOGI(TWAI_TAG, "以 %d 帧发送 %d 字节的数据包", TWAI_DATA_LEN, num_frames);
//             for (int i = 0; i < num_frames; i++) {
//                 data[i].frame.header.id = TWAI_DATA_ID;
//                 data[i].frame.buffer = data[i].data;
//                 data[i].frame.buffer_len = TWAI_FRAME_MAX_LEN;
//                 memset(data[i].data, i, TWAI_FRAME_MAX_LEN);
//                 ESP_ERROR_CHECK(twai_node_transmit(can_gateway_node, &data[i].frame, 500));
//             }

//             // 帧已挂载，等待所有帧发送完毕
//             ESP_ERROR_CHECK(twai_node_transmit_wait_all_done(can_gateway_node, -1));
//             free(data);
//         }

//         vTaskDelay(pdMS_TO_TICKS(1000));
//         twai_node_status_t status;
//         twai_node_get_info(can_gateway_node, &status, NULL);
//         if (status.state == TWAI_ERROR_BUS_OFF) {
//             ESP_LOGW(TWAI_TAG, "检测到总线关闭状态");
//             return;
//         }
//     } 