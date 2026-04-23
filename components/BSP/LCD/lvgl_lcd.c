#include "lvgl/lvgl.h" /* 定义 LV_LVGL_H_INCLUDE_SIMPLE 以使用 "lvgl.h" 方式包含 */
#include "lcd.h"
#include "demos/lv_demos.h"

#define TFT_HOR_RES 320
#define TFT_VER_RES 240

static uint32_t my_tick_cb(void)
{
    return lv_tick_inc(1);
}

static void my_flush_cb(lv_display_t * disp, const lv_area_t * area, uint8_t * px_map)
{
    /* 将 px_map 写入帧缓冲区或外部显示控制器的指定区域 
     * (area->x1, area->x2, area->y1, area->y2) */
    esp_lcd_panel_handle_t panel_handle = (esp_lcd_panel_handle_t)drv->user_data;

    /* 特定区域打点 */
    esp_lcd_panel_draw_bitmap(panel_handle, area->x1, area->y1, area->x2 + 1, area->y2 + 1, color_map);

    /* 重要!!! 通知图形库，已经刷新完毕了 */
    lv_disp_flush_ready(drv);
}

static void my_touch_read_cb(lv_indev_t * indev, lv_indev_data_t * data)
{
   if(my_touch_is_pressed()) {
       data->point.x = touchpad_x;
       data->point.y = touchpad_y;
       data->state = LV_INDEV_STATE_PRESSED;
   } else {
       data->state = LV_INDEV_STATE_RELEASED;
   }
}

void lv_test(void)
{
    my_hardware_init();

    /* 初始化 LVGL */
    lv_init();

    /* 为 LVGL 设置基于毫秒的时钟源，以便其跟踪时间 */
    lv_tick_set_cb(my_tick_cb);

    /* 创建一个显示对象，用于添加屏幕和控件 */
    lv_display_t * display = lv_display_create(TFT_HOR_RES, TFT_VER_RES);

    /* 为屏幕添加渲染缓冲区。
     * 这里添加了一个较小的部分缓冲区，假设为 16 位色深 (RGB565 格式) */
    static uint8_t buf[TFT_HOR_RES * TFT_VER_RES / 10 * 2]; /* x2 是因为 16 位色深 */
    lv_display_set_buffers(display, buf, NULL, sizeof(buf), LV_DISPLAY_RENDER_MODE_PARTIAL);

    /* 添加一个回调函数，当缓冲区内容渲染完成后执行刷新操作 */
    lv_display_set_flush_cb(display, my_flush_cb);

    /* 创建一个用于处理触摸输入的输入设备 */
    lv_indev_t * indev = lv_indev_create();
    lv_indev_set_type(indev, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev, my_touch_read_cb);

    /* 驱动已就绪；现在可以创建 UI 界面了 */
    lv_obj_t * label = lv_label_create(lv_screen_active());
    lv_label_set_text(label, "Hello world");
    lv_obj_center(label);

    /* 在循环中执行 LVGL 相关任务 */
    while(1) {
        lv_timer_handler();
        my_sleep_ms(5);         /* 短暂等待，让系统喘息 */
    }
}