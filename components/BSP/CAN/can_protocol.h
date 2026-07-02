#ifndef __CAN_PROTOCOL_H
#define __CAN_PROTOCOL_H

#include "can.h"

typedef enum {
    RX_STATE_IDLE = 0,
    RX_STATE_SYNC_1,      // 等待 0x55 (保留以兼容旧逻辑，虽然代码中直接跳到了 SYNC_2)
    RX_STATE_SYNC_2,      // 等待 0xAA
    RX_STATE_HEADER,      // 读取剩余 6 字节头
    RX_STATE_PAYLOAD,     // 读取 Payload
    RX_STATE_CRC_1,       // 读取 CRC 高字节
    RX_STATE_CRC_2,       // 读取 CRC 低字节
    RX_STATE_FRAME_COMPLETE // 新增：一帧接收完成，等待上层读取
} private_data_state_e;

#define MAX_PRIVATE_PAYLOAD_LEN 256 // 根据实际需求调整最大负载长度

typedef struct {
    private_data_state_e state;
    uint8_t header_buf[6];
    uint8_t header_idx;
    uint16_t total_len;
    uint16_t payload_len;
    uint16_t payload_idx;
    uint16_t calc_crc;
    uint8_t recv_crc_high;
    
    // 新增：接收数据缓冲区
    uint8_t recv_buf[MAX_PRIVATE_PAYLOAD_LEN]; 
} private_data_state_t;


#define CAN_ID_DATA_LEN 512
typedef struct {
    uint32_t can_id;
    uint8_t data_buffers[CAN_ID_DATA_LEN];
    volatile int head;
    volatile int tail;
    void* state;
} can_id_rvce_buffer_t;

can_id_rvce_buffer_t* find_can_entry_by_id(uint32_t id);
int can_id_buffer_read_bit(can_id_rvce_buffer_t *ring_buf , uint8_t *data);

void can_init(void);
int can_send_data_private(uint32_t can_id, uint16_t cmd,uint8_t *data, int len);
int can_rcve_data_private(uint32_t can_id,uint16_t cmd, uint8_t *data, int *len, int timeout_ms);

#endif
