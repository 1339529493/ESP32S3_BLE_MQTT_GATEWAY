#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "nvs_flash.h"
#include <stdio.h>
#include <string.h>
#include "wifi.h"
#include "esp_sntp.h"
#include "gw_log.h"

const char *WIFI_TAG = "static_ip";
/* 链接wifi名称 */
#define DEFAULT_SSID "LinksField"
/* wifi密码 */
#define DEFAULT_PWD "linksfield2023"
/* 事件标志 */
static EventGroupHandle_t wifi_event;
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT BIT1

#define RECONNECT_MAX_RETRY_NUMBER 20

char lcd_buff[100] = {0};

/* WIFI默认配置 */
#define WIFICONFIG() {                            \
    .sta = {                                      \
        .ssid = DEFAULT_SSID,                     \
        .password = DEFAULT_PWD,                  \
        .threshold.authmode = WIFI_AUTH_WPA2_PSK, \
    },                                            \
}
#define DEFAULT_SCAN_LIST_SIZE  12

// 设置时区：北京时间 UTC+8
static void set_timezone(void) {
    setenv("TZ", "CST-8", 1);   // 1. 设置环境变量 TZ 为 "CST-8" (中国标准时间, UTC+8)
    tzset();                    // 2. 让系统立即生效这个时区设置
}

// 初始化SNTP
static void initialize_sntp(void) {
    LOGI(WIFI_TAG, "Initializing SNTP");
    sntp_setoperatingmode(SNTP_OPMODE_POLL);
    
    // 配置多个NTP服务器（冗余）
    sntp_setservername(0, "cn.pool.ntp.org");
    sntp_setservername(1, "time.apple.com");
    sntp_setservername(2, "ntp.aliyun.com");
    
    sntp_init();
}

// 等待时间同步完成
static void obtain_time(void) {
    initialize_sntp();
    
    // 等待同步
    int retry = 0;
    const int retry_count = 15;
    while (sntp_get_sync_status() != SNTP_SYNC_STATUS_COMPLETED && retry++ < retry_count) {
        LOGI(WIFI_TAG, "Waiting for time sync... (%d/%d)", retry, retry_count);
        vTaskDelay(2000 / portTICK_PERIOD_MS);
    }
    
    if (retry >= retry_count) {
        LOGW(WIFI_TAG, "Time sync failed after %d retries", retry_count);
    } else {
        LOGI(WIFI_TAG, "Time sync completed!");
    }
    
    set_timezone();
}

/**
 * @brief       WIFI链接糊掉函数
 * @param       arg:传入网卡控制块
 * @param       event_base:WIFI事件
 * @param       event_id:事件ID
 * @param       event_data:事件数据
 * @retval      无
 */
