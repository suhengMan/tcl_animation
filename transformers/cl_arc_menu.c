#include "cl_ui.h"
#include "lvgl.h"
#ifndef SIMULATOR
#include "vb_adapter.h"
#endif
#include "lv_arc_menu/lv_arc_menu.h"

#define TAG "arc_menu"

static lv_obj_t *g_arc_menu = NULL;
static lv_obj_t *s_bat_bar = NULL;
static lv_timer_t *s_timer = NULL;

LV_IMG_DECLARE(icon_home_28)
LV_IMG_DECLARE(icon_alarm_28)
LV_IMG_DECLARE(icon_setting_28)
LV_IMG_DECLARE(icon_music_28)
LV_IMG_DECLARE(icon_video_28)
LV_IMG_DECLARE(icon_image_28)
LV_IMG_DECLARE(icon_countdown_32)



void cl_arc_menu_show(bool show){
    if (g_arc_menu == NULL)
    {
        return;
    }
    
    int32_t y = lv_obj_get_y(g_arc_menu);
    lv_anim_t anim;
    lv_anim_init(&anim);
    lv_anim_set_var(&anim, g_arc_menu);
    lv_anim_set_time(&anim, 300); // 动画时长300ms，可根据需求调整
    lv_anim_set_exec_cb(&anim, (lv_anim_exec_xcb_t)lv_obj_set_y);
    if (show && y > 0)
    {
        lv_anim_set_values(&anim, y, 0);
        lv_anim_start(&anim);
    }else if (!show && y < 360)
    {
        lv_anim_set_values(&anim, y, 360);
        lv_anim_start(&anim);
    }
}


static void _alarm_click(lv_event_t *e){
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_SHORT_CLICKED)
    {
        cl_arc_menu_show(0);
        page_change("alarm");
        // vb_time_t t = {0};
        // vb_api_get_time(&t);
        // ESP_LOGI(TAG, "t: %04lu-%02lu-%02lu %02lu:%02lu:%02lu", t.year, t.month, t.day, t.hour, t.min, t.sec);
    }
}

static void _setting_click(lv_event_t *e){
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_SHORT_CLICKED)
    {
        cl_arc_menu_show(0);
        page_change("setting");
    }
}

static void _home_click(lv_event_t *e){
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_SHORT_CLICKED)
    {
        cl_arc_menu_show(0);
        page_change("home");
    }
}

static void _music_click(lv_event_t *e){
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_SHORT_CLICKED)
    {
        cl_arc_menu_show(0);
        page_change("music_fft");
    }
}

static void _video_click(lv_event_t *e){
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_SHORT_CLICKED)
    {
        cl_arc_menu_show(0);
        page_change("anim");
        // lv_tileview_set_tile(vw.tv, vw.tv_video, LV_ANIM_OFF);
    }
}

static void _img_click(lv_event_t *e){
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_SHORT_CLICKED)
    {
        cl_arc_menu_show(0);
        page_change("img_play");
    }
}

static void _countdown_click(lv_event_t *e){
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_SHORT_CLICKED)
    {
        cl_arc_menu_show(0);
        page_change("countdown");
    }
}

static void _menu_event_cb(lv_event_t *e){
    lv_event_code_t code = lv_event_get_code(e);   
    if (code == LV_EVENT_CLICKED)
    {
        cl_arc_menu_show(0);
    }
}

static void _timer_check_bat(lv_timer_t *t){
    static uint8_t tick = 0;
    tick = (tick+1)%200;
    if (xz_sys_is_charg() == true)
    {
        lv_obj_set_style_bg_color(s_bat_bar, lv_color_hex(0x3ee923), LV_PART_INDICATOR);
    }else{
        lv_obj_set_style_bg_color(s_bat_bar, lv_color_hex(0xffffff), LV_PART_INDICATOR);
    }
    if (tick%25 == 0)
    {
#ifndef SIMULATOR
        int val = vb_api_get_battery_level();
        if (val >= 95)
        {
            val = 100;
        }
        
        lv_bar_set_value(s_bat_bar, val, LV_ANIM_OFF);
#endif
    }
    

}

void cl_init_arc_menu(){
    lv_obj_t *cont_menu = lv_obj_create(lv_layer_top());
    g_arc_menu = cont_menu;
    lv_obj_set_style_border_width(cont_menu, 0, 0);
    lv_obj_set_size(cont_menu, 360, 360);
    lv_obj_clear_flag(cont_menu, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(lv_layer_top(), LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(cont_menu, LV_OPA_40, 0);
    lv_obj_set_style_bg_color(cont_menu, lv_color_black(), 0);

    lv_obj_t *arc_menu = lv_arc_menu_create(cont_menu);
    lv_obj_set_size(arc_menu, 240, 240);
    lv_obj_center(arc_menu);
    lv_arc_menu_add_btn(arc_menu, &icon_home_28, _home_click);
    lv_arc_menu_add_btn(arc_menu, &icon_alarm_28, _alarm_click);
    lv_arc_menu_add_btn(arc_menu, &icon_setting_28, _setting_click);
    lv_arc_menu_add_btn(arc_menu, &icon_music_28, _music_click);
    lv_arc_menu_add_btn(arc_menu, &icon_video_28, _video_click);
    lv_arc_menu_add_btn(arc_menu, &icon_image_28, _img_click);
    lv_arc_menu_add_btn(arc_menu, &icon_countdown_32, _countdown_click);
    lv_arc_menu_set_rotate(arc_menu, -90);
    

    lv_obj_t *bar = lv_bar_create(cont_menu);
    s_bat_bar = bar;
    lv_obj_set_size(bar, 30, 15);
    // lv_obj_center(bar);
    lv_obj_align(bar, LV_ALIGN_BOTTOM_MID, 0, -5);
    lv_obj_set_style_bg_color(bar, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(bar, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(bar, 2, 0);
    lv_obj_set_style_border_width(bar, 1, 0);
    lv_obj_set_style_border_color(bar, lv_color_hex(0xFFFFFF), 0);
    lv_obj_clear_flag(bar, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(bar, 1, LV_PART_INDICATOR); 
    lv_obj_set_style_radius(bar, 3, LV_PART_MAIN); 
    lv_obj_set_style_bg_color(bar, lv_color_hex(0x3ee923), LV_PART_INDICATOR);
    lv_obj_t *bar_t = lv_obj_create(cont_menu);
    lv_obj_set_size(bar_t, 2, 8);
    lv_obj_set_style_bg_color(bar_t, lv_color_white(), 0);
    lv_obj_set_style_border_width(bar_t, 0, 0);
    lv_obj_align_to(bar_t, bar, LV_ALIGN_OUT_RIGHT_MID, 2, 0);

#ifndef SIMULATOR
    int val = vb_api_get_battery_level();
    lv_bar_set_value(s_bat_bar, val, LV_ANIM_OFF);
#else
    lv_bar_set_value(s_bat_bar, 50, LV_ANIM_OFF);
#endif

    lv_obj_set_pos(cont_menu, 0, 360);

    s_timer = lv_timer_create(_timer_check_bat, 200, NULL);
    lv_obj_add_event_cb(cont_menu, _menu_event_cb, LV_EVENT_CLICKED, NULL);
}