#ifndef __MQTTS_H
#define __MQTTS_H

#include <string.h>
#include <sys/socket.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "lwip/apps/mqtt.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "mqtt_client.h"
#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"
#include "esp_netif.h"

#include "gateway_msg.h"

//腾讯云个人mosquitto 服务
#define HOST_NAME           "lh-world.icu"                  
#define HOST_PORT           8883                                                                
#define CLIENT_ID           "esp32client"                                                       /* 客户端ID */
#define USER_NAME           "lh_mqtts"                                                          /* 客户端用户名 */
#define PASSWORD            "1993047286"                                                         /* 密码 */
/* 发布与订阅 */
#define DEVICE_PUBLISH      "user1/topic"                                                       /* 发布 */
#define DEVICE_SUBSCRIBE    "user1/topic"                                                       /* 订阅 */

void mqtts_init(void);
int mqtts_publish(const char *data, int len, int qos, int retain);
// void mqtts_set_queue(QueueHandle_t to_ble_q);
#endif