static void wifi_event_handler(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data)
{
    static int s_retry_num = 0;
    gateway_event_t evt;

    /* 扫描到要连接的WIFI事件 */
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START)
    {
        update_wifi_status(STATUS_CONNECTING);
        esp_wifi_connect();
    }
    /* 连接WIFI事件 (表示 Wi-Fi 链路已打通,但还没有获取到 IP 地址)*/
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_CONNECTED)
    {
        update_wifi_status(STATUS_CONNECTING);
    }
    /* 连接WIFI失败事件 */
    else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED)
    {
        // 1. 构造停止 MQTT 的事件
        gateway_event_create_ref(&evt, MODULE_ID_WIFI, MODULE_ID_MQTT, CMD_WIFI_TO_MQTT_STOP, NULL, 0);
        // 2. 发送给 MQTT 模块
        // 注意：这里使用 send，如果队列满可能会失败，但对于控制指令通常没问题
        gateway_event_send(MODULE_ID_MQTT, &evt, pdMS_TO_TICKS(100));

        /* 尝试连接 20次重连尝试*/
        if (s_retry_num < RECONNECT_MAX_RETRY_NUMBER)
        {
            if (!s_retry_num) update_wifi_status(STATUS_RECONNECT);
            esp_wifi_connect();
            s_retry_num++;
            LOGI(WIFI_TAG, "retry to connect to the AP");
        }
        else
        {
            update_wifi_status(STATUS_DISCONNECTED);
            xEventGroupSetBits(wifi_event, WIFI_FAIL_BIT);
        }

        LOGI(WIFI_TAG, "connect to the AP fail");
    }
    /* 工作站从连接的AP获得IP */
    else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP)
    {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
        LOGI(WIFI_TAG, "static ip:" IPSTR, IP2STR(&event->ip_info.ip));
        s_retry_num = 0;
        update_wifi_status(STATUS_CONNECTED);
        // 1. 构造启动 MQTT 的事件
        gateway_event_create_ref(&evt, MODULE_ID_WIFI, MODULE_ID_MQTT, CMD_WIFI_TO_MQTT_START, NULL, 0);
        // 2. 发送给 MQTT 模块
        gateway_event_send(MODULE_ID_MQTT, &evt, pdMS_TO_TICKS(100));

        obtain_time();
        xEventGroupSetBits(wifi_event, WIFI_CONNECTED_BIT);
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
    wifi_config_t wifi_config = WIFICONFIG();
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_set_config(ESP_IF_WIFI_STA, &wifi_config));        //设置要连接的wifi，内部扫描到指定wifi会触发事件，在事件中进行连接
    ESP_ERROR_CHECK(esp_wifi_start());
    // uint16_t number = DEFAULT_SCAN_LIST_SIZE;            //这部分为扫描显示部分
    // wifi_ap_record_t ap_info[DEFAULT_SCAN_LIST_SIZE];
    // uint16_t ap_count = 0;
    // memset(ap_info, 0, sizeof(ap_info));
    // /* 开始扫描附件的WIFI */
    // vTaskDelay(pdMS_TO_TICKS(500));     //等待wifi彻底启动
    // esp_wifi_scan_start(NULL, true);
    // /* 获取上次扫描中找到的AP数量 */
    // ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));
    // /* 获取上次扫描中找到的AP列表 */
    // ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&number, ap_info));
    // LOGI(WIFI_TAG, "Total APs scanned = %u", ap_count);
    // /* 下面是打印附件的WIFI信息 */
    // for (int i = 0; (i < DEFAULT_SCAN_LIST_SIZE) && (i < ap_count); i++)
    // {
    //     // sprintf(lcd_buff, "%s",ap_info[i].ssid);
    //     // spilcd_show_string(200, 20 * i, 240, 16, 16, lcd_buff, BLUE);
    //     LOGI(WIFI_TAG, "SSID \t\t%s", ap_info[i].ssid);
    //     LOGI(WIFI_TAG, "RSSI \t\t%d", ap_info[i].rssi);
    //     print_auth_mode(ap_info[i].authmode);
        
    //     if (ap_info[i].authmode != WIFI_AUTH_WEP)
    //     {
    //         print_cipher_type(ap_info[i].pairwise_cipher, ap_info[i].group_cipher);
    //     }

    //     LOGI(WIFI_TAG, "Channel \t\t%d\n", ap_info[i].primary);
    // }

    /* 等待链接成功后、ip生成 */
    EventBits_t bits = xEventGroupWaitBits(wifi_event,
                                           WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
                                           pdFALSE,
                                           pdFALSE,
                                           portMAX_DELAY);

    /* 判断连接事件 */
    if (bits & WIFI_CONNECTED_BIT)
    {
        LOGI(WIFI_TAG, "connected to ap SSID:%s password:%s",
                 DEFAULT_SSID, DEFAULT_PWD);
    }
    else if (bits & WIFI_FAIL_BIT)
    {
        update_wifi_status(STATUS_DISCONNECTED);
        LOGI(WIFI_TAG, "Failed to connect to SSID:%s, password:%s",
                 DEFAULT_SSID, DEFAULT_PWD);
    }
    else
    {
        LOGE(WIFI_TAG, "UNEXPECTED EVENT");
    }
}
