#include "can.h"
#include "esp_timer.h"

extern can_id_rvce_buffer_t* find_can_entry_by_id(uint32_t id);
extern int can_id_buffer_read_bit(can_id_rvce_buffer_t *ring_buf , uint8_t *data);
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
 * @param data 数据指针
 * @param len 数据长度
 * @return 0 成功，-1 失败
*/
int can_send_data_private(uint32_t can_id, uint8_t *data, int len)
{
    esp_err_t err;
    uint8_t frame_buf[TWAI_FRAME_MAX_LEN] = HEAD_FAREM_INIT(len+HEAD_FAREM_LEN+CAN_CRC_LEN, 0x0000);
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
    }
    err = twai_node_transmit_wait_all_done(can_gateway_node, pdMS_TO_TICKS(1000)); // 等待队列清空，等待所有帧数据发送完毕(-1 表示永久等待)
    if (err != ESP_OK) {
        ESP_LOGW(TWAI_TAG, "等待发送完成超时");
        return -2;
    }
    return 0;
}

/*
 * @brief CAN 单个数据接收
 *
 * @param can_id CAN ID
 * @param data 数据指针 (输出缓冲区)
 * @return 1，一帧数据接收完成，0 一帧数据接收中，-1 失败 
*/
int can_rcve_data_private_bit(uint32_t can_id, uint8_t *data)
{
    return 0;
}

/**
 * @brief CAN 数据接收 私有协议数据(head(2)len(2)cmd(2)reserved(2)data(len)crc(2)) test canid 0x100
 * 
 * @param can_id CAN ID
 * @param data 数据指针 (输出缓冲区)
 * @param len 数据长度指针 (输入:最大缓冲大小, 输出:实际接收到的有效数据长度)
 * @param timeout_ms 超时时间 (毫秒)。如果为 0，则非阻塞模式；如果 >0，则等待直到超时或收满一帧。
 * @return 0 成功，-1 失败 (校验错误、超时或ID未找到)，-2 用户缓冲区太小
*/
int can_rcve_data_private(uint32_t can_id, uint8_t *data, int *len, int timeout_ms)
{ 
    return 0;
}
