#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include <stdio.h>
#include <string.h>
#include "wifi.h"
#include "esp_sntp.h"
#include "gw_log.h"

const char *WIFI_TAG = "static_ip";

static EventGroupHandle_t wifi_event;
#define WIFI_START_BIT BIT0

#define RECONNECT_MAX_RETRY_NUMBER 2
#define BLE_SET_NET_FLAG -1
#define RESET_RETRYNUM 0
static int s_retry_num = RESET_RETRYNUM;
char ssid[32];
char pwd[64];

// 设置时区：北京时间 UTC+8
static void set_timezone(void) {
    setenv("TZ", "CST-8", 1);   // 1. 设置环境变量 TZ 为 "CST-8" (中国标准时间, UTC+8)
    tzset();                    // 2. 让系统立即生效这个时区设置
}

// 初始化SNTP
static int initialize_sntp(void) {
    static bool sntp_initialized = false;  // 静态标志位，只初始化一次
    if (sntp_initialized) {
        LOGI(WIFI_TAG, "SNTP already initialized, skipping");
        return 1;
    }
    LOGI(WIFI_TAG, "Initializing SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    
    // 配置多个NTP服务器（冗余）
    sntp_setservername(0, "cn.pool.ntp.org");
    sntp_setservername(1, "time.apple.com");
    sntp_setservername(2, "ntp.aliyun.com");
    
    // 设置同步间隔（单位：毫秒），默认1小时
    sntp_set_sync_interval(60 * 60 * 1000);  // 1 小时
    esp_sntp_init();
    sntp_initialized = true;
    return 0;
}

// 等待时间同步完成
static void obtain_time(void) {
    if (initialize_sntp())
    {
        esp_sntp_stop();  
        vTaskDelay(pdMS_TO_TICKS(500));
        esp_sntp_init(); 
    }

    // 等待同步
    int retry = 0;
    const int retry_count = 3;
    while (sntp_get_sync_status() != SNTP_SYNC_STATUS_COMPLETED && retry++ < retry_count) {
        LOGI(WIFI_TAG, "Waiting for time sync... (%d/%d)", retry, retry_count);
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
    
    if (retry >= retry_count) {
        LOGW(WIFI_TAG, "Time sync failed after %d retries", retry_count);
    } else {
        LOGI(WIFI_TAG, "Time sync completed!");
    }
    
    set_timezone();
}

/**
 * @brief       WIFI链接回调函数
 * @param       arg:传入网卡控制块
 * @param       event_base:WIFI事件
 * @param       event_id:事件ID
 * @param       event_data:事件数据
 * @retval      无
 */
static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    gateway_event_t evt;
    LOGD(WIFI_TAG, "event_id:%d", event_id);
    /* 扫描到要连接的WIFI事件 */
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        xEventGroupSetBits(wifi_event, WIFI_START_BIT);
    }
    /* 连接WIFI事件 (表示 Wi-Fi 链路已打通,但还没有获取到 IP 地址)*/
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED)
    {
        update_wifi_status(STATUS_CONNECTING);
    }
    /* 连接WIFI失败事件 */
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        wifi_event_sta_disconnected_t *disconn = (wifi_event_sta_disconnected_t*)event_data;
        LOGE(WIFI_TAG, "Wi-Fi断开! 原因码: %d", disconn->reason);

        /* 尝试连接 20次重连尝试*/
        if (s_retry_num < RECONNECT_MAX_RETRY_NUMBER && s_retry_num >= 0 )
        {
            if (!s_retry_num)
            {
                update_wifi_status(STATUS_RECONNECT);
                // 停止 MQTT 的事件
                GATEWAY_EVENT_INIT_CMD(&evt, MODULE_ID_WIFI, MODULE_ID_MQTT, CMD_WIFI_TO_MQTT_STOP, 0);
                gateway_event_send(MODULE_ID_MQTT, &evt, 0);
            }
            esp_wifi_connect();
            s_retry_num++;
            LOGI(WIFI_TAG, "retry to connect to the AP");
        }
        else
        {
            update_wifi_status(STATUS_DISCONNECTED);
        }

        LOGI(WIFI_TAG, "connect to the AP fail");
    }
    /* 工作站从连接的AP获得IP */
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        LOGI(WIFI_TAG, "static ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = RESET_RETRYNUM;
        update_wifi_status(STATUS_CONNECTED);
        // 1. 构造启动 MQTT 的事件
        GATEWAY_EVENT_INIT_CMD(&evt, MODULE_ID_WIFI, MODULE_ID_MQTT, CMD_WIFI_TO_MQTT_START, 0);
        // 2. 发送给 MQTT 模块
        gateway_event_send(MODULE_ID_MQTT, &evt, 0);

        obtain_time();
    }
}

