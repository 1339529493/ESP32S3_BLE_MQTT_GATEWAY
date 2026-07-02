#include "can_protocol.h"
#include "esp_timer.h"

/**
 * @brief 支持分片计算的 CRC16-CCITT
 * @param init_crc 初始 CRC 值。如果是第一片数据，传 0；如果是后续数据，传上一次计算的返回值。
 * @param data 数据指针
 * @param length 数据长度
 * @return 计算后的 CRC 值
 */
uint16_t crc16_ccitt_update(uint16_t init_crc, uint8_t *data, uint16_t length)
{
    uint8_t i;
    uint16_t crc = init_crc; // 使用传入的初始值，而不是固定为 0
    while(length--)
    {
        crc ^= *data++;
        for (i = 0; i < 8; ++i)
        {
            if (crc & 1)
                crc = (crc >> 1) ^ 0x8408;
            else
                crc = (crc >> 1);
        }
    }
    return crc;
}

uint16_t crc16_ccitt(uint8_t *data, uint16_t length)
{
    return crc16_ccitt_update(0, data, length);
}

#define CAN_CRC_LEN 2
#define HEAD_FAREM_LEN 8
#define HEAD_FAREM_INIT(len, cmd)   \
    {0X55, 0XAA, \
    ((len) & 0xff00) >> 8, (len) & 0xff, \
    ((cmd) & 0xff00) >> 8, (cmd) & 0xff, \
    0, 0}

/*
 * @brief CAN 数据发送 私有协议数据(head+data+crc) test canid 0x100
 * 
 * @param can_id CAN ID
 * @param cmd 命令
 * @param data 数据指针
 * @param len 数据长度
 * @return 0 成功，-1 失败
*/
int can_send_data_private(uint32_t can_id, uint16_t cmd,uint8_t *data, int len)
{
    esp_err_t err;
    uint8_t frame_buf[TWAI_FRAME_MAX_LEN] = HEAD_FAREM_INIT(len+HEAD_FAREM_LEN+CAN_CRC_LEN, cmd);
    twai_frame_t tx_frame;
    uint16_t crc;
    crc = crc16_ccitt_update(0, data, len);
    tx_frame.header.id = can_id;
    tx_frame.buffer = frame_buf;
    tx_frame.buffer_len = 8;
    err = twai_node_transmit(can_gateway_node, &tx_frame, pdMS_TO_TICKS(500));
    if (err != ESP_OK) {
        ESP_LOGE(TWAI_TAG, "帧头发送失败, err: 0x%x", err);
        return -1;
    }
    for (int offset = 0; offset < len; offset+= TWAI_FRAME_MAX_LEN) {
        if (len - offset > TWAI_FRAME_MAX_LEN)  // 数据长度大于TWAI_FRAME_MAX_LEN
        {
            tx_frame.buffer = data + offset;
            tx_frame.buffer_len = TWAI_FRAME_MAX_LEN;
        }
        else if(len - offset == TWAI_FRAME_MAX_LEN || len - offset == TWAI_FRAME_MAX_LEN - 1) // 算上校验位一帧长度不够
        {
            tx_frame.buffer = data + offset;
            tx_frame.buffer_len = len - offset;
            err = twai_node_transmit(can_gateway_node, &tx_frame, pdMS_TO_TICKS(500));
            if (err != ESP_OK) {
                ESP_LOGE(TWAI_TAG, "CAN 发送失败 at offset %d, err: 0x%x", offset, err);
                return -1;
            }
            err = twai_node_transmit_wait_all_done(can_gateway_node, pdMS_TO_TICKS(1000));
            if (err != ESP_OK) {
                ESP_LOGW(TWAI_TAG, "等待发送完成超时");
                return -2;
            }
            frame_buf[0] = (crc & 0xff00) >> 8;
            frame_buf[1] = crc & 0xff;
            tx_frame.buffer = frame_buf;
            tx_frame.buffer_len = 2;
        }
        else
        {
            memcpy(frame_buf, data + offset, len - offset);
            frame_buf[len - offset] = (crc & 0xff00) >> 8;
            frame_buf[len - offset + 1] = crc & 0xff;
            tx_frame.buffer = frame_buf;
            tx_frame.buffer_len = len - offset + 2;
        }
        err = twai_node_transmit(can_gateway_node, &tx_frame, pdMS_TO_TICKS(500));      //异步，把数据丢入传输数据队列，ID 值大仲裁失败会导致数据一直发不出去
        if (err != ESP_OK) {
            ESP_LOGE(TWAI_TAG, "CAN 发送失败 at offset %d, err: 0x%x", offset, err);
            return -1;
        }
        err = twai_node_transmit_wait_all_done(can_gateway_node, pdMS_TO_TICKS(1000)); // 等待队列清空，等待所有帧数据发送完毕(-1 表示永久等待)
    }
    if (err != ESP_OK) {
        ESP_LOGW(TWAI_TAG, "等待发送完成超时");
        return -2;
    }
    return 0;
}

/**
 * @brief 重置单个 ID 的接收状态机
 */
static void reset_private_state(private_data_state_t *state)
{
    state->state = RX_STATE_IDLE;
    state->header_idx = 0;
    state->payload_idx = 0;
    state->total_len = 0;
    state->payload_len = 0;
    state->calc_crc = 0;
    state->recv_crc_high = 0;
}

