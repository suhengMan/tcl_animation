#include "lvgl.h"
#include "cl_ui.h"
#include "vw_setting.h"
#include "lv_toast/lv_toast.h"

static setting_view_t vw;

#define TAG "vw_setting"
LV_FONT_DECLARE(font_noto_28_4)
LV_IMG_DECLARE(icon_power_64)
LV_IMG_DECLARE(icon_play_64)
LV_IMG_DECLARE(icon_about_64)

static void _on_btn_cb(lv_event_t *e)
{
    if (!vw.is_act) return;

    cl_button_t *btn = (cl_button_t*)lv_event_get_param(e);
    if (!btn) return;

    if (btn->id == CL_UI_KEY_POWER && btn->event == CL_BTN_CLICK)
    {
        if (lv_obj_has_flag(vw.cont_autoplay, LV_OBJ_FLAG_HIDDEN))
        {
            page_change("home");
        }else{
            lv_obj_add_flag(vw.cont_autoplay, LV_OBJ_FLAG_HIDDEN);
        }
    }
    
}


static void _on_ges_btn(lv_event_t *e){
    lv_event_code_t code = lv_event_get_code(e);
    if (lv_indev_get_gesture_dir(lv_indev_active()) == LV_DIR_RIGHT)
    {
        lv_indev_wait_release(lv_indev_active());
        if (lv_obj_has_flag(vw.cont_autoplay, LV_OBJ_FLAG_HIDDEN))
        {
            page_change("home");
        }else{
            lv_obj_add_flag(vw.cont_autoplay, LV_OBJ_FLAG_HIDDEN);
        }
        
    }
}


static lv_obj_t* creat_setting_item(lv_obj_t * parent, void *icon_src, const char *title, void (*on_click)(lv_event_t* e))
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
    lv_obj_set_style_pad_all(cont, 5, 0);
    lv_obj_set_style_border_width(cont, 0, 0);
    lv_obj_set_style_bg_opa(cont, 200, 0);
    lv_obj_set_style_bg_color(cont, lv_color_hex(0x383838), 0);
    lv_obj_set_style_radius(cont, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_shadow_width(cont, 0, 0);
    lv_obj_set_style_shadow_opa(cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_shadow_ofs_x(cont, 0, 0);
    lv_obj_set_style_shadow_ofs_y(cont, 0, 0);


    lv_obj_t *icon = lv_img_create(cont);
    lv_img_set_src(icon, icon_src);
    lv_obj_align(icon, LV_ALIGN_LEFT_MID, 2, 0);

    lv_obj_t *label = lv_label_create(cont);
    lv_obj_set_style_text_font(label, &font_noto_28_4, 0);
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_label_set_text(label, title);
    lv_obj_align_to(label, icon, LV_ALIGN_OUT_RIGHT_MID, 10, 0);
    
    if (on_click != NULL)
    {
        lv_obj_add_event_cb(cont, on_click, LV_EVENT_CLICKED, cont);
    }
    return cont;
    // 点击事件
    // lv_obj_add_event_cb(cont, alarm_long_press_cb, LV_EVENT_LONG_PRESSED, alarm);
}

static void _on_sleep_menu(lv_event_t *e){
    page_change("setting_sleep");
}

static void _on_dev_menu(lv_event_t *e){
    page_change("setting_dev");
}

static void _on_play_menu(lv_event_t *e){

    if (lv_obj_has_flag(vw.cont_autoplay, LV_OBJ_FLAG_HIDDEN))
    {
        lv_obj_scroll_to_y(vw.cont_autoplay, 0, LV_ANIM_OFF);
        lv_obj_clear_flag(vw.cont_autoplay, LV_OBJ_FLAG_HIDDEN);
    }   
}

static void _on_slider_img_auto_set_value_change(lv_event_t *e){
    lv_obj_t *slider = lv_event_get_current_target(e);
    lv_obj_t *label = lv_event_get_user_data(e);
    int val = (int)lv_slider_get_value(slider);
    vw.auto_play_time = val;
    lv_label_set_text_fmt(label, "%d", (int)lv_slider_get_value(slider));
}


static void _on_switch_img_auto_change(lv_event_t *e){
    lv_obj_t *switch_obj = lv_event_get_current_target(e);
    lv_obj_t *cont = lv_event_get_user_data(e);
    if (lv_obj_has_state(switch_obj, LV_STATE_CHECKED)) {
        lv_obj_clear_flag(cont, LV_OBJ_FLAG_HIDDEN);
        vw.auto_play_time = (int)lv_slider_get_value(vw.slider_img);
    } else {
        vw.auto_play_time = 0;
        lv_obj_add_flag(cont, LV_OBJ_FLAG_HIDDEN);
    }
}

static void _on_switch_anim_auto_change(lv_event_t *e){
    lv_obj_t *switch_obj = lv_event_get_current_target(e);
    if (lv_obj_has_state(switch_obj, LV_STATE_CHECKED)) {
        vw.auto_play_anim_time = 1;
    } else {
        vw.auto_play_anim_time = 0;
    }
}

static lv_obj_t *_create_autoplay_setting_cont(lv_obj_t *parent){
    lv_obj_t* main_cont = lv_obj_create(parent);
    lv_obj_center(main_cont);
    lv_obj_set_size(main_cont, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_border_width(main_cont, 0, 0);
    lv_obj_set_style_bg_color(main_cont, lv_color_hex(0x000000), 0);
    lv_obj_set_style_radius(main_cont, 0, 0);
    lv_obj_set_style_pad_left(main_cont, 0, 0);
    lv_obj_set_scroll_dir(main_cont, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(main_cont, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_flex_flow(main_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(main_cont, 40,0);
    lv_obj_set_flex_align(main_cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t *title = lv_label_create(main_cont);
    lv_obj_set_size(title, 280, 30);
    lv_obj_set_style_border_width(title, 0, 0);
    lv_obj_set_style_text_font(title, &font_noto_28_4, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0xffffff), 0);
    lv_label_set_text(title, "自动播放");
    lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_CENTER, 0);

    // 图片自动播放
    lv_obj_t *cont_img_auto = lv_obj_create(main_cont);
    lv_obj_set_size(cont_img_auto, 280, 80);
    lv_obj_set_style_border_width(cont_img_auto, 0, 0);
    lv_obj_set_style_bg_color(cont_img_auto, lv_color_hex(0x222222), 0);
    lv_obj_set_style_radius(cont_img_auto, LV_RADIUS_CIRCLE, 0);
    
    lv_obj_t *label_img_auto = lv_label_create(cont_img_auto);
    lv_obj_align(label_img_auto, LV_ALIGN_LEFT_MID, 10, 0);
    lv_obj_set_style_text_font(label_img_auto, &font_noto_28_4, 0);
    lv_obj_set_style_text_color(label_img_auto, lv_color_hex(0xffffff), 0);
    lv_label_set_text(label_img_auto, "图片");

    lv_obj_t *switch_img_auto = lv_switch_create(cont_img_auto);
    lv_obj_set_size(switch_img_auto, 80, 40);
    lv_obj_align(switch_img_auto, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_set_style_bg_color(switch_img_auto, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_radius(switch_img_auto, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(switch_img_auto, 0, 0);
    lv_obj_set_style_border_color(switch_img_auto, lv_color_hex(0xffffff), 0);
    /* 关闭状态下使用灰底 + 白色圆点，保持开启时默认配色 */
    lv_obj_set_style_bg_color(switch_img_auto, lv_color_hex(0x5a5a5a), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(switch_img_auto, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(switch_img_auto, lv_color_hex(0xffffff), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(switch_img_auto, LV_OPA_COVER, LV_PART_KNOB | LV_STATE_DEFAULT);

    lv_obj_t *cont_img_auto_set = lv_obj_create(main_cont);
    lv_obj_set_size(cont_img_auto_set, 280, 110);
    lv_obj_set_style_pad_all(cont_img_auto_set, 0, 0);
    lv_obj_set_style_border_width(cont_img_auto_set, 0, 0);
    lv_obj_set_style_bg_color(cont_img_auto_set, lv_color_hex(0x000000), 0);
    lv_obj_set_style_radius(cont_img_auto_set, 0, 0);
    lv_obj_clear_flag(cont_img_auto_set, LV_OBJ_FLAG_GESTURE_BUBBLE);
    lv_obj_t *label_img_auto_set = lv_label_create(cont_img_auto_set);
    lv_obj_align(label_img_auto_set, LV_ALIGN_OUT_LEFT_TOP, 15, 0);
    lv_obj_set_style_text_font(label_img_auto_set, cl_ui_get_font(), 0);
    lv_obj_set_style_text_color(label_img_auto_set, lv_color_hex(0xffffff), 0);
    lv_label_set_text(label_img_auto_set, "图片轮播间隔");

    lv_obj_t *cont_img_auto_set_value = lv_obj_create(cont_img_auto_set);
    lv_obj_align(cont_img_auto_set_value, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_clear_flag(cont_img_auto_set_value, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(cont_img_auto_set_value, 280, 80);
    lv_obj_set_style_border_width(cont_img_auto_set_value, 0, 0);
    lv_obj_set_style_bg_color(cont_img_auto_set_value, lv_color_hex(0x222222), 0);
    lv_obj_set_style_radius(cont_img_auto_set_value, LV_RADIUS_CIRCLE, 0);

    lv_obj_t *slider_img_auto_set_value = lv_slider_create(cont_img_auto_set_value);
    vw.slider_img = slider_img_auto_set_value;
    lv_obj_set_size(slider_img_auto_set_value, 240, 10);
    lv_obj_align(slider_img_auto_set_value, LV_ALIGN_LEFT_MID, 5, 0);
    lv_obj_set_style_border_width(slider_img_auto_set_value, 0, 0);
    lv_obj_set_style_bg_color(slider_img_auto_set_value, lv_color_hex(0xcccccc), 0);
    lv_obj_set_style_radius(slider_img_auto_set_value, LV_RADIUS_CIRCLE, 0);
    lv_slider_set_range(slider_img_auto_set_value, 1, 60);
    lv_obj_t *label_img_auto_set_value = lv_label_create(cont_img_auto_set);
    lv_obj_align(label_img_auto_set_value, LV_ALIGN_TOP_RIGHT, -20, 0);
    lv_obj_set_style_text_font(label_img_auto_set_value, cl_ui_get_font(), 0);
    lv_obj_set_style_text_color(label_img_auto_set_value, lv_color_hex(0xffffff), 0);
    lv_label_set_text_fmt(label_img_auto_set_value, "%d", (int)lv_slider_get_value(slider_img_auto_set_value));
    lv_obj_add_event_cb(slider_img_auto_set_value, _on_slider_img_auto_set_value_change, LV_EVENT_VALUE_CHANGED, label_img_auto_set_value);


    // 动画自动播放
    lv_obj_t *cont_anim_auto = lv_obj_create(main_cont);
    lv_obj_align_to(cont_anim_auto, cont_img_auto, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 20);
    lv_obj_set_size(cont_anim_auto, 280, 80);
    lv_obj_set_style_border_width(cont_anim_auto, 0, 0);
    lv_obj_set_style_bg_color(cont_anim_auto, lv_color_hex(0x222222), 0);
    lv_obj_set_style_radius(cont_anim_auto, LV_RADIUS_CIRCLE, 0);
    
    lv_obj_t *label_anim_auto = lv_label_create(cont_anim_auto);
    lv_obj_align(label_anim_auto, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_set_style_text_font(label_anim_auto, &font_noto_28_4, 0);
    lv_obj_set_style_text_color(label_anim_auto, lv_color_hex(0xffffff), 0);
    lv_label_set_text(label_anim_auto, "动画");

    lv_obj_t *switch_anim_auto = lv_switch_create(cont_anim_auto);
    lv_obj_set_size(switch_anim_auto, 80, 40);
    lv_obj_align(switch_anim_auto, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_set_style_bg_color(switch_anim_auto, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_radius(switch_anim_auto, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(switch_anim_auto, 0, 0);
    lv_obj_set_style_border_color(switch_anim_auto, lv_color_hex(0xffffff), 0);
    /* 关闭状态下使用灰底 + 白色圆点，保持开启时默认配色 */
    lv_obj_set_style_bg_color(switch_anim_auto, lv_color_hex(0x5a5a5a), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(switch_anim_auto, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(switch_anim_auto, lv_color_hex(0xffffff), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(switch_anim_auto, LV_OPA_COVER, LV_PART_KNOB | LV_STATE_DEFAULT);

 
    lv_obj_t *blank = lv_obj_create(main_cont);
    lv_obj_set_size(blank, LV_HOR_RES, 100);
    lv_obj_set_style_bg_color(blank, lv_color_hex(0x000000), 0);
    lv_obj_set_style_border_width(blank, 0, 0);
    lv_obj_set_style_radius(blank, 0, 0);
    lv_obj_align(blank, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_align_to(blank, cont_anim_auto, LV_ALIGN_OUT_BOTTOM_MID, 0, 20);

    if (vw.auto_play_time > 0)
    {
        lv_obj_add_state(switch_img_auto, LV_STATE_CHECKED);
        lv_slider_set_value(slider_img_auto_set_value, vw.auto_play_time, LV_ANIM_OFF);
        lv_label_set_text_fmt(label_img_auto_set_value, "%d", (int)lv_slider_get_value(slider_img_auto_set_value));
    }else{
        lv_obj_add_flag(cont_img_auto_set, LV_OBJ_FLAG_HIDDEN);
    }
    
    if (vw.auto_play_anim_time > 0)
    {
        lv_obj_add_state(switch_anim_auto, LV_STATE_CHECKED);
    }

    lv_obj_add_event_cb(switch_anim_auto, _on_switch_anim_auto_change, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(switch_img_auto, _on_switch_img_auto_change, LV_EVENT_VALUE_CHANGED, cont_img_auto_set);
    lv_obj_add_flag(main_cont, LV_OBJ_FLAG_HIDDEN);
    return main_cont;
}

static void _on_btn_debug(lv_event_t *e){
    int id = (intptr_t)lv_event_get_user_data(e);
    static int count1 = 0, count2 = 0;
    static int32_t last_tick = 0;
    int32_t tick = lv_tick_get();
    // ESP_LOGI(TAG, "button id: %d, count1: %d, count2: %d", id, count1, count2);

    if (id == 1) {
        if (tick - last_tick > 1000) {
            count1 = 1;
        } else {
            count1++;
        }
        last_tick = tick;
        count2 = 0; // 每次点ID=1都清空ID=2计数
        if (count1 == 3) {
            // 三连击 1 成功后，可开始点 2
            // 可以插入 id=1 三连击达成的视觉/逻辑反馈
        }
    } else if (id == 2) {
        // 只有在1刚好点满三次后，2才可计数
        
    // ESP_LOGW(TAG, "button id: %d, count1: %d, count2: %d", id, count1, count2);
        if (count1 >= 3) {
            if (tick - last_tick > 1000) {
                count1 = 0;
                count2 = 0;
            } else {
                count2++;
            }
            last_tick = tick;
            if (count2 == 3) {
                lv_toast_show("debug mode", 1000);
                xz_setting_set_int("debug", 1);
            }else if (count2 >= 10)
            {
                count1 = 0;
                count2 = 0;
                last_tick = 0;
                lv_toast_show("debug mode 1", 1000);
                xz_setting_set_int("debug", 3);
            }
        } else {
            // 如果1没到三下，点2就清空状态
            count1 = 0;
            count2 = 0;
            last_tick = 0;
        }
    }
}

setting_view_t* setting_view_create(lv_obj_t *root)
{
    lv_obj_remove_style_all(root);
    lv_obj_set_size(root, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_bg_color(root, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(root, LV_OPA_COVER, 0);

    // 新建一个cont，使用flex纵向布局
    lv_obj_t *cont = lv_obj_create(root);
    lv_obj_set_size(cont, LV_PCT(100), LV_PCT(100));  // 填满父容器
    lv_obj_set_style_border_width(cont, 0, 0);
    lv_obj_set_style_bg_opa(cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_top(cont, 30, 0);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN); // 纵向flex布局
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_t *label = lv_label_create(cont);
    lv_obj_set_height(label, 40);
    lv_obj_set_style_text_font(label, &font_noto_28_4, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
    lv_label_set_text(label, "设置");

    creat_setting_item(cont, &icon_power_64, "自动关机", _on_sleep_menu);
    creat_setting_item(cont, &icon_play_64, "自动播放", _on_play_menu);
    
    if (xz_setting_get_int("debug", 0) == 3)
    {
        creat_setting_item(cont, &icon_about_64, "开发者设置", _on_dev_menu);
    }else{
        lv_obj_t *btn_1 = lv_btn_create(root);
        lv_obj_set_size(btn_1, 100, 60);
        lv_obj_align(btn_1, LV_ALIGN_BOTTOM_MID, -60, -20);
        lv_obj_set_style_shadow_width(btn_1, 0, 0);
        lv_obj_add_event_cb(btn_1, _on_btn_debug, LV_EVENT_SHORT_CLICKED, (void*)1);
    
        lv_obj_t *btn_2 = lv_btn_create(root);
        lv_obj_align(btn_2, LV_ALIGN_BOTTOM_MID, 60, -20);
        lv_obj_set_style_shadow_width(btn_2, 0, 0);
        lv_obj_set_size(btn_2, 100, 60);
        lv_obj_set_style_bg_opa(btn_1, LV_OPA_TRANSP,0);
        lv_obj_set_style_bg_opa(btn_2, LV_OPA_TRANSP,0);
        lv_obj_add_event_cb(btn_2, _on_btn_debug, LV_EVENT_SHORT_CLICKED, (void*)2);    
    }
    
    vw.cont_autoplay = _create_autoplay_setting_cont(root);
    lv_obj_add_event_cb(root, _on_btn_cb, CL_UI_EVENT_BUTTON, NULL);
    lv_obj_add_event_cb(root, _on_ges_btn, LV_EVENT_GESTURE, NULL);
    lv_obj_clear_flag(root, LV_OBJ_FLAG_GESTURE_BUBBLE);
    
    return &vw;
}

void setting_view_delete(void)
{
    xz_setting_set_int("img_play", vw.auto_play_time);
    xz_setting_set_int("anim_play", vw.auto_play_anim_time);
}
