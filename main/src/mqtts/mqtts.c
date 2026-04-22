#include "mqtts.h"
#include <string.h>

#include "gw_log.h"

extern QueueHandle_t mqtt_to_ble_q;

static const char *MQTTS_TAG = "MQTT_EXAMPLE";
static esp_mqtt_client_handle_t mqtt_client = NULL;

/**
 * @brief       错误日记
 * @param       message     :错误消息
 * @param       error_code  :错误码
 * @retval      无
 */
static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0)
    {
        LOGE(MQTTS_TAG, "Last error %s: 0x%x", message, error_code);
    }
}

/**
 * @brief       注册接收MQTT事件的事件处理程序
 * @param       handler_args:注册到事件的用户数据
 * @param       base        :处理程序的事件库
 * @param       event_id    :接收到的事件的id
 * @param       event_data  :事件的数据
 * @retval      无
 */
static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    LOGD(MQTTS_TAG, "Event dispatched from event loop base=%s, event_id=%" PRIi32 "", base, event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;

    switch ((esp_mqtt_event_id_t)event_id)
    {
        case MQTT_EVENT_CONNECTED:      /* 连接事件 */
            /* 订阅主题 */
            msg_id = esp_mqtt_client_subscribe(client, DEVICE_SUBSCRIBE, 0);
            LOGI(MQTTS_TAG, "sent subscribe successful, msg_id=%d", msg_id);
            break;
        case MQTT_EVENT_DISCONNECTED:   /* 断开连接事件 */

            break;

        case MQTT_EVENT_SUBSCRIBED:     /* 取消事件 */
            LOGI(MQTTS_TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
            msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
            LOGI(MQTTS_TAG, "sent publish successful, msg_id=%d", msg_id);
            break;
        case MQTT_EVENT_UNSUBSCRIBED:   /* 取消订阅事件 */
            LOGI(MQTTS_TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_PUBLISHED:      /* 发布事件 */
            LOGI(MQTTS_TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
            break;
        case MQTT_EVENT_DATA:           /* 接收数据事件 */
            printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
            printf("DATA=%.*s\r\n", event->data_len, event->data);
            if (mqtt_to_ble_q != NULL) {
                gateway_msg_t msg;
                msg.type = MSG_TYPE_MQTT_CMD;
                msg.len = event->data_len > MAX_PAYLOAD_LEN ? MAX_PAYLOAD_LEN : event->data_len;
                memcpy(msg.payload, event->data, msg.len);
                msg.timestamp = xTaskGetTickCount();

                if (xQueueSend(mqtt_to_ble_q, &msg, pdMS_TO_TICKS(100)) != pdTRUE) {
                    LOGW(MQTTS_TAG, "MQTT to BLE Queue Full!");
                }
            }
            break;
        case MQTT_EVENT_ERROR:

            if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT)
            {
                log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
                log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
                log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
                LOGI(MQTTS_TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));
            }

            break;
        default:
            LOGI(MQTTS_TAG, "Other event id:%d", event->event_id);
            break;
    }
}

extern const char *get_cert();          ///获取腾讯云个人mosquitto服务的证书
/**
 * @brief       lwip_demo进程
 * @param       无
 * @retval      无
 */
void mqtts_init(void)
{
    /* 设置客户端的信息量 */ ;
    esp_mqtt_client_config_t mqtt_cfg = {
    .broker.address.hostname = HOST_NAME,                   /* MQTT地址 */
    .broker.address.port = HOST_PORT,                       /* MQTT端口号 */
    // .broker.address.transport = MQTT_TRANSPORT_OVER_TCP,    /* TCP模式 */
    .broker.address.transport = MQTT_TRANSPORT_OVER_SSL,
    .credentials.client_id = CLIENT_ID,                     /* 设备名称 */
    .credentials.username = (char*)USER_NAME,               /* 产品ID */
    .credentials.authentication.password = PASSWORD,        /* 计算出来的密码 */
    .broker.verification.certificate = get_cert(),
    .broker.verification.certificate_len = strlen(get_cert()) + 1,  //strlen是不包含结尾符/0的，但是证书长度需要/0来确定边界
    };

    mqtt_client = esp_mqtt_client_init(&mqtt_cfg);

    esp_mqtt_client_register_event(mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(mqtt_client);      /* 启动MQTT */
}

int mqtts_publish(const char *data, int len, int qos, int retain)
{
    return esp_mqtt_client_publish(mqtt_client,DEVICE_PUBLISH,data,len,qos,retain);
}

