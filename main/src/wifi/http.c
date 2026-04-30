#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_http_client.h"

static const char *TAG = "HTTP_CLIENT";

// 你的巴法云私钥
#define BEMFA_UID "7d5aa18173e277db18ebc007c479c443"

// 事件处理函数，用于接收和管理服务器返回的数据块
static esp_err_t http_event_handler(esp_http_client_event_t *evt)
{
    switch (evt->event_id) {
        case HTTP_EVENT_ON_DATA:
            // 当收到数据块时，打印到控制台
            printf("%.*s", evt->data_len, (char*)evt->data);
            break;
        default:
            break;
    }
    return ESP_OK;
}

/**
 * 发送微信消息（使用GET方式）
 * @param device 设备名称（自定义，如 "ESP32_Gateway"）
 * @param message 要推送的消息内容
 * @return ESP_OK 成功，其他值失败
 */
esp_err_t send_wechat_msg_get(const char *device, const char *message)
{
    char url[512];
    snprintf(url, sizeof(url), 
             "http://apis.bemfa.com/vb/wechat/v1/wechatAlert?uid=%s&device=%s&message=%s",
             BEMFA_UID, device, message);
    
    ESP_LOGI(TAG, "请求URL: %s", url);
    
    // 配置HTTP客户端
    esp_http_client_config_t config = {
        .url = url,
        .event_handler = http_event_handler,
        .timeout_ms = 10000,
        .skip_cert_common_name_check = true,  // 跳过证书验证（测试用）
        .crt_bundle_attach = NULL,
        .cert_pem = NULL,
    };
    
    // 初始化客户端
    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (client == NULL) {
        ESP_LOGE(TAG, "HTTP 客户端初始化失败");
        return ESP_FAIL;
    }
    
    // 执行请求
    esp_err_t err = esp_http_client_perform(client);
    
    // 检查执行结果并获取状态码
    if (err == ESP_OK) {
        int status_code = esp_http_client_get_status_code(client);
        ESP_LOGI(TAG, "微信消息发送成功，状态码 = %d", status_code);
    } else {
        ESP_LOGE(TAG, "微信消息发送失败: %s", esp_err_to_name(err));
    }
    
    // 清理并释放资源
    esp_http_client_cleanup(client);
    return err;
}

/**
 * 发送微信消息（使用POST方式，推荐）
 * @param device 设备名称（自定义，如 "ESP32_Gateway"）
 * @param message 要推送的消息内容
 * @return ESP_OK 成功，其他值失败
 */
esp_err_t send_wechat_msg_post(const char *device, const char *message)
{
    char post_data[256];
    
    // 构建POST请求的JSON数据
    snprintf(post_data, sizeof(post_data),
             "{\"uid\":\"%s\",\"device\":\"%s\",\"message\":\"%s\"}",
             BEMFA_UID, device, message);
    
    ESP_LOGI(TAG, "POST数据: %s", post_data);
    
    // 配置HTTP客户端
    esp_http_client_config_t config = {
        .url = "http://apis.bemfa.com/vb/wechat/v1/wechatAlertJson",
        .event_handler = http_event_handler,
        .method = HTTP_METHOD_POST,
        .timeout_ms = 10000,
        .skip_cert_common_name_check = true,  // 跳过证书验证（测试用）
        .crt_bundle_attach = NULL,
        .cert_pem = NULL,
    };
    
    // 初始化客户端
    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (client == NULL) {
        ESP_LOGE(TAG, "HTTP 客户端初始化失败");
        return ESP_FAIL;
    }
    
    // 设置请求头
    esp_http_client_set_header(client, "Content-Type", "application/json");
    
    // 设置POST数据
    esp_http_client_set_post_field(client, post_data, strlen(post_data));
    
    // 执行请求
    esp_err_t err = esp_http_client_perform(client);
    
    // 检查执行结果并获取状态码
    if (err == ESP_OK) {
        int status_code = esp_http_client_get_status_code(client);
        ESP_LOGI(TAG, "微信消息发送成功，状态码 = %d", status_code);
    } else {
        ESP_LOGE(TAG, "微信消息发送失败: %s", esp_err_to_name(err));
    }
    
    // 清理并释放资源
    esp_http_client_cleanup(client);
    return err;
}

// 测试函数
void http_test(void)
{
    // 发送一条测试消息
    // device: 设备名称，可以自定义，比如 "ESP32_Gateway"
    // message: 要推送的消息内容
    esp_err_t ret = send_wechat_msg_post("ESP32_Gateway", "ESP32_enable");
    
    if (ret == ESP_OK) {
        ESP_LOGI(TAG, "微信消息推送测试成功！请检查手机微信是否收到消息。");
    } else {
        ESP_LOGE(TAG, "微信消息推送测试失败！");
    }
}