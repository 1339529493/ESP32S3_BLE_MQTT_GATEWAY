#include "smf.h"
#include "ui.h"
#include <time.h>
#include <stdio.h>
/* ==========================================================================
 * 桌面用户数据结构
 * ========================================================================== */
struct desktop_data {
    struct smf_ctx ctx;
    
    /* UI 控件 */
    lv_obj_t *wallpaper;      // 壁纸图片（或纯色背景）
    lv_obj_t *time_label;     // 时间标签
    lv_obj_t *date_label;     // 日期标签
    lv_obj_t *battery_arc;    // 电池电量弧形指示器
    lv_obj_t *battery_label;  // 电池百分比标签
    lv_obj_t *step_label;     // 计步器显示
    
    /* 数据 */
    uint32_t last_tick;       // 上次更新时间戳
    int battery_level;        // 电量 0-100
    int step_count;           // 步数
    
    /* LVGL 定时器 */
    lv_timer_t *update_timer;
};

/* 前向声明 */
static void desktop_update_ui(struct desktop_data *data);

/* ==========================================================================
 * 创建渐变背景（纯色背景 + 装饰）
 * ========================================================================== */
static lv_obj_t *create_wallpaper(lv_obj_t *parent)
{
    lv_obj_t *bg = lv_obj_create(parent);
    lv_obj_set_size(bg, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_bg_color(bg, lv_color_hex(0x1a1a2e), 0);  // 深紫色背景
    lv_obj_set_style_bg_grad_color(bg, lv_color_hex(0x16213e), 0);
    lv_obj_set_style_bg_grad_dir(bg, LV_GRAD_DIR_VER, 0);
    lv_obj_set_style_radius(bg, 0, 0);
    lv_obj_set_style_border_width(bg, 0, 0);
    
    /* 添加装饰圆点 */
    lv_obj_t *decor = lv_obj_create(bg);
    lv_obj_set_size(decor, 150, 150);
    lv_obj_set_pos(decor, -50, -50);
    lv_obj_set_style_radius(decor, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(decor, lv_color_hex(0x0f3460), 0);
    lv_obj_set_style_bg_opa(decor, LV_OPA_50, 0);
    lv_obj_set_style_border_width(decor, 0, 0);
    
    /* 右下角装饰 */
    lv_obj_t *decor2 = lv_obj_create(bg);
    lv_obj_set_size(decor2, 200, 200);
    lv_obj_set_pos(decor2, LV_HOR_RES - 100, LV_VER_RES - 100);
    lv_obj_set_style_radius(decor2, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(decor2, lv_color_hex(0xe94560), 0);
    lv_obj_set_style_bg_opa(decor2, LV_OPA_20, 0);
    lv_obj_set_style_border_width(decor2, 0, 0);
    
    return bg;
}

/* ==========================================================================
 * 创建时间显示（大字体数字时钟）
 * ========================================================================== */
static lv_obj_t *create_time_display(lv_obj_t *parent)
{
    lv_obj_t *container = lv_obj_create(parent);
    lv_obj_set_size(container, LV_HOR_RES, 200);
    lv_obj_set_style_bg_opa(container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(container, 0, 0);
    lv_obj_align(container, LV_ALIGN_CENTER, 0, -30);
    
    lv_obj_t *time_label = lv_label_create(container);
    lv_obj_set_style_text_color(time_label, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_text_font(time_label, &lv_font_montserrat_48, 0);  // 大字体
    lv_label_set_text(time_label, "--:--");
    lv_obj_center(time_label);
    
    return time_label;
}

/* ==========================================================================
 * 创建日期显示
 * ========================================================================== */
static lv_obj_t *create_date_display(lv_obj_t *parent)
{
    lv_obj_t *date_label = lv_label_create(parent);
    lv_obj_set_style_text_color(date_label, lv_color_hex(0xcccccc), 0);
    lv_obj_set_style_text_font(date_label, &lv_font_montserrat_16, 0);
    lv_label_set_text(date_label, "----年--月--日");
    lv_obj_align(date_label, LV_ALIGN_CENTER, 0, 50);
    
    return date_label;
}

/* ==========================================================================
 * 创建电池电量指示器（弧形进度条）
 * ========================================================================== */
static lv_obj_t *create_battery_indicator(lv_obj_t *parent)
{
    /* 创建弧形 */
    lv_obj_t *arc = lv_arc_create(parent);
    lv_arc_set_range(arc, 0, 100);
    lv_arc_set_value(arc, 75);
    lv_arc_set_bg_angles(arc, 0, 360);
    lv_arc_set_rotation(arc, 0);
    lv_obj_set_size(arc, 60, 60);
    lv_obj_align(arc, LV_ALIGN_TOP_RIGHT, -15, 15);
    
    /* 弧形颜色：根据电量变化 */
    lv_obj_set_style_arc_color(arc, lv_color_hex(0x4caf50), 0);
    lv_obj_remove_style(arc, NULL, LV_PART_KNOB);  // 隐藏旋钮
    lv_obj_set_style_arc_width(arc, 6, 0);
    lv_obj_set_style_bg_opa(arc, LV_OPA_TRANSP, 0);
    
    /* 百分比标签 */
    lv_obj_t *label = lv_label_create(parent);
    lv_label_set_text(label, "75%");
    lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_12, 0);
    lv_obj_align(label, LV_ALIGN_TOP_RIGHT, -15, 35);
    
    /* 将弧和标签存到 data 中需要额外存储，
       这里简化返回 arc，实际使用时通过 lv_obj_get_user_data 获取 */
    return arc;
}

/* ==========================================================================
 * 创建计步器显示（卡片样式）
 * ========================================================================== */
static lv_obj_t *create_step_card(lv_obj_t *parent)
{
    /* 卡片背景 */
    lv_obj_t *card = lv_obj_create(parent);
    lv_obj_set_size(card, 160, 80);
    lv_obj_align(card, LV_ALIGN_BOTTOM_LEFT, 15, -20);
    lv_obj_set_style_bg_color(card, lv_color_hex(0x16213e), 0);
    lv_obj_set_style_bg_opa(card, LV_OPA_70, 0);
    lv_obj_set_style_radius(card, 12, 0);
    lv_obj_set_style_border_width(card, 0, 0);
    
    /* 步行图标（用 emoji 或文本代替图标） */
    lv_obj_t *icon = lv_label_create(card);
    lv_label_set_text(icon, "🚶");
    lv_obj_set_style_text_font(icon, &lv_font_montserrat_24, 0);
    lv_obj_align(icon, LV_ALIGN_LEFT_MID, 15, 0);
    
    /* 步数数字 */
    lv_obj_t *step_label = lv_label_create(card);
    lv_label_set_text(step_label, "0");
    lv_obj_set_style_text_color(step_label, lv_color_hex(0xffd700), 0);
    lv_obj_set_style_text_font(step_label, &lv_font_montserrat_28, 0);
    lv_obj_align(step_label, LV_ALIGN_CENTER, 20, -10);
    
    /* "步" 单位 */
    lv_obj_t *unit = lv_label_create(card);
    lv_label_set_text(unit, "步");
    lv_obj_set_style_text_font(unit, &lv_font_montserrat_12, 0);
    lv_obj_align(unit, LV_ALIGN_CENTER, 20, 15);
    
    return step_label;
}

/* ==========================================================================
 * 更新时间/日期显示
 * ========================================================================== */
static void desktop_update_time(struct desktop_data *data)
{
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    
    char time_str[16];
    char date_str[32];
    
    strftime(time_str, sizeof(time_str), "%H:%M", tm_info);
    strftime(date_str, sizeof(date_str), "%Y年%m月%d日 %A", tm_info);
    
    if (data->time_label) {
        lv_label_set_text(data->time_label, time_str);
    }
    if (data->date_label) {
        lv_label_set_text(data->date_label, date_str);
    }
}

/* ==========================================================================
 * 更新电池显示
 * ========================================================================== */
static void desktop_update_battery(struct desktop_data *data)
{
    if (data->battery_arc) {
        lv_arc_set_value(data->battery_arc, data->battery_level);
        
        /* 根据电量改变颜色 */
        lv_color_t color;
        if (data->battery_level > 60) {
            color = lv_color_hex(0x4caf50);  // 绿色
        } else if (data->battery_level > 20) {
            color = lv_color_hex(0xff9800);  // 橙色
        } else {
            color = lv_color_hex(0xf44336);  // 红色
        }
        lv_obj_set_style_arc_color(data->battery_arc, color, 0);
    }
    
    if (data->battery_label) {
        char buf[8];
        snprintf(buf, sizeof(buf), "%d%%", data->battery_level);
        lv_label_set_text(data->battery_label, buf);
    }
}

/* ==========================================================================
 * 更新步数显示
 * ========================================================================== */
static void desktop_update_step(struct desktop_data *data)
{
    if (data->step_label) {
        char buf[16];
        snprintf(buf, sizeof(buf), "%d", data->step_count);
        lv_label_set_text(data->step_label, buf);
    }
}

/* ==========================================================================
 * 统一更新 UI
 * ========================================================================== */
static void desktop_update_ui(struct desktop_data *data)
{
    desktop_update_time(data);
    desktop_update_battery(data);
    desktop_update_step(data);
}

/* ==========================================================================
 * LVGL 定时器回调（每秒更新一次）
 * ========================================================================== */
static void desktop_timer_cb(lv_timer_t *timer)
{
    struct desktop_data *data = (struct desktop_data *)lv_timer_get_user_data(timer);
    if (data) {
        desktop_update_time(data);
    }
}

/* ==========================================================================
 * SMF 状态函数
 * ========================================================================== */
void desktop_entry(void *obj)
{
    struct desktop_data *data = (struct desktop_data *)obj;
    
    // 1. 创建壁纸
    data->wallpaper = create_wallpaper(lv_screen_active());
    
    // 2. 创建时间（大字体）
    data->time_label = create_time_display(lv_screen_active());
    
    // 3. 创建日期
    data->date_label = create_date_display(lv_screen_active());
    
    // 4. 创建电池指示器
    data->battery_arc = create_battery_indicator(lv_screen_active());
    // 获取电池标签（需要额外存储，简化起见在 create 中一并创建）
    // 实际使用中可以用 lv_obj_get_user_data 或全局变量
    data->battery_level = 85;  // 示例电量
    
    // 5. 创建步数卡片
    data->step_label = create_step_card(lv_screen_active());
    data->step_count = 8234;   // 示例步数
    
    // 6. 初始化数据
    data->last_tick = lv_tick_get();
    desktop_update_ui(data);
    
    // 7. 创建定时器（每秒更新时间）
    data->update_timer = lv_timer_create(desktop_timer_cb, 1000, data);
    
    printf("桌面已加载\n");
}

enum smf_state_result desktop_run(void *obj)
{
    struct desktop_data *data = (struct desktop_data *)obj;
    
    // 这里可以检查新事件，例如接收到电量更新
    // 如果有外部数据更新，调用 desktop_update_battery/step 等
    
    return SMF_EVENT_PROPAGATE;
}

void desktop_exit(void *obj)
{
    struct desktop_data *data = (struct desktop_data *)obj;
    
    // 删除定时器
    if (data->update_timer) {
        lv_timer_del(data->update_timer);
        data->update_timer = NULL;
    }
    
    // 删除所有 UI 控件（LVGL 会自动处理父子关系，只需删除根容器）
    if (data->wallpaper) {
        lv_obj_del(data->wallpaper);
        data->wallpaper = NULL;
    }
    
    printf("桌面已退出\n");
}