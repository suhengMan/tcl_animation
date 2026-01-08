#include "cl_ui.h"
#include "lvgl.h"
#ifndef SIMULATOR
#include "vb_adapter.h"
#endif

static lv_obj_t *s_vol_cont = NULL;
static lv_obj_t *s_vol_bar = NULL;
static lv_obj_t *s_btn_vol_up= NULL;
static lv_obj_t *s_btn_vol_down = NULL;
static lv_timer_t *s_hide_timer = NULL;

LV_IMG_DECLARE(icon_exit_32)

void cl_ui_vol_set_vol(uint8_t vol){
    if (s_vol_bar == NULL)
    {
        return;
    }
    lv_bar_set_value(s_vol_bar, vol, LV_ANIM_OFF);
}

static void _timer_cb(lv_timer_t *t){
    if (s_vol_cont && !lv_obj_has_flag(s_vol_cont, LV_OBJ_FLAG_HIDDEN))
    {
        lv_obj_add_flag(s_vol_cont, LV_OBJ_FLAG_HIDDEN);
    }
}

//音量按钮
static void _on_volume_btn_cb(lv_event_t *e){
    lv_obj_t *btn = lv_event_get_target(e);
#ifndef SIMULATOR
    int cur_vol = vb_audio_get_volume();
    cur_vol += (btn == s_btn_vol_up)?10:-10;
    if (cur_vol > 100) cur_vol = 100;
    else if (cur_vol < 0) cur_vol = 0;
    vb_audio_set_volume((uint8_t)cur_vol);
#else
    int cur_vol = lv_bar_get_value(s_vol_bar);
    cur_vol += (btn == s_btn_vol_up)?10:-10;
    if (cur_vol > 100) cur_vol = 100;
    else if (cur_vol < 0) cur_vol = 0;
    lv_bar_set_value(s_vol_bar, cur_vol, LV_ANIM_OFF);
#endif

}

static void _tiemr_reset(int show_time){
    if (s_hide_timer == NULL)
    {
        s_hide_timer = lv_timer_create(_timer_cb, show_time, NULL);
        lv_timer_set_auto_delete(s_hide_timer, false);
        lv_timer_set_repeat_count(s_hide_timer, 1);
    }else{
        lv_timer_set_period(s_hide_timer, show_time);
        lv_timer_reset(s_hide_timer);
        lv_timer_set_repeat_count(s_hide_timer, 1);
        lv_timer_resume(s_hide_timer);
    }
    
}

void cl_ui_show_vol_bar(int show_time){
    if (s_vol_cont == NULL)
    {
        return;
    }
    _tiemr_reset(show_time);
    lv_obj_clear_flag(s_vol_cont, LV_OBJ_FLAG_HIDDEN);
}


uint8_t cl_ui_vol_bar_is_show(){
    if (s_vol_cont == NULL)
    {
        return 0;
    }
    
    return 1==lv_obj_has_flag(s_vol_cont, LV_OBJ_FLAG_HIDDEN)?0:1;
}

void cl_ui_vol_bar_hide(){
    if (s_vol_cont != NULL)
    {
        lv_obj_add_flag(s_vol_cont, LV_OBJ_FLAG_HIDDEN);
    }
}

static void _event_cb(lv_event_t *e){
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_PRESSED)
    {
        lv_timer_pause(s_hide_timer);
    }else if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST)
    {
        lv_timer_reset(s_hide_timer);
    }else if (code == LV_EVENT_GESTURE)
    {
        lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_active());
        if (dir == LV_DIR_RIGHT)
        {
            lv_timer_pause(s_hide_timer);
            lv_obj_add_flag(s_vol_cont, LV_OBJ_FLAG_HIDDEN);
        }
    }else if (code == LV_EVENT_SHORT_CLICKED)
    {
        lv_timer_pause(s_hide_timer);
        lv_obj_add_flag(s_vol_cont, LV_OBJ_FLAG_HIDDEN);
    }
}

