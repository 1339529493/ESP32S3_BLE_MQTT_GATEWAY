#ifndef __SPI_H
#define __SPI_H

#include "driver/sdspi_host.h"
#include "esp_err.h"

/* 引脚定义 */
#define SPI_MOSI_GPIO_PIN GPIO_NUM_11 /* SPI2_MOSI */
#define SPI_CLK_GPIO_PIN GPIO_NUM_12  /* SPI2_CLK */
#define SPI_MISO_GPIO_PIN GPIO_NUM_13 /* SPI2_MISO */

void spi2_init(void);
void spi2_write_cmd(spi_device_handle_t handle, uint8_t cmd);
void spi2_write_data(spi_device_handle_t handle, const uint8_t *data, int len);
uint8_t spi2_transfer_byte(spi_device_handle_t handle, uint8_t data);
;
#endif