/**
 * @brief       WIFI初始化
 * @param       无
 * @retval      无
 */
void wifi_sta_init(void)
{
    static esp_netif_t *sta_netif = NULL;
    wifi_event = xEventGroupCreate(); /* 创建一个事件标志组 */
    /* 网卡初始化 */
    ESP_ERROR_CHECK(esp_netif_init());
    /* 创建新的事件循环 */
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    sta_netif = esp_netif_create_default_wifi_sta();
    assert(sta_netif);
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL));
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    /* 等待链接成功后、ip生成 */
    EventBits_t bits = xEventGroupWaitBits(wifi_event,
                                           WIFI_START_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           portMAX_DELAY);
    /* 判断连接事件 */
    if (bits & WIFI_START_BIT)
    {
        LOGI(WIFI_TAG, "WIFI START");
    }
}

void wifi_task(void *pvParameter)
{
    LOGI(WIFI_TAG, "WIFI Task Started");
    gateway_event_t msg;
    wifi_config_t wifi_config = {0};
    esp_err_t err;
    wifi_sta_init(); 
    if (ssid[0] != 0 && pwd[0] != 0)    //后面存入flash空间中
    {
        LOGI(WIFI_TAG, "设备当前wifi, ssid : [%s] ,password : [%s]",ssid, pwd);
        memcpy(wifi_config.sta.ssid, ssid, sizeof(ssid));
        memcpy(wifi_config.sta.password, pwd, sizeof(pwd));
        wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
        ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));        //设置要连接的wifi，内部扫描到指定wifi会触发事件，在事件中进行连接
        esp_wifi_connect();
    }

    while(1) {
        if (gateway_event_receive(MODULE_ID_WIFI, &msg, portMAX_DELAY) == pdTRUE) {
            LOGI(WIFI_TAG, "WIFI EVT : Received from %d, len: %d",msg.src_id ,msg.data_len);            
            switch (msg.cmd_id) {                
                case CMD_BLE_TO_WIFI_PROVISION:
                    s_retry_num = BLE_SET_NET_FLAG;
                    memset(&wifi_config, 0, sizeof(wifi_config));
                    LOGD(WIFI_TAG, "msg : ssid : [%s] ,password : [%s]",((wifi_provision_info_t*)msg.data)->ssid, ((wifi_provision_info_t*)msg.data)->pwd);
                    memcpy(wifi_config.sta.ssid, ((wifi_provision_info_t*)msg.data)->ssid, sizeof(((wifi_provision_info_t*)msg.data)->ssid));
                    memcpy(wifi_config.sta.password, ((wifi_provision_info_t*)msg.data)->pwd, sizeof(((wifi_provision_info_t*)msg.data)->pwd));
                    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
                    // 3. 关键步骤：如果当前已连接或正在连接，先断开
                    wifi_ap_record_t ap_info;
                    esp_err_t ret = esp_wifi_sta_get_ap_info(&ap_info);
                    if (ret == ESP_OK) {
                        LOGI(WIFI_TAG, "WiFi is currently connected to [%s], disconnecting first...", ap_info.ssid);
                        esp_wifi_disconnect();
                        // 等待一小会儿让断开动作生效，避免立即重连时的状态竞争
                        vTaskDelay(pdMS_TO_TICKS(500)); 
                        s_retry_num = RESET_RETRYNUM;
                    } else {
                        LOGI(WIFI_TAG, "WiFi is not connected, proceeding to connect.");
                    }
                case CMD_BLE_TO_WIFI_CONNECT: 
                    LOGD(WIFI_TAG, "ssid : [%s] ,password : [%s]",wifi_config.sta.ssid, wifi_config.sta.password);
                    err = esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config);        //设置要连接的wifi，内部扫描到指定wifi会触发事件，在事件中进行连接
                    esp_wifi_connect();
                    break;                   
                default:
                    break;
            }
            gateway_event_free(&msg);
        }
    }
}