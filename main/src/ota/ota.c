#include <string.h>
#include <inttypes.h>
#include "esp_ota_ops.h"
#include "esp_app_format.h"
#include "esp_http_client.h"
#include "esp_flash_partitions.h"
#include "esp_partition.h"
#include "nvs.h"
#include "nvs_flash.h"
#include "gw_log.h"
#include "esp_task_wdt.h"

#define CONFIG_EXAMPLE_SKIP_VERSION_CHECK     // 设置跳过版本检查，同git版本也下载
#define CONFIG_EXAMPLE_SKIP_COMMON_NAME_CHECK // 设置跳过证书CN检查
#define CONFIG_EXAMPLE_FIRMWARE_UPG_URL "http://81.70.100.183/ota/esp_ble_mqtt_gateway_test_ota1.bin"
#define CONFIG_EXAMPLE_OTA_RECV_TIMEOUT 256

#define NVS_NAMESPACE "nvs"
#define NVS_KEY_BOOT_COUNT "boot_cnt"
#define REQUIRED_SUCCESSFUL_BOOTS 3 // 连续成功启动3次才标记为有效

#define BUFFSIZE 1024
#define HASH_LEN 32 /* SHA-256 摘要长度 */

static const char *OTA_TAG = "ota";
/* 准备写入 Flash 的 OTA 数据缓冲区 */
static char ota_write_data[BUFFSIZE + 1] = {0};
// extern const uint8_t server_cert_pem_start[] asm("_binary_ca_cert_pem_start");
// extern const uint8_t server_cert_pem_end[] asm("_binary_ca_cert_pem_end");

// static void __attribute__((noreturn)) task_fatal_error(void)
// {
//     LOGE(OTA_TAG, "Exiting task due to fatal error...");
//     (void)vTaskDelete(NULL);

//     while (1)
//     {
//         ;
//     }
// }

static void task_fatal_error(void)
{
    LOGE(OTA_TAG, "Exiting task due to fatal error...");
    return;
}

static void ota_safe_cleanup(esp_ota_handle_t handle, esp_http_client_handle_t client)
{
    if (handle != 0)
    {
        esp_ota_abort(handle);
    }
    if (client != NULL)
    {
        esp_http_client_close(client);
        esp_http_client_cleanup(client);
    }
}

#define CHECK_ERR(err, msg, handle, client)                        \
    do                                                             \
    {                                                              \
        esp_err_t _err = (err);                                    \
        if (_err != ESP_OK)                                        \
        {                                                          \
            LOGE(OTA_TAG, "%s: %s", (msg), esp_err_to_name(_err)); \
            ota_safe_cleanup((handle), (client));                  \
            task_fatal_error();                                    \
        }                                                          \
    } while (0)

#define CHECK_NULL(ptr, msg, handle, client)      \
    do                                            \
    {                                             \
        if ((ptr) == NULL)                        \
        {                                         \
            LOGE(OTA_TAG, "%s", (msg));           \
            ota_safe_cleanup((handle), (client)); \
            task_fatal_error();                   \
        }                                         \
    } while (0)

#define CHECK_COND(cond, msg, handle, client)     \
    do                                            \
    {                                             \
        if ((cond))                               \
        {                                         \
            LOGE(OTA_TAG, "%s", (msg));           \
            ota_safe_cleanup((handle), (client)); \
            task_fatal_error();                   \
        }                                         \
    } while (0)

static void print_sha256(const uint8_t *image_hash, const char *label)
{
    char hash_print[HASH_LEN * 2 + 1];
    hash_print[HASH_LEN * 2] = 0;
    for (int i = 0; i < HASH_LEN; ++i)
    {
        sprintf(&hash_print[i * 2], "%02x", image_hash[i]);
    }
    LOGI(OTA_TAG, "%s: %s", label, hash_print);
}

