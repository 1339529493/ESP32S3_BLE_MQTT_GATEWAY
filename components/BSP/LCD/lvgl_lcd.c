#include "esp_timer.h"
#include "lvgl_lcd.h"
#include "iic.h"
#include "xl9555.h"
#include "spi.h"
#include "lcd.h"
void lcd_init()
{
    iic_init();
    xl9555_init();
    spi2_init();
    spilcd_init();
}
static uint32_t my_tick_cb(void)
{
    return (uint32_t)(esp_timer_get_time() / 1000);
}

static void my_flush_cb(lv_display_t * disp, const lv_area_t * area, uint8_t * px_map)
{
    /* 将 px_map 写入帧缓冲区或外部显示控制器的指定区域 
     * (area->x1, area->x2, area->y1, area->y2) */
    esp_lcd_panel_handle_t handle = lv_display_get_user_data(disp);
    /* 特定区域打点 */
    esp_lcd_panel_draw_bitmap(handle, area->x1, area->y1, area->x2 + 1, area->y2 + 1, px_map);
    /* 通知图形库，已经刷新完毕了 */
    lv_disp_flush_ready(disp);
}

void lv_lcd_init(void)
{
    lcd_init();
    /* 初始化 LVGL */
    esp_timer_get_time();
    lv_init();

    /* 为 LVGL 设置基于毫秒的时钟源，以便其跟踪时间 */
    lv_tick_set_cb(my_tick_cb);

    /* 创建一个显示对象，用于添加屏幕和控件 */
    lv_display_t *display = lv_display_create(spilcddev.pwidth, spilcddev.pheight);

    /* 为屏幕添加渲染缓冲区(双缓冲区)。
     * 这里添加了一个较小的部分缓冲区，假设为 16 位色深 (RGB565 格式) */
    int lcd_size = spilcddev.pheight * spilcddev.pwidth / 10 * 2; /* x2 是因为 16 位色深 */
    uint8_t *buf1 = (uint8_t *)heap_caps_malloc(lcd_size, MALLOC_CAP_SPIRAM);
    uint8_t *buf2 = (uint8_t *)heap_caps_malloc(lcd_size, MALLOC_CAP_SPIRAM);
    lv_display_set_buffers(display, buf1, buf2, lcd_size, LV_DISPLAY_RENDER_MODE_PARTIAL);

    lv_display_set_user_data(display, panel_handle);

    /* 添加一个回调函数，当缓冲区内容渲染完成后执行刷新操作 */
    lv_display_set_flush_cb(display, my_flush_cb);


}