/**
 * @brief CAN 单个字节数据接收处理 (核心状态机)
 *
 * @param entry CAN ID 对应的缓冲区入口
 * @return int 
 *         1: 一帧完整数据接收并校验成功 (进入 FRAME_COMPLETE 状态)
 *         0: 接收中，或无数据，或正在处理头部/同步字
 *        -1: 校验失败、长度非法或协议错误，状态已重置
 */
static int can_rcve_data_private_bit_process(can_id_rvce_buffer_t *entry)
{
    private_data_state_t *state = (private_data_state_t *)entry->state;
    if (state == NULL) return -1;

    // 如果已经处于完成状态，不再处理新数据，直到上层读取并重置
    if (state->state == RX_STATE_FRAME_COMPLETE) {
        return 1; 
    }

    uint8_t byte;
    if (can_id_buffer_read_bit(entry, &byte) != 0) {
        return 0; // 缓冲区空，无数据
    }

    switch (state->state)
    {
    case RX_STATE_IDLE:
        if (byte == 0x55) {
            state->state = RX_STATE_SYNC_2;
        }
        break;

    case RX_STATE_SYNC_2:
        if (byte == 0xAA) {
            state->state = RX_STATE_HEADER;
            state->header_idx = 0;
            state->calc_crc = 0;
        } else {
            state->state = RX_STATE_IDLE; // 同步失败
        }
        break;

    case RX_STATE_HEADER:
        state->header_buf[state->header_idx] = byte;
        state->header_idx++;
        
        // 当收满 6 个字节后 (Len(2) + Cmd(2) + Res(2))
        if (state->header_idx >= 6) {
            uint16_t len = ((uint16_t)state->header_buf[0] << 8) | state->header_buf[1];
            
            state->payload_len = len - HEAD_FAREM_LEN - CAN_CRC_LEN;
            
            // 合法性检查：防止内存越界
            if (len > MAX_PRIVATE_PAYLOAD_LEN) {
                reset_private_state(state);
                return -1;
            }
            
            state->payload_idx = 0;
            
            if (len == 0) {
                // 如果没有 Payload，直接跳到 CRC 读取
                state->state = RX_STATE_CRC_1;
            } else {
                state->state = RX_STATE_PAYLOAD;
            }
        }
        break;

    case RX_STATE_PAYLOAD:
        // 保存数据到状态机缓冲区
        if (state->payload_idx < state->payload_len) {
            state->recv_buf[state->payload_idx] = byte;
            state->payload_idx++;
            
            // 如果 Payload 接收完毕
            if (state->payload_idx == state->payload_len) {
                state->calc_crc = crc16_ccitt_update(state->calc_crc, state->recv_buf, state->payload_len);
                state->state = RX_STATE_CRC_1;
            }
        } else {
            // 异常保护：索引超出范围，重置
            reset_private_state(state);
            return -1;
        }
        break;

    case RX_STATE_CRC_1:
        state->recv_crc_high = byte;
        state->state = RX_STATE_CRC_2;
        break;

    case RX_STATE_CRC_2:
        {
            uint16_t recv_crc = ((uint16_t)state->recv_crc_high << 8) | byte;
            if (recv_crc == state->calc_crc) {
                // 校验成功，进入完成状态
                state->state = RX_STATE_FRAME_COMPLETE; 
                return 1; 
            } else {
                reset_private_state(state);
                return -1;
            }
        }
        break;
        
    default:
        reset_private_state(state);
        break;
    }

    return 0;
}

/**
 * @brief CAN 数据接收 私有协议数据 (阻塞/非阻塞封装)
 * 
 * @param can_id CAN ID
 * @param cmd 命令
 * @param data 数据指针 (输出缓冲区)
 * @param len 数据长度指针 (输入:最大缓冲大小, 输出:实际接收到的有效数据长度)
 * @param timeout_ms 超时时间 (毫秒)。
 * @return 0 成功，其他 失败 (校验错误、超时、ID未找到、用户缓冲区太小)
*/
int can_rcve_data_private(uint32_t can_id,uint16_t cmd, uint8_t *data, int *len, int timeout_ms)
{
    if (data == NULL || len == NULL) return -1;

    can_id_rvce_buffer_t *entry = find_can_entry_by_id(can_id);
    if (entry == NULL || entry->state == NULL) {
        return -1;
    }

    private_data_state_t *state = (private_data_state_t *)entry->state;
    
    TickType_t start_tick = xTaskGetTickCount();
    TickType_t wait_tick = pdMS_TO_TICKS(timeout_ms);
    
    // 1. 循环调用 bit 接口，直到帧完成或出错
    while (1) {
        int ret = can_rcve_data_private_bit_process(entry);
        
        if (ret == 1) {
            // 帧接收成功
            break;
        } else if (ret == -1) {
            // 校验错误或协议错误
            return -2;
        }
        
        // 检查超时
        if (timeout_ms > 0) {
            if (xTaskGetTickCount() - start_tick > wait_tick) {
                reset_private_state(state);
                return -3;
            }
        } else {

            return -4; 
        }
        
        // 延时 1ms，让出 CPU 给 can_task 填充缓冲区
        vTaskDelay(pdMS_TO_TICKS(1));
    }

    // 2. 帧已完成，从 state 中拷贝数据
    if (*len < state->payload_len) {
        reset_private_state(state);
        return -5; // 缓冲区太小
    }

    memcpy(data, state->recv_buf, state->payload_len);
    *len = state->payload_len;

    // 3. 重置状态机，准备下一帧
    reset_private_state(state);

    return 0;
}
