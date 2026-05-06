#include <stdio.h>

#include "gw_log.h"
#include "lvgl_lcd.h"
#include "demos/lv_demos.h"

/* central_time_zone.c - 适用于 320x240 竖屏的中央大时间区 */

#include "lvgl.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdio.h>
#include <time.h>

/*******************************
 * 时间变量
*******************************/
static struct tm s_time_info;
static bool s_time_valid = false;

/*******************************
 * UI 控件句柄
*******************************/
static lv_obj_t *g_hour_label;      // 小时数字
static lv_obj_t *g_minute_label;    // 分钟数字
static lv_obj_t *g_second_label;    // 秒数（小字）
static lv_obj_t *g_week_label;      // 星期
static lv_obj_t *g_date_label;      // 月/日
static lv_obj_t *g_ampm_label;      // 上午/下午（12小时制用）

/*******************************
 * 星期字符串（英文）
*******************************/
static const char *week_days[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
static const char *week_days_cn[] = {"周日", "周一", "周二", "周三", "周四", "周五", "周六"};

/*******************************
 * 外部时间设置接口
*******************************/
void watch_set_time(int year, int month, int day, int hour, int minute, int second)
{
    s_time_info.tm_year = year - 1900;
    s_time_info.tm_mon = month - 1;
    s_time_info.tm_mday = day;
    s_time_info.tm_hour = hour;
    s_time_info.tm_min = minute;
    s_time_info.tm_sec = second;
    s_time_info.tm_wday = 0; // 让 mktime 自动计算星期
    mktime(&s_time_info);     // 归一化并自动计算 tm_wday
    s_time_valid = true;
}

/*******************************
 * 获取当前星期（字符串）
*******************************/
static const char* get_week_day_str(void)
{
    if (!s_time_valid) return "Monday";
    return week_days[s_time_info.tm_wday];
}

/*******************************
 * 更新 UI 显示
*******************************/
static void update_time_display(void)
{
    char buf[8];
    
    if (!s_time_valid) {
        lv_label_set_text(g_hour_label, "--");
        lv_label_set_text(g_minute_label, "--");
        lv_label_set_text(g_second_label, "--");
        lv_label_set_text(g_week_label, "Monday");
        lv_label_set_text(g_date_label, "--/--");
        return;
    }
    
    // 1. 小时（12小时制，不想显示AM/PM就用24小时制）
    int hour_12 = s_time_info.tm_hour % 12;
    if (hour_12 == 0) hour_12 = 12;
    lv_snprintf(buf, sizeof(buf), "%02d", hour_12);
    lv_label_set_text(g_hour_label, buf);
    
    // 2. 分钟
    lv_snprintf(buf, sizeof(buf), "%02d", s_time_info.tm_min);
    lv_label_set_text(g_minute_label, buf);
    
    // 3. 秒数（小字）
    lv_snprintf(buf, sizeof(buf), "%02d", s_time_info.tm_sec);
    lv_label_set_text(g_second_label, buf);
    
    // 4. AM/PM 标识
    const char *ampm = s_time_info.tm_hour >= 12 ? "PM" : "AM";
    lv_label_set_text(g_ampm_label, ampm);
    
    // 5. 星期（中文更亲切）
    lv_label_set_text(g_week_label, week_days_cn[s_time_info.tm_wday]);
    
    // 6. 月/日
    lv_snprintf(buf, sizeof(buf), "%02d/%02d", s_time_info.tm_mon + 1, s_time_info.tm_mday);
    lv_label_set_text(g_date_label, buf);
}

/*******************************
 * 自动走秒定时器
*******************************/
static void clock_timer_cb(lv_timer_t *timer)
{
    if (s_time_valid) {
        s_time_info.tm_sec++;
        mktime(&s_time_info);  // 自动处理进位
        update_time_display();
    }
}

/*******************************
 * UI 主线程（白色背景 + 中央大时间区）
*******************************/
void ui_task(void *pvParameter)
{
    lv_lcd_init();
    
    lv_obj_t *scr = lv_screen_active();
    
    /* ========== 1. 白色背景（方便未来替换壁纸） ========== */
    lv_obj_set_style_bg_color(scr, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);
    
    /* 可选：添加一个中央容器，方便整体管理（非必须） */
    lv_obj_t *time_container = lv_obj_create(scr);
    lv_obj_set_size(time_container, 280, 160);
    lv_obj_center(time_container);
    lv_obj_set_style_bg_opa(time_container, LV_OPA_TRANSP, 0);  // 透明
    lv_obj_set_style_border_width(time_container, 0, 0);
    lv_obj_set_style_pad_all(time_container, 0, 0);
    
    /* ========== 2. 小时数字（左侧） ========== */
    g_hour_label = lv_label_create(time_container);
    lv_label_set_text(g_hour_label, "12");
    lv_obj_set_style_text_font(g_hour_label, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(g_hour_label, lv_color_hex(0x000000), 0);
    lv_obj_set_pos(g_hour_label, 40, 30);
    
    /* ========== 3. 冒号（静态装饰） ========== */
    lv_obj_t *colon_label = lv_label_create(time_container);
    lv_label_set_text(colon_label, ":");
    lv_obj_set_style_text_font(colon_label, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(colon_label, lv_color_hex(0x000000), 0);
    lv_obj_set_pos(colon_label, 130, 30);
    
    /* ========== 4. 分钟数字（右侧） ========== */
    g_minute_label = lv_label_create(time_container);
    lv_label_set_text(g_minute_label, "00");
    lv_obj_set_style_text_font(g_minute_label, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(g_minute_label, lv_color_hex(0x000000), 0);
    lv_obj_set_pos(g_minute_label, 160, 30);
    
    /* ========== 5. 秒数（右上小字） ========== */
    g_second_label = lv_label_create(time_container);
    lv_label_set_text(g_second_label, "00");
    lv_obj_set_style_text_font(g_second_label, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(g_second_label, lv_color_hex(0x888888), 0);
    lv_obj_set_pos(g_second_label, 220, 20);
    
    /* ========== 6. AM/PM 标识 ========== */
    g_ampm_label = lv_label_create(time_container);
    lv_label_set_text(g_ampm_label, "AM");
    lv_obj_set_style_text_font(g_ampm_label, &lv_font_montserrat_14, 0);
    lv_obj_set_style_text_color(g_ampm_label, lv_color_hex(0x888888), 0);
    lv_obj_set_pos(g_ampm_label, 220, 40);
    
    /* ========== 7. 星期（下方左侧） ========== */
    g_week_label = lv_label_create(time_container);
    lv_label_set_text(g_week_label, "星期一");
    lv_obj_set_style_text_font(g_week_label, &lv_font_montserrat_18, 0);
    lv_obj_set_style_text_color(g_week_label, lv_color_hex(0x555555), 0);
    lv_obj_set_pos(g_week_label, 50, 110);
    
    /* ========== 8. 日期（下方右侧） ========== */
    g_date_label = lv_label_create(time_container);
    lv_label_set_text(g_date_label, "01/01");
    lv_obj_set_style_text_font(g_date_label, &lv_font_montserrat_18, 0);
    lv_obj_set_style_text_color(g_date_label, lv_color_hex(0x555555), 0);
    lv_obj_set_pos(g_date_label, 170, 110);
    
    /* ========== 9. 可选：装饰线（分割状态栏和下面） ========== */
    lv_obj_t *line = lv_line_create(scr);
    static lv_point_precise_t line_pts[] = {{0, 200}, {320, 200}};
    lv_line_set_points(line, line_pts, 2);
    lv_obj_set_style_line_width(line, 1, 0);
    lv_obj_set_style_line_color(line, lv_color_hex(0xCCCCCC), 0);
    
    /* ========== 10. 启动计时器（每秒更新） ========== */
    lv_timer_create(clock_timer_cb, 1000, NULL);
    
    /* 设置一个示例时间（2025年1月15日 14:30:45 星期三） */
    watch_set_time(2025, 1, 15, 14, 30, 45);
    update_time_display();
    
    /* 主循环 */
    while (1) {
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}