static void diagnostic(void)
{
    nvs_handle_t my_handle;
    esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &my_handle);
    if (err != ESP_OK)
    {
        LOGE(OTA_TAG, "NVS Open Failed: %s", esp_err_to_name(err));
        esp_ota_mark_app_invalid_rollback_and_reboot();
    }

    uint32_t boot_count = 0;
    err = nvs_get_u32(my_handle, NVS_KEY_BOOT_COUNT, &boot_count);
    if (err == ESP_ERR_NVS_NOT_FOUND)
    {
        boot_count = 1; // 第一次启动
    }
    else if (err == ESP_OK)
    {
        boot_count++;
    }
    else
    {
        LOGE(OTA_TAG, "NVS Read Failed, rolling back");
        nvs_close(my_handle);
        esp_ota_mark_app_invalid_rollback_and_reboot();
    }

    LOGI(OTA_TAG, "Current boot count for new firmware: %lu", boot_count);

    if (boot_count >= REQUIRED_SUCCESSFUL_BOOTS)
    {
        // 已经连续成功启动了 N 次，认为固件稳定
        LOGI(OTA_TAG, "Firmware verified stable. Marking as valid.");
        esp_ota_mark_app_valid_cancel_rollback();
        nvs_erase_key(my_handle, NVS_KEY_BOOT_COUNT); // 清理计数器
    }
    else
    {
        // 还没达到次数，保存计数器并重启（或者继续运行，取决于你的策略）
        // 注意：这里通常不需要重启，只是不标记 Valid。
        // 如果在这期间发生了看门狗复位，Bootloader 会发现它还是 PENDING 且没被标记 Valid，从而回滚。
        nvs_set_u32(my_handle, NVS_KEY_BOOT_COUNT, boot_count);
        nvs_commit(my_handle);
        LOGW(OTA_TAG, "Waiting for more successful boots to verify stability.");
    }

    nvs_close(my_handle);
}

