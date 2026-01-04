#include "vw_setting_sleep.h"
#include "lvgl.h"
#include "page_manager.h"
#include "cl_ui.h"


LV_FONT_DECLARE(font_noto_28_4)

static setting_sleep_view_t vw = {0};
#define _this    vw


// 按键事件处理
static void _on_btn_cb(lv_event_t *e) {
    if (!vw.is_act) return;
    cl_button_t *btn = (cl_button_t*)lv_event_get_param(e);
    if (!btn) return;
    if (!btn) return;

    if (btn->id == CL_UI_KEY_POWER && btn->event == CL_BTN_CLICK)
    {
        page_change("setting");
    }
}


static void _on_ges_btn(lv_event_t *e){
    lv_event_code_t code = lv_event_get_code(e);
    if (lv_indev_get_gesture_dir(lv_indev_active()) == LV_DIR_RIGHT)
    {
        lv_indev_wait_release(lv_indev_active());
        page_change("setting");
    }
}

static void _on_item_click(lv_event_t *e){
    int sleep_time = (intptr_t)lv_event_get_user_data(e);
    lv_obj_t *obj = lv_event_get_target(e);
    if (_this.set_curr == obj)
    {
        return;
    }
    
    if (_this.set_curr)
    {
        lv_obj_set_style_bg_color(_this.set_curr, lv_color_hex(0x1c1c1c), 0);
    }
    _this.set_curr = obj;
    xz_setting_set_int("sleep", (int)sleep_time);
    set_shutdown_time((int)sleep_time);
    lv_obj_set_style_bg_color(_this.set_curr, lv_color_hex(0x53a9ff), 0);
}


static lv_obj_t* creat_sleep_time_item(lv_obj_t * parent, const char *name, int sleep_sec)
{

    lv_obj_t *priv = NULL;
    int count = lv_obj_get_child_count(parent);
    if (count != 0)
    {
        priv = lv_obj_get_child(parent, count-1);
    }


    // 创建容器，布局水平排列，宽度和父容器一样
    lv_obj_t *cont = lv_button_create(parent);
    lv_obj_set_size(cont, 300, 80);
    lv_obj_set_style_border_width(cont, 0, 0);
    lv_obj_set_style_bg_opa(cont, 200, 0);
    lv_obj_set_style_bg_color(cont, lv_color_hex(0x1c1c1c), 0);
    lv_obj_set_style_radius(cont, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_shadow_width(cont, 0, 0);
    lv_obj_set_style_shadow_opa(cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_shadow_ofs_x(cont, 0, 0);
    lv_obj_set_style_shadow_ofs_y(cont, 0, 0);


    lv_obj_t *label = lv_label_create(cont);
    lv_obj_set_style_text_font(label, &font_noto_28_4, 0);
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_label_set_text(label, name);
    lv_obj_align(label, LV_ALIGN_LEFT_MID, 10, 0);
    

    if (xz_setting_get_int("sleep", 300) == sleep_sec)
    {
        vw.set_curr = cont;    
        lv_obj_set_style_bg_color(cont, lv_color_hex(0x53A9FF), 0);
    }
    

    lv_obj_add_event_cb(cont, _on_item_click, LV_EVENT_CLICKED, (void*)sleep_sec);


    return cont;
    // 点击事件
    // lv_obj_add_event_cb(cont, alarm_long_press_cb, LV_EVENT_LONG_PRESSED, alarm);
}

setting_sleep_view_t* setting_sleep_view_create(lv_obj_t *root)
{

    vw.set_curr = NULL;
    vw.root = root;
    vw.set_target = NULL;

    lv_obj_remove_style_all(root);
    lv_obj_set_size(root, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_border_width(root, 0, 0);
    lv_obj_set_style_pad_all(root, 0, 0);
    lv_obj_remove_flag(root, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_center(root);
    lv_obj_set_style_bg_color(root, lv_color_hex(0x000000), 0);

    lv_obj_t *item_cont = lv_obj_create(root);
    lv_obj_set_scroll_dir(item_cont, LV_DIR_VER);
    lv_obj_align(item_cont, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_size(item_cont, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_border_width(item_cont, 0, 0);
    lv_obj_set_style_bg_color(item_cont, lv_color_hex(0x000000), 0);
    lv_obj_set_style_radius(item_cont, 0, 0);
    lv_obj_set_scrollbar_mode(item_cont, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_flex_flow(item_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(item_cont, 5,0);
    lv_obj_set_flex_align(item_cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);


    lv_obj_t *top_blank = lv_obj_create(item_cont);
    lv_obj_set_size(top_blank, LV_HOR_RES, 20);
    lv_obj_set_style_bg_color(top_blank, lv_color_hex(0x000000), 0);
    lv_obj_set_style_border_width(top_blank, 0, 0);

    lv_obj_t *title_label = lv_label_create(item_cont);

    lv_obj_set_style_text_font(title_label, &font_noto_28_4, 0);
    lv_label_set_text(title_label, "自动关机");
    lv_obj_set_height(title_label, 50);
    lv_obj_set_style_text_color(title_label, lv_color_hex(0xe9feff), 0);
    
    creat_sleep_time_item(item_cont, "从不",    -1);
    creat_sleep_time_item(item_cont, "5分钟",   5*60);
    creat_sleep_time_item(item_cont, "10分钟",  10*60);
    creat_sleep_time_item(item_cont, "30分钟",  30*60);
    creat_sleep_time_item(item_cont, "1小时",   60*60);
    creat_sleep_time_item(item_cont, "2小时",   2*60*60);
    creat_sleep_time_item(item_cont, "3小时",   3*60*60);
    creat_sleep_time_item(item_cont, "4小时",   4*60*60);
    creat_sleep_time_item(item_cont, "5小时",   5*60*60);
    creat_sleep_time_item(item_cont, "6小时",   6*60*60);

    lv_obj_t *bottom_blank = lv_obj_create(item_cont);
    lv_obj_set_size(bottom_blank, LV_HOR_RES, 100);
    lv_obj_set_style_bg_color(bottom_blank, lv_color_hex(0x000000), 0);
    lv_obj_set_style_border_width(bottom_blank, 0, 0);

    if (vw.set_curr)
    {
        lv_group_focus_obj(vw.set_curr);
    }

    vw.is_act = 0;

    lv_obj_add_event_cb(root, _on_btn_cb, CL_UI_EVENT_BUTTON, NULL);
    lv_obj_add_event_cb(root, _on_ges_btn, LV_EVENT_GESTURE, NULL);
    lv_obj_clear_flag(root, LV_OBJ_FLAG_GESTURE_BUBBLE);
    return &vw;
}

void setting_sleep_view_delete(void) {}