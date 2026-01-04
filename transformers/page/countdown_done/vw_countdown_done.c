#include "lvgl.h"
#include "cl_ui.h"
#include "vw_countdown_done.h"

static countdown_done_view_t vw;

LV_FONT_DECLARE(font_noto_regular_20_4)
LV_FONT_DECLARE(font_noto_20_4)
LV_FONT_DECLARE(font_noto_28_4)

#define TAG "vw_countdown_done"

static void _on_btn_cb(lv_event_t *e)
{
    if (!vw.is_act) return;

    cl_button_t *btn = (cl_button_t*)lv_event_get_param(e);
    if (!btn) return;
    if (btn->id == CL_UI_KEY_POWER && btn->event == CL_BTN_CLICK)
    {
        page_change("home");
    }
}

static void _on_repaet_btn(lv_event_t *e){
    page_change_with_arg("countdown", &vw.time, sizeof(uint32_t));
}

static void _on_close_btn(lv_event_t *e){
    page_change("home");
}


static void _root_ges_cb(lv_event_t *e){
    lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_active());
    if (dir == LV_DIR_RIGHT)
    {
        lv_indev_wait_release(lv_indev_active());
        page_change("home");
    }
}


countdown_done_view_t* countdown_done_view_create(lv_obj_t *root, uint32_t time)
{
    lv_obj_remove_style_all(root);
    lv_obj_set_size(root, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_bg_color(root, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(root, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(root, 0, 0);
    lv_obj_clear_flag(root, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *label_title = lv_label_create(root);
    lv_obj_set_style_text_font(label_title, &font_noto_20_4, 0);
    uint32_t hour = time/1000/3600;
    uint32_t min = (time/1000/60)%60;
    uint32_t s = (time/1000)%60;

    vw.time = time;

    if (hour>0)
    {
        lv_label_set_text_fmt(label_title, "共%ld时%ld分%ld秒", hour, min, s);
    }else if (min > 0)
    {
        lv_label_set_text_fmt(label_title, "共%ld分%ld秒", min, s);
    }else{
        lv_label_set_text_fmt(label_title, "共%ld秒", s);
    }
    
    lv_obj_set_style_text_color(label_title, lv_color_hex(0xcccccc), 0);
    lv_obj_align(label_title, LV_ALIGN_TOP_MID, 0, 50);

    lv_obj_t *label_title1 = lv_label_create(root);
    lv_obj_set_style_text_font(label_title1, &font_noto_28_4, 0);
    lv_obj_set_style_text_color(label_title1, lv_color_hex(0xFFFFFF), 0);
    lv_label_set_text(label_title1, "计时结束");
    lv_obj_align_to(label_title1, label_title, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);

    lv_obj_t *btn_repeat = lv_btn_create(root);
    lv_obj_set_size(btn_repeat, 240, 60);
    lv_obj_align(btn_repeat, LV_ALIGN_CENTER, 0, 20);
    lv_obj_set_style_bg_color(btn_repeat, lv_color_hex(0x0080FF), 0);
    lv_obj_set_style_bg_opa(btn_repeat, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(btn_repeat, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(btn_repeat, 0, 0);
    lv_obj_set_style_border_color(btn_repeat, lv_color_hex(0x0080FF), 0);
    lv_obj_set_style_border_opa(btn_repeat, LV_OPA_COVER, 0);
    lv_obj_set_style_shadow_width(btn_repeat, 0, 0);
    lv_obj_t *btn_label_repeat = lv_label_create(btn_repeat);
    lv_obj_set_style_text_font(btn_label_repeat, &font_noto_regular_20_4, 0);
    lv_obj_set_style_text_color(btn_label_repeat, lv_color_hex(0xFFFFFF), 0);
    lv_label_set_text(btn_label_repeat, "重复");
    lv_obj_center(btn_label_repeat);
    lv_obj_add_event_cb(btn_repeat, _on_repaet_btn, LV_EVENT_CLICKED, (void *)time);

    lv_obj_t *btn_close = lv_btn_create(root);
    lv_obj_set_size(btn_close, 240, 60);
    lv_obj_align_to(btn_close, btn_repeat, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
    lv_obj_set_style_bg_color(btn_close, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_bg_opa(btn_close, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(btn_close, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(btn_close, 0, 0);
    lv_obj_set_style_border_color(btn_close, lv_color_hex(0x0080FF), 0);
    lv_obj_set_style_border_opa(btn_close, LV_OPA_COVER, 0);
    lv_obj_set_style_shadow_width(btn_close, 0, 0);
    lv_obj_t *btn_label_close = lv_label_create(btn_close);
    lv_obj_set_style_text_font(btn_label_close, &font_noto_regular_20_4, 0);
    lv_obj_set_style_text_color(btn_label_close, lv_color_hex(0x000000), 0);
    lv_label_set_text(btn_label_close, "关闭");
    lv_obj_center(btn_label_close);
    lv_obj_add_event_cb(btn_close, _on_close_btn, LV_EVENT_CLICKED, NULL);

    lv_obj_add_event_cb(root, _on_btn_cb, CL_UI_EVENT_BUTTON, NULL);
    lv_obj_add_event_cb(root, _root_ges_cb, LV_EVENT_GESTURE, NULL);
    lv_obj_clear_flag(root, LV_OBJ_FLAG_GESTURE_BUBBLE);
    return &vw;
}

void countdown_done_view_delete(void)
{
    /* 如需释放资源在此处理 */
}