void cl_ui_init_vol_bar(){
    s_vol_cont = lv_obj_create(lv_layer_top());
    lv_obj_set_size(s_vol_cont, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_opa(s_vol_cont, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(s_vol_cont, lv_color_black(), 0);
    lv_obj_set_style_border_width(s_vol_cont, 0, 0);

    // 创建中间的横条进度条
    lv_obj_t *bar = lv_bar_create(s_vol_cont);
    s_vol_bar = bar;
    lv_obj_set_size(bar, 180, 10);
    lv_obj_align(bar, LV_ALIGN_CENTER, 0, 0);
    lv_bar_set_range(bar, 0, 100);
    lv_obj_set_style_bg_color(bar, lv_color_hex(0xcccccc), LV_PART_MAIN);
    lv_obj_set_style_bg_color(bar, lv_color_hex(0xff5353), LV_PART_INDICATOR);
    lv_obj_set_style_radius(bar, 10, LV_PART_MAIN);
    lv_obj_set_style_radius(bar, 10, LV_PART_INDICATOR);
    
    // 创建减号按钮（左侧）
    lv_obj_t *btn_minus = lv_btn_create(s_vol_cont);
    s_btn_vol_down = btn_minus;
    lv_obj_set_size(btn_minus, 60, 60);
    lv_obj_align(btn_minus, LV_ALIGN_CENTER, -130, 0);
    lv_obj_set_style_bg_color(btn_minus, lv_color_hex(0x333333), 0);
    lv_obj_set_style_bg_opa(btn_minus, LV_OPA_TRANSP, 0);
    lv_obj_set_style_radius(btn_minus, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_shadow_width(btn_minus, 0, 0);
    
    LV_IMG_DECLARE(icon_minus_32)
    lv_obj_t *label_minus = lv_img_create(btn_minus);
    lv_img_set_src(label_minus, &icon_minus_32);
    lv_obj_center(label_minus);
    
    // 创建加号按钮（右侧）
    lv_obj_t *btn_plus = lv_btn_create(s_vol_cont);
    s_btn_vol_up = btn_plus;
    lv_obj_set_size(btn_plus, 60, 60);
    lv_obj_align(btn_plus, LV_ALIGN_CENTER, 130, 0);
    lv_obj_set_style_bg_color(btn_plus, lv_color_hex(0x333333), 0);
    lv_obj_set_style_bg_opa(btn_plus, LV_OPA_TRANSP, 0);
    lv_obj_set_style_radius(btn_plus, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_shadow_width(btn_plus, 0, 0);
    
    LV_IMG_DECLARE(icon_plus_32)
    lv_obj_t *label_plus = lv_img_create(btn_plus);
    lv_img_set_src(label_plus, &icon_plus_32);
    lv_obj_center(label_plus);

    // 创建 exit 按钮，居中屏幕下方
    lv_obj_t *btn_exit = lv_btn_create(s_vol_cont);
    lv_obj_set_size(btn_exit, 80, 80);
    lv_obj_align(btn_exit, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_set_style_bg_opa(btn_exit, LV_OPA_TRANSP, 0);
    lv_obj_set_style_radius(btn_exit, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_shadow_width(btn_exit, 0, 0);
    lv_obj_t *label_exit = lv_img_create(btn_exit);
    lv_img_set_src(label_exit, &icon_exit_32);
    lv_obj_center(label_exit);

    lv_obj_add_event_cb(s_vol_cont, _event_cb, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(s_vol_cont, _event_cb, LV_EVENT_PRESS_LOST, NULL);
    lv_obj_add_event_cb(s_vol_cont, _event_cb, LV_EVENT_RELEASED, NULL);
    lv_obj_add_event_cb(s_vol_cont, _event_cb, LV_EVENT_GESTURE, NULL);
    lv_obj_add_event_cb(btn_exit, _event_cb, LV_EVENT_SHORT_CLICKED, NULL);

    lv_obj_add_event_cb(btn_minus, _on_volume_btn_cb, LV_EVENT_SHORT_CLICKED, NULL);
    lv_obj_add_event_cb(btn_plus, _on_volume_btn_cb, LV_EVENT_SHORT_CLICKED, NULL);

    lv_obj_clear_flag(s_vol_cont, LV_OBJ_FLAG_GESTURE_BUBBLE);

    lv_obj_add_flag(s_vol_cont, LV_OBJ_FLAG_HIDDEN);
}