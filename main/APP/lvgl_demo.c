#include "lvgl_demo.h"
#include "lcd.h"
#include "touch.h"
#include "esp_timer.h"
#include "lvgl.h"
#include "demos/lv_demos.h"
#include <time.h>
#include <stdlib.h>
#include "esp_heap_caps.h"
#include "esp_lcd_panel_ops.h"


// LCD驱动全局变量
extern esp_lcd_panel_handle_t panel_handle;

static void increase_lvgl_tick(void *arg);
static void lvgl_disp_flush_cb(lv_display_t * disp, const lv_area_t * area, uint8_t * px_map);
static void touchpad_read(lv_indev_t * indev, lv_indev_data_t * data);

static void slider_event_cb(lv_event_t * e)
{
    lv_obj_t * slider = lv_event_get_target(e);
    lv_obj_t * label = lv_event_get_user_data(e);
    int temp = lv_slider_get_value(slider);
    lv_label_set_text_fmt(label, "Temperature: %d °C", temp);
}

// 测试UI
// 测试控件界面
void test_widget(void)
{
    // 创建滑块
    lv_obj_t * slider = lv_slider_create(lv_scr_act());
    lv_obj_set_width(slider, 200);   // 设置滑块宽度
    lv_obj_center(slider);           // 滑块居中
    lv_slider_set_range(slider, 0, 100); // 设置滑块范围0-100
    lv_slider_set_value(slider, 35, LV_ANIM_OFF); // 设置初始值35，无动画

    // 创建温度显示标签
    lv_obj_t * label = lv_label_create(lv_scr_act());
    lv_obj_align_to(label, slider, LV_ALIGN_OUT_BOTTOM_MID, 0, 10); // 标签放滑块下方
    lv_label_set_text_fmt(label, "Temperature: %d °C", 35); // 初始温度文本

    // 滑块值变化事件，更新标签
    lv_obj_add_event_cb(slider, slider_event_cb, LV_EVENT_VALUE_CHANGED, label);

    // 创建顶部文字标签1
    lv_obj_t * ltr_label = lv_label_create(lv_scr_act());
    lv_label_set_text(ltr_label, "Hello World");
    lv_obj_align(ltr_label, LV_ALIGN_TOP_MID, 0, 60); // 顶部居中显示

    // 创建顶部文字标签2
    lv_obj_t * rtl_label = lv_label_create(lv_scr_act());
    lv_label_set_text(rtl_label, "LVGL TEST");
    lv_obj_align(rtl_label, LV_ALIGN_TOP_MID, 0, 90); // 顶部居中显示
}
/**
 * @brief       lvgl_demo入口函数
 */
void lvgl_demo(void)
{
    lv_init();
    lv_port_disp_init();
    lv_port_indev_init();

    const esp_timer_create_args_t lvgl_tick_timer_args = {
        .callback = &increase_lvgl_tick,
        .name = "lvgl_tick"
    };
    esp_timer_handle_t lvgl_tick_timer = NULL;
    ESP_ERROR_CHECK(esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer));
    ESP_ERROR_CHECK(esp_timer_start_periodic(lvgl_tick_timer, 1 * 1000));

    // 运行你自己的UI
    // setup_ui(&guider_ui);
    // test_widget();  // 备用测试UI
	// lv_demo_music();
	// lv_demo_widgets();
	// lv_demo_keypad_encoder() ;
	lv_demo_stress();
    while (1)
    {
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}

/**
 * @brief       初始化显示设备（最终稳定版）
 */
void lv_port_disp_init(void)
{
    void *buf1 = NULL;
    void *buf2 = NULL;

    lcd_cfg_t lcd_config_info = {0};
    lcd_config_info.notify_flush_ready = NULL;
    lcd_init(lcd_config_info);

    size_t buf_size = lcd_dev.width * 60 * sizeof(lv_color_t);
    buf1 = heap_caps_malloc(buf_size, MALLOC_CAP_DMA);
    buf2 = heap_caps_malloc(buf_size, MALLOC_CAP_DMA);

    // 防御性判断（增强稳定性）
    if(buf1 == NULL || buf2 == NULL) {
        printf("LVGL Buffer Alloc Failed!\r\n");
        return;
    }

    static lv_draw_buf_t disp_buf1;
    static lv_draw_buf_t disp_buf2;
    uint32_t stride = lv_draw_buf_width_to_stride(lcd_dev.width, LV_COLOR_FORMAT_RGB565);

    lv_draw_buf_init(&disp_buf1, lcd_dev.width, 60, LV_COLOR_FORMAT_RGB565, stride, buf1, buf_size);
    lv_draw_buf_init(&disp_buf2, lcd_dev.width, 60, LV_COLOR_FORMAT_RGB565, stride, buf2, buf_size);

    lv_display_t * disp = lv_display_create(lcd_dev.width, lcd_dev.height);
    lv_display_set_flush_cb(disp, lvgl_disp_flush_cb);
    lv_display_set_draw_buffers(disp, &disp_buf1, &disp_buf2);

    LCD_BL(1);
}

/**
 * @brief       初始化触摸输入
 */
void lv_port_indev_init(void)
{
    tp_dev.init();
    lv_indev_t *indev_touchpad = lv_indev_create();
    lv_indev_set_type(indev_touchpad, LV_INDEV_TYPE_POINTER);
    lv_indev_set_read_cb(indev_touchpad, touchpad_read);
}

/**
 * @brief       显示刷新回调
 */
static void lvgl_disp_flush_cb(lv_display_t * disp, const lv_area_t * area, uint8_t * px_map)
{
    esp_lcd_panel_draw_bitmap(panel_handle, area->x1, area->y1, area->x2 + 1, area->y2 + 1, (lv_color_t *)px_map);
    lv_display_flush_ready(disp);
}

/**
 * @brief       LVGL 系统时钟
 */
static void increase_lvgl_tick(void *arg)
{
    lv_tick_inc(1);
}

/**
 * @brief       触摸读取
 */
void touchpad_read(lv_indev_t * indev, lv_indev_data_t * data)
{
    static lv_coord_t last_x = 0;
    static lv_coord_t last_y = 0;

    if(tp_dev.scan(0) && (tp_dev.sta & TP_PRES_DOWN))
    {
        last_x = tp_dev.x[0];
        last_y = tp_dev.y[0];
        data->state = LV_INDEV_STATE_PR;
    }
    else
    {
        data->state = LV_INDEV_STATE_REL;
    }

    data->point.x = last_x;
    data->point.y = last_y;
}