void ota_verify_and_check_integrity()
{
    uint8_t sha_256[HASH_LEN] = {0};
    esp_partition_t partition;

    // 获取分区表的 sha256 摘要
    partition.address = ESP_PARTITION_TABLE_OFFSET;
    partition.size = ESP_PARTITION_TABLE_MAX_LEN;
    partition.type = ESP_PARTITION_TYPE_DATA;
    esp_partition_get_sha256(&partition, sha_256);
    print_sha256(sha_256, "分区表的 SHA-256: ");

    // 获取 bootloader 的 sha256 摘要
    partition.address = ESP_BOOTLOADER_OFFSET;
    partition.size = ESP_PARTITION_TABLE_OFFSET;
    partition.type = ESP_PARTITION_TYPE_APP;
    esp_partition_get_sha256(&partition, sha_256);
    print_sha256(sha_256, "Bootloader 的 SHA-256: ");

    // 获取当前运行分区的 sha256 摘要
    esp_partition_get_sha256(esp_ota_get_running_partition(), sha_256);
    print_sha256(sha_256, "当前固件的 SHA-256: ");

    const esp_partition_t *running = esp_ota_get_running_partition();
    esp_ota_img_states_t ota_state;
    if (esp_ota_get_state_partition(running, &ota_state) == ESP_OK)
    {
        if (ota_state == ESP_OTA_IMG_PENDING_VERIFY)
        {
            diagnostic();
        }
        else if (ota_state == ESP_OTA_IMG_VALID)
        {
            nvs_handle_t my_handle;
            esp_err_t err = nvs_open(NVS_NAMESPACE, NVS_READWRITE, &my_handle);
            if (err != ESP_OK)
            {
                LOGE(OTA_TAG, "NVS Open Failed: %s", esp_err_to_name(err));
            }
            // 检查是否有残留的 boot_count
            uint32_t boot_count = 0;
            if (nvs_get_u32(my_handle, NVS_KEY_BOOT_COUNT, &boot_count) == ESP_OK)
            {
                LOGI(OTA_TAG, "Cleaning up stale NVS ota_status key");
                nvs_erase_key(my_handle, NVS_KEY_BOOT_COUNT);
                nvs_commit(my_handle);
            }
            nvs_close(my_handle);
        }
    }
}
void ota_start(uint8_t *url)
{
    esp_err_t err;
    /* 更新句柄：由 esp_ota_begin() 设置，必须通过 esp_ota_end() 释放 */
    esp_ota_handle_t update_handle = 0;
    const esp_partition_t *update_partition = NULL;

    LOGI(OTA_TAG, "开始 OTA 任务");

    const esp_partition_t *configured = esp_ota_get_boot_partition();
    const esp_partition_t *running = esp_ota_get_running_partition();

    if (configured != running)
    {
        LOGW(OTA_TAG, "配置的 OTA 启动分区位于偏移量 0x%08" PRIx32 "，但当前从偏移量 0x%08" PRIx32 " 运行", configured->address, running->address);
        LOGW(OTA_TAG, "(如果 OTA 启动数据或首选启动镜像以某种方式损坏，可能会发生这种情况。)");
    }
    LOGI(OTA_TAG, "正在运行的分区类型 %d 子类型 %d (偏移量 0x%08" PRIx32 ")", running->type, running->subtype, running->address);

    esp_http_client_config_t config = {
        .url = CONFIG_EXAMPLE_FIRMWARE_UPG_URL,
        .cert_pem = NULL,
        .crt_bundle_attach = NULL,
        .timeout_ms = CONFIG_EXAMPLE_OTA_RECV_TIMEOUT,
        .keep_alive_enable = true,
#ifdef CONFIG_EXAMPLE_SKIP_COMMON_NAME_CHECK
        .skip_cert_common_name_check = true,
#endif
    };

    esp_http_client_handle_t client = esp_http_client_init(&config);
    CHECK_NULL(client, "初始化 HTTP 配置失败", update_handle, client);
    err = esp_http_client_open(client, 0);
    CHECK_ERR(err, "打开 HTTP 连接失败: ", update_handle, client);
    esp_http_client_fetch_headers(client);
    // 步骤 1: 获取可用的 OTA 分区
    update_partition = esp_ota_get_next_update_partition(NULL);
    CHECK_NULL(update_partition, "无法获取下一个更新分区", update_handle, client);
    LOGI(OTA_TAG, "写入子类型为 %d 偏移量为 0x%" PRIx32 " 的分区", update_partition->subtype, update_partition->address);

    int binary_file_length = 0;
    /* 处理所有接收到的数据包 */
    bool image_header_was_checked = false;
    while (1)
    {
        // esp_task_wdt_reset(); // 重置WDT
        int data_read = esp_http_client_read(client, ota_write_data, BUFFSIZE);
        if (data_read < 0)
        {
            CHECK_COND(data_read, "错误: HTTP 数据读取错误", update_handle, client);
        }
        else if (data_read > 0)
        {
            if (image_header_was_checked == false)
            {
                esp_app_desc_t new_app_info;
                if (data_read > sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t) + sizeof(esp_app_desc_t))
                {
                    // 检查当前版本与正在下载的版本
                    memcpy(&new_app_info, &ota_write_data[sizeof(esp_image_header_t) + sizeof(esp_image_segment_header_t)], sizeof(esp_app_desc_t));
                    LOGI(OTA_TAG, "新固件版本: %s", new_app_info.version);

                    esp_app_desc_t running_app_info; // 获取running分区(当前运行固件)的版本
                    if (esp_ota_get_partition_description(running, &running_app_info) == ESP_OK)
                    {
                        LOGI(OTA_TAG, "当前运行的固件版本: %s", running_app_info.version);
                    }

                    // 设备回滚之后调用，获取上一个无效分区，不然返回NULL
                    const esp_partition_t *last_invalid_app = esp_ota_get_last_invalid_partition();
                    esp_app_desc_t invalid_app_info;
                    if (esp_ota_get_partition_description(last_invalid_app, &invalid_app_info) == ESP_OK)
                    {
                        LOGI(OTA_TAG, "上一个无效固件版本: %s", invalid_app_info.version);
                    }

                    // 检查当前版本与最后一个无效分区
                    if (last_invalid_app != NULL)
                    {
                        if (memcmp(invalid_app_info.version, new_app_info.version, sizeof(new_app_info.version)) == 0)
                        {
                            LOGW(OTA_TAG, "新版本与无效版本相同");
                            LOGW(OTA_TAG, "此前，曾尝试启动版本为 %s 的固件，但失败了", invalid_app_info.version);
                            CHECK_COND(1, "固件已回滚到之前的版本", update_handle, client);
                        }
                    }
#ifndef CONFIG_EXAMPLE_SKIP_VERSION_CHECK
                    if (memcmp(new_app_info.version, running_app_info.version, sizeof(new_app_info.version)) == 0)
                    {
                        CHECK_COND(1, "当前运行版本与新版本相同。我们将不会继续更新", update_handle, client)
                    }
#endif

                    image_header_was_checked = true;
                    // 步骤 2: 开始 OTA 操作，获取句柄// OTA_WITH_SEQUENTIAL_WRITES 表示顺序写入，适合从网络流式接收数据
                    err = esp_ota_begin(update_partition, OTA_WITH_SEQUENTIAL_WRITES, &update_handle);
                    CHECK_ERR(err, "esp_ota_begin 错误: ", update_handle, client);
                    LOGI(OTA_TAG, "esp_ota_begin 成功");
                }
                else
                {
                    CHECK_COND(data_read, "接收到的数据包长度过短", update_handle, client);
                }
            }
            // esp_ota_write可以多次调用，会接着上次写入偏移位继续写入
            err = esp_ota_write(update_handle, (const void *)ota_write_data, data_read);
            CHECK_ERR(err, "esp_ota_write 错误: ", update_handle, client);
            binary_file_length += data_read;
            LOGD(OTA_TAG, "已写入镜像长度 %d", binary_file_length);
        }
        else if (data_read == 0)
        {
            if (errno == ECONNRESET || errno == ENOTCONN)
            {
                LOGE(OTA_TAG, "连接关闭, errno = %d", errno);
                break;
            }
            if (esp_http_client_is_complete_data_received(client) == true) // 检查是否已完整无误地读取了 HTTP 响应的全部数据
            {
                LOGI(OTA_TAG, "连接关闭");
                break;
            }
        }
    }
    LOGI(OTA_TAG, "总写入二进制数据长度: %d", binary_file_length);
    CHECK_COND(!esp_http_client_is_complete_data_received(client), "接收完整文件时出错", update_handle, client);

    err = esp_ota_end(update_handle);
    CHECK_ERR(err, "esp_ota_end 错误: ", 0, client);

    // 下次启动时，从update_partition这个新分区加载并运行固件
    err = esp_ota_set_boot_partition(update_partition);
    CHECK_ERR(err, "esp_ota_set_boot_partition 错误: ", update_handle, client);
    LOGI(OTA_TAG, "准备重启系统!");
    esp_restart();
    return;
}

// 手动切换到备份分区
void manual_rollback(void)
{
    esp_ota_img_states_t ota_state;
    const esp_partition_t *running = esp_ota_get_running_partition();
    const esp_partition_t *next = esp_ota_get_next_update_partition(NULL);

    if (next != NULL && next != running)
    {
        // 检查备用分区是否有效
        if (esp_ota_get_state_partition(next, &ota_state) == ESP_OK)
        {
            if (ota_state == ESP_OTA_IMG_VALID || ota_state == ESP_OTA_IMG_PENDING_VERIFY)
            {
                esp_ota_set_boot_partition(next);
                esp_restart(); // 重启后运行旧版本
            }
        }
    }
    LOGE(OTA_TAG, "没有可用的备用分区");
}
