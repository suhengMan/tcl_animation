#include "lvgl.h"
#include "cl_ui.h"
#include "vw_alarm_ring.h"
#include <time.h>
#ifndef SIMULATOR
#include "vb_adapter.h"
#endif

static alarm_ring_view_t vw;

#define TAG "vw_alarm_ring"

LV_FONT_DECLARE(font_puhui_ascii_80_4)
LV_IMG_DECLARE(icon_alarm_64)

static void _on_btn_cb(lv_event_t *e)
{
    if (!vw.is_act) return;

    cl_button_t *btn = (cl_button_t*)lv_event_get_param(e);
    if (!btn) return;

    switch (btn->id)
    {
        case CL_UI_KEY_POWER:
            if (btn->event == CL_BTN_CLICK)
            {
#ifndef SIMULATOR
                vb_alarm_stop();
                page_change("home");
#endif
            }
            break;
        default:
            break;
    }
}

static void _stop_click(lv_event_t *e){
#ifndef SIMULATOR
    vb_alarm_stop();
    page_change("home");
#endif
}


// 新建一个timer刷新时间，每秒一次
static void _update_time_cb(lv_timer_t* timer)
{
    // 获取系统时间，可根据平台替换成具体实现
    static char buf[6];
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);
    int hour = tm_info->tm_hour;
    int min = tm_info->tm_min;
    snprintf(buf, sizeof(buf), "%02d:%02d", hour, min);
    lv_label_set_text(vw.label_time, buf);
}


alarm_ring_view_t* alarm_ring_view_create(lv_obj_t *root)
{
    lv_obj_remove_style_all(root);
    lv_obj_set_size(root, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_bg_color(root, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(root, LV_OPA_COVER, 0);
    lv_obj_add_event_cb(root, _on_btn_cb, CL_UI_EVENT_BUTTON, NULL);


    lv_obj_t *time_label = lv_label_create(root);
    vw.label_time = time_label;
    lv_obj_set_style_text_color(time_label,lv_color_hex(0xffffff),0);
    lv_obj_set_style_text_font(time_label, &font_puhui_ascii_80_4, 0);
    lv_label_set_text(time_label, "");
    lv_obj_center(time_label);
    vw.timer = lv_timer_create(_update_time_cb, 1000, NULL);
    _update_time_cb(NULL); // 启动时主动刷新一次

    lv_obj_t *alarm_label = lv_img_create(root);
    lv_img_set_src(alarm_label, &icon_alarm_64);
    lv_obj_align_to(alarm_label, time_label, LV_ALIGN_OUT_TOP_MID, 0, -20);

    // INSERT_YOUR_CODE
    lv_obj_t *btb = lv_btn_create(root);
    lv_obj_set_style_radius(btb, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_size(btb, 180, 60);
    lv_obj_set_style_shadow_width(btb, 0, 0);
    lv_obj_align(btb, LV_ALIGN_BOTTOM_MID, 0, -50);
    lv_obj_t *exit_label = lv_label_create(btb);
    lv_obj_set_style_text_font(exit_label, cl_ui_get_font(), 0);
    lv_obj_set_style_text_color(exit_label, lv_color_white(), 0);
    lv_label_set_text(exit_label, "停止");
    lv_obj_center(exit_label);

    lv_obj_add_event_cb(btb, _stop_click, LV_EVENT_SHORT_CLICKED, NULL);
    return &vw;
}

void alarm_ring_view_delete(void)
{
    if (vw.timer)
    {
        lv_timer_delete(vw.timer);
        vw.timer = NULL;
    }
    
}
