#include "can_protocol.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

private_data_state_t private_data_state[2];
can_id_rvce_buffer_t can_id_rvce_buffer[] = {
    {.can_id = TWAI_DATA_ID, .state = &private_data_state[0]},
    {.can_id = TWAI_HEARTBEAT_ID, .state = &private_data_state[1]},
    {.can_id = 0x0, .state = NULL}
};

can_id_rvce_buffer_t* find_can_entry_by_id(uint32_t id)
{
    for (int i = 0; i < sizeof(can_id_rvce_buffer)/sizeof(can_id_rvce_buffer_t); i++) {
        if (can_id_rvce_buffer[i].can_id == id) {
            return &can_id_rvce_buffer[i];
        }
    }
    return NULL;
}

/**
 * @brief 写入数据到缓冲区
 * 
 * @param ring_buf 缓冲区
 * @param data 数据
 * @return int 0 成功，-1 缓冲区已满
 */
static int can_id_buffer_write_bit(can_id_rvce_buffer_t *ring_buf , uint8_t data)
{
    int next_head = (ring_buf->head + 1) % CAN_ID_DATA_LEN;
    if (next_head == ring_buf->tail) {
        return -1; // 缓冲区已满
    }
    ring_buf->data_buffers[ring_buf->head] = data;
    ring_buf->head = next_head;
    return 0;
}

/**
 * @brief 写入数据到缓冲区
 * 
 * @param frame 数据帧
 * @return int 0 成功，-1 缓冲区已满
 */
static int can_id_buffer_write(twai_frame_t *frame)
{
    can_id_rvce_buffer_t *entry = find_can_entry_by_id(frame->header.id);
    if (entry == NULL) {
        return -1;  // 未找到该ID对应的缓冲区
    }
    int used = (entry->head >= entry->tail) ? (entry->head - entry->tail) : (CAN_ID_DATA_LEN - entry->tail + entry->head);
    int free_space = CAN_ID_DATA_LEN - 1 - used;
    if (frame->header.dlc > free_space) {
        return -1;  // 数据帧长度超过缓冲区剩余空间
    }
    for (int i = 0; i < frame->header.dlc; i++) {
        can_id_buffer_write_bit(entry, frame->buffer[i]);
    }
    return 0;
}

/**
 * @brief 读取缓冲区数据
 * 
 * @param ring_buf 缓冲区
 * @param data 数据
 * @return int 0 读取成功，-1 缓冲区已空
 */
int can_id_buffer_read_bit(can_id_rvce_buffer_t *ring_buf , uint8_t *data)
{
    int next_tail = (ring_buf->tail + 1) % CAN_ID_DATA_LEN;
    if (ring_buf->tail == ring_buf->head) {
        return -1; // 缓冲区已空
    }
    *data = ring_buf->data_buffers[ring_buf->tail];
    ring_buf->tail = next_tail;
    return 0;
}

void can_task(void *pvParameter)
{
    twai_frame_t frame;
    while (1)
    {
        if (ulTaskNotifyTake(pdTRUE, portMAX_DELAY)) {
            while (twai_node_receive(&frame))
            {
                // ESP_LOGI(TWAI_TAG, "CAN ID: 0x%03X, LEN : %d, Data: %02X %02X %02X %02X %02X %02X %02X %02X",frame.header.id,frame.buffer_len ,frame.buffer[0],frame.buffer[1],frame.buffer[2],frame.buffer[3],frame.buffer[4],frame.buffer[5],frame.buffer[6],frame.buffer[7]);
                if (can_id_buffer_write(&frame))
                {
                    ESP_LOGI(TWAI_TAG, "缓冲区已满/未知ID, CAN ID: 0x%03X",frame.header.id);
                }
            }
        }
    }
}

extern TaskHandle_t xCanTaskHandle;
void can_init(void) 
{
    can_bus_init();
    xTaskCreatePinnedToCore(can_task, "CAN Task", 4096, NULL, 5, &xCanTaskHandle, 0);
}