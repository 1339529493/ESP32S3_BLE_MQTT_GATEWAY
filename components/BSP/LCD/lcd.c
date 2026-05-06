#include "lcd.h"

DRAM_ATTR uint8_t refresh_done_flag = 0;    
esp_lcd_panel_handle_t panel_handle = NULL;
_spilcd_dev spilcddev;
#define SPI_LCD_TYPE    1           /* SPI接口屏幕类型（1：2.4寸SPILCD  0：1.3寸SPILCD） */ 

/* LCD的宽和高定义 */
#if SPI_LCD_TYPE                    /* 2.4寸SPI_LCD屏幕 */
uint16_t spilcd_width  = 320;       /* 屏幕的宽度 320(横屏) */
uint16_t spilcd_height = 240;       /* 屏幕的宽度 240(横屏) */
#else
uint16_t spilcd_width  = 240;       /* 屏幕的宽度 240(横屏) */
uint16_t spilcd_height = 240;       /* 屏幕的宽度 240(横屏) */
#endif                              /* 1.3寸SPI_LCD屏幕 */

static bool notify_lcd_flush_ready(esp_lcd_panel_io_handle_t panel_io, esp_lcd_panel_io_event_data_t *edata, void *user_ctx)
{
    refresh_done_flag = 1;
    return false;
}

/**
 * @brief       spilcd初始化
 * @param       无
 * @retval      ESP_OK:初始化成功
 */
esp_err_t spilcd_init(void)
{
    LCD_RST(0);
    vTaskDelay(pdMS_TO_TICKS(100));
    LCD_RST(1);
    vTaskDelay(pdMS_TO_TICKS(100));

    esp_lcd_panel_io_handle_t io_handle = NULL;     /* LCD IO设备句柄 */
    /* spi配置 */
    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num         = LCD_DC_PIN,          /* DC IO */
        .cs_gpio_num         = LCD_CS_PIN,          /* CS IO */
        .pclk_hz             = 60 * 1000 * 1000,    /* PCLK为60MHz */
        .lcd_cmd_bits        = 8,                   /* 命令位宽 */
        .lcd_param_bits      = 8,                   /* LCD参数位宽 */
        .spi_mode            = 0,                   /* SPI模式 */
        .trans_queue_depth   = 7,                   /* 传输队列 */
    };
    /* 将LCD设备挂载至SPI总线上 */
    ESP_ERROR_CHECK(esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST, &io_config, &io_handle));

    spilcddev.pheight = spilcd_height;  /* 高度 */
    spilcddev.pwidth  = spilcd_width;   /* 宽度 */

    /* LCD设备配置 */
    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = LCD_RST_PIN,                  /* RTS IO */
        .rgb_ele_order  = COLOR_RGB_ELEMENT_ORDER_RGB,  /* RGB颜色格式 */
        .bits_per_pixel = 16,                           /* 颜色深度 */
        .data_endian    = LCD_RGB_DATA_ENDIAN_BIG,      /* 大端顺序 */
    };
    /* 为ST7789创建LCD面板句柄，并指定SPI IO设备句柄 */
    ESP_ERROR_CHECK(esp_lcd_new_panel_st7789(io_handle, &panel_config, &panel_handle));
    /* 复位LCD */
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    /* 反显 */
    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel_handle, true));
    /* 初始化LCD句柄 */
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
    /* 打开屏幕 */
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));

    const esp_lcd_panel_io_callbacks_t cbs = {
        .on_color_trans_done = notify_lcd_flush_ready,
    };
    /* 注册屏幕刷新完成回调函数 */
    ESP_ERROR_CHECK(esp_lcd_panel_io_register_event_callbacks(io_handle, &cbs, NULL));

    spilcd_display_dir(0);      /* 横屏显示 */
    
    spilcd_clear(WHITE);        /* 清屏 */
    LCD_PWR(1);
    return ESP_OK;
}

/**
 * @brief       设置屏幕方向
 * @param       dir: 0为竖屏，1为横屏
 * @retval      无
 */
void spilcd_display_dir(uint8_t dir)
{
    spilcddev.dir = dir;

    if (spilcddev.dir == 0)         /* 竖屏 */
    {
        spilcddev.width = spilcddev.pheight;
        spilcddev.height = spilcddev.pwidth;
        esp_lcd_panel_swap_xy(panel_handle, false);
        esp_lcd_panel_mirror(panel_handle, false, false);

        if (SPI_LCD_TYPE == 0)
        {
            esp_lcd_panel_set_gap(panel_handle, 0, 80);
        }
    }
    else if (spilcddev.dir == 1)    /* 横屏 */
    {
        spilcddev.width = spilcddev.pwidth;
        spilcddev.height = spilcddev.pheight;
        esp_lcd_panel_swap_xy(panel_handle, true);
        esp_lcd_panel_mirror(panel_handle, true, false);

        if (SPI_LCD_TYPE == 0)
        {
            esp_lcd_panel_set_gap(panel_handle, 80, 0);
        }
    }
}

/**
 * @brief       清屏
 * @param       color: 颜色值
 * @retval      无
 */
void spilcd_clear(uint16_t color)
{
    /* 以40行作为缓冲,提高速率,若出现内存不足,可以减少缓冲行数 */
    uint16_t *buffer = heap_caps_malloc(spilcddev.width * sizeof(uint16_t) * 40, MALLOC_CAP_SPIRAM);
    uint16_t color_tmp = ((color & 0x00FF) << 8) | ((color & 0xFF00) >> 8);   /* 需要转换一下颜色值 */
    if (NULL == buffer)
    {
        ESP_LOGE("TAG", "Memory for bitmap is not enough");
    }
    else
    {
        for (uint32_t i = 0; i < spilcddev.width * 40; i++)
        {
            buffer[i] = color_tmp;
        }
        
        for (uint16_t y = 0; y < spilcddev.height; y+=40)
        {
            esp_lcd_panel_draw_bitmap(panel_handle, 0, y, spilcddev.width, y + 40, buffer);
        }
    }

    refresh_done_flag = 0;

    do
    {
        /* 等待内部缓存刷新完成 */
        vTaskDelay(1);
    }
    while (refresh_done_flag != 1);

    heap_caps_free(buffer);
}
