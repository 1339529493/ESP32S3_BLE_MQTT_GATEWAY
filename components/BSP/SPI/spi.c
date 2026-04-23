#include <string.h>

#include "spi.h"

/**
 * @brief 初始化 SPI2
 * @param 无
 * @retval 无
 */
void spi2_init(void)
{
    esp_err_t ret = 0;
    spi_bus_config_t spi_bus_conf = {0};
    /* SPI 总线配置 */
    spi_bus_conf.miso_io_num = SPI_MISO_GPIO_PIN; /* SPI_MISO 引脚 */
    spi_bus_conf.mosi_io_num = SPI_MOSI_GPIO_PIN; /* SPI_MOSI 引脚 */
    spi_bus_conf.sclk_io_num = SPI_CLK_GPIO_PIN;  /* SPI_SCLK 引脚 */
    spi_bus_conf.quadwp_io_num = -1;              /* SPI 写保护信号引脚，该引脚未使能 */
    spi_bus_conf.quadhd_io_num = -1;              /* SPI 保持信号引脚，该引脚未使能 */
    spi_bus_conf.max_transfer_sz = 320 * 240 * 2; /* 配置最大传输大小，以字节为单位 */
                                                  /* 初始化 SPI 总线 */
    ret = spi_bus_initialize(SPI2_HOST, &spi_bus_conf, SPI_DMA_CH_AUTO);
    ESP_ERROR_CHECK(ret); /* 校验参数值 */
}

/**
 * @brief SPI 发送命令
 * @param handle : SPI 句柄
 * @param cmd : 要发送命令
 * @retval 无
 */
void spi2_write_cmd(spi_device_handle_t handle, uint8_t cmd)
{
    esp_err_t ret;
    spi_transaction_t t = {0};
    t.length = 8;                                  /* 要传输的位数 一个字节 8 位 */
    t.tx_buffer = &cmd;                            /* 将命令填充进去 */
    ret = spi_device_polling_transmit(handle, &t); /* 开始传输 */
    ESP_ERROR_CHECK(ret);                          /* 一般不会有问题 */
}
/**
 * @brief SPI 发送数据
 * @param handle : SPI 句柄
 * @param data : 要发送的数据
 * @param len : 要发送的数据长度
 * @retval 无
 */
void spi2_write_data(spi_device_handle_t handle, const uint8_t *data, int len)
{
    esp_err_t ret;
    spi_transaction_t t = {0};
    if (len == 0)
    {
        return; /* 长度为 0 没有数据要传输 */
    }
    t.length = len * 8;                            /* 要传输的位数 一个字节 8 位 */
    t.tx_buffer = data;                            /* 将命令填充进去 */
    // ret = spi_device_polling_transmit(handle, &t); /* 开始传输 */   /* 轮询模式，阻塞式传输，短数据传输效率高 */
    if (len > 128)
    {
        ret = spi_device_transmit(handle, &t);
    }
    else
    {
        ret = spi_device_polling_transmit(handle, &t);
    } 
    ESP_ERROR_CHECK(ret);                          /* 一般不会有问题 */
}
/**
 * @brief SPI 处理数据
 * @param handle : SPI 句柄
 * @param data : 要发送的数据
 * @retval t.rx_data[0] : 接收到的数据
 */
uint8_t spi2_transfer_byte(spi_device_handle_t handle, uint8_t data)
{
    spi_transaction_t t;
    memset(&t, 0, sizeof(t));
    t.flags = SPI_TRANS_USE_TXDATA | SPI_TRANS_USE_RXDATA;
    t.length = 8;
    t.tx_data[0] = data;
    spi_device_transmit(handle, &t);
    return t.rx_data[0];
}