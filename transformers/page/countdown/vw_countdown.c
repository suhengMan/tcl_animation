#include "lvgl.h"
#include "cl_ui.h"
#include "vw_countdown.h"

static countdown_view_t vw;

LV_FONT_DECLARE(font_noto_28_4)
LV_FONT_DECLARE(font_noto_num_32_4)
LV_FONT_DECLARE(font_noto_num_48_4)

LV_IMG_DECLARE(icon_confirm_24)
LV_IMG_DECLARE(icon_play_24)
LV_IMG_DECLARE(icon_pause_24)
LV_IMG_DECLARE(icon_del_24)

#define TAG "vw_countdown"
#define HOUR_OPTIONS "00\n01\n02\n03\n04\n05\n06\n07\n08\n09\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23\n24"
#define MINUTE_OPTIONS "00\n01\n02\n03\n04\n05\n06\n07\n08\n09\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23\n24\n25\n26\n27\n28\n29\n30\n31\n32\n33\n34\n35\n36\n37\n38\n39\n40\n41\n42\n43\n44\n45\n46\n47\n48\n49\n50\n51\n52\n53\n54\n55\n56\n57\n58\n59"
#define SECOND_OPTIONS MINUTE_OPTIONS


// roller拖动事件处理
static void roller_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *roller = lv_event_get_current_target(e);
    
    if (code == LV_EVENT_PRESSED) {
        // 将所有roller的选中项文字颜色设为白色
        if (vw.roller_hour) {
            lv_obj_set_style_text_color(vw.roller_hour, lv_color_hex(0xFFFFFF), LV_PART_SELECTED);
        }
        if (vw.roller_min) {
            lv_obj_set_style_text_color(vw.roller_min, lv_color_hex(0xFFFFFF), LV_PART_SELECTED);
        }
        if (vw.roller_sec) {
            lv_obj_set_style_text_color(vw.roller_sec, lv_color_hex(0xFFFFFF), LV_PART_SELECTED);
        }
        // 设置当前roller的选中项文字颜色为蓝色
        lv_obj_set_style_text_color(roller, lv_color_hex(0x0080FF), LV_PART_SELECTED);
    }
}

// 为roller创建渐变覆盖层
static void create_fade_overlay(lv_obj_t *parent, lv_obj_t *roller)
{
    
    // 创建顶部渐变覆盖层（从黑色到透明）
    lv_obj_t *top_fade = lv_obj_create(parent);
    lv_obj_set_size(top_fade, 280, 60);
    lv_obj_align(top_fade, LV_ALIGN_TOP_MID, 0, 0);
    lv_obj_set_style_bg_color(top_fade, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_grad_color(top_fade, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_grad_dir(top_fade, LV_GRAD_DIR_VER, 0);
    lv_obj_set_style_bg_main_stop(top_fade, 20, 0);
    lv_obj_set_style_bg_grad_stop(top_fade, 255, 0);
    lv_obj_set_style_bg_main_opa(top_fade, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_grad_opa(top_fade, LV_OPA_60, 0);
    lv_obj_set_style_border_width(top_fade, 0, 0);
    lv_obj_clear_flag(top_fade, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(top_fade, LV_OBJ_FLAG_CLICKABLE);
    
    // 创建底部渐变覆盖层（从透明到黑色）
    lv_obj_t *bottom_fade = lv_obj_create(parent);
    lv_obj_set_size(bottom_fade, 300, 60);
    lv_obj_align(bottom_fade, LV_ALIGN_BOTTOM_MID, 0, 0);
    lv_obj_set_style_bg_color(bottom_fade, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_grad_color(bottom_fade, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_grad_dir(bottom_fade, LV_GRAD_DIR_VER, 0);
    lv_obj_set_style_bg_main_stop(bottom_fade, 20, 0);
    lv_obj_set_style_bg_grad_stop(bottom_fade, 255, 0);
    lv_obj_set_style_bg_main_opa(bottom_fade, LV_OPA_60, 0);
    lv_obj_set_style_bg_grad_opa(bottom_fade, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(bottom_fade, 0, 0);
    lv_obj_clear_flag(bottom_fade, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_clear_flag(bottom_fade, LV_OBJ_FLAG_CLICKABLE);
}

static void _on_btn_cb(lv_event_t *e)
{
    if (!vw.is_act) return;

    cl_button_t *btn = (cl_button_t*)lv_event_get_param(e);
    if (!btn) return;
    if (btn->id == CL_UI_KEY_POWER && btn->event == CL_BTN_CLICK)
    {
        if (!lv_obj_has_flag(vw.countdown_cont, LV_OBJ_FLAG_HIDDEN))
        {
            if (vw.timer_ui)
            {
                lv_timer_del(vw.timer_ui);
                vw.timer_ui = NULL;
            }
            vw.last_time = 0;
            lv_obj_add_flag(vw.countdown_cont, LV_OBJ_FLAG_HIDDEN);               
        }
    }
     
}

static void _countdown_timer(lv_timer_t *t){
    uint32_t now = lv_tick_get();
    uint32_t diff = now-vw.last_time;
    vw.last_time = now;
    if (vw.curr_time > diff)
    {
        vw.curr_time -= diff;
        uint32_t left_hour = vw.curr_time/3600000;
        uint32_t left_min = (vw.curr_time/60000)%60;
        uint32_t left_sec = (vw.curr_time/1000)%60;
        uint16_t percent = (uint16_t)((uint32_t)vw.curr_time*1000/vw.set_time);
        if (vw.is_act)
        {
            lv_label_set_text_fmt(vw.countdown_time, "%02ld:%02ld:%02ld", left_hour, left_min, left_sec+1);
            lv_arc_set_value(vw.countdown_arc, percent);
        }
    }else{
        lv_timer_del(t);
        vw.timer_ui = NULL;
        if (vw.is_act)
        {
            lv_obj_add_flag(vw.countdown_cont, LV_OBJ_FLAG_HIDDEN);
        }
        uint32_t time = vw.set_time;
        page_change_with_arg("countdown_done", &time, sizeof(uint32_t));
        
    }
}

static void on_confirm_btn(lv_event_t *e){
    uint32_t hour = lv_roller_get_selected(vw.roller_hour);
    uint32_t min = lv_roller_get_selected(vw.roller_min);
    uint32_t s = lv_roller_get_selected(vw.roller_sec);
    vw.set_time = (hour*3600+min*60+s)*1000;
    vw.curr_time = vw.set_time;
    vw.last_time = lv_tick_get();
    if (vw.set_time <= 0)
    {
        return;
    }
    if (vw.timer_ui)
    {
        lv_timer_delete(vw.timer_ui);
        vw.timer_ui = NULL;
    }
    lv_obj_t *btn = lv_obj_get_child(vw.countdown_cont, -1);
    lv_obj_t *icon = lv_obj_get_child(btn, 0);
    lv_img_set_src(icon, &icon_pause_24);
    vw.timer_ui = lv_timer_create(_countdown_timer, 20, NULL);
    lv_label_set_text(vw.countdown_title, "正在计时");
    lv_obj_remove_flag(vw.countdown_cont, LV_OBJ_FLAG_HIDDEN);
}

void start_countdown(uint32_t time){
    vw.set_time = time;
    vw.curr_time = vw.set_time;
    vw.last_time = lv_tick_get();
    if (vw.set_time <= 0)
    {
        return;
    }
    if (vw.timer_ui)
    {
        lv_timer_delete(vw.timer_ui);
        vw.timer_ui = NULL;
    }
    if (vw.is_act)
    {
        lv_obj_t *btn = lv_obj_get_child(vw.countdown_cont, -1);
        lv_obj_t *icon = lv_obj_get_child(btn, 0);
        lv_img_set_src(icon, &icon_pause_24);
        lv_label_set_text(vw.countdown_title, "正在计时");
        if (lv_obj_has_flag(vw.countdown_cont, LV_OBJ_FLAG_HIDDEN))
        {
            lv_obj_remove_flag(vw.countdown_cont, LV_OBJ_FLAG_HIDDEN);
        }
    }
    
    vw.timer_ui = lv_timer_create(_countdown_timer, 20, NULL);
    
}

static lv_obj_t *create_set_cont(lv_obj_t *parent){
    lv_obj_t *cont = lv_obj_create(parent);
    lv_obj_set_size(cont, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_bg_color(cont, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(cont, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(cont, 0, 0);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *label_title = lv_label_create(cont);
    lv_obj_set_style_text_font(label_title, &font_noto_28_4, 0);
    lv_obj_set_style_text_color(label_title, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_align(label_title, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_text(label_title, "倒计时设置");
    lv_obj_align(label_title, LV_ALIGN_TOP_MID, 0, 25);

    // 创建容器用于放置时分秒roller和冒号
    lv_obj_t *time_cont = lv_obj_create(cont);
    lv_obj_set_size(time_cont, 300, 200);
    lv_obj_set_style_pad_all(time_cont, 0, 0);
    lv_obj_set_style_bg_opa(time_cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(time_cont, 0, 0);
    lv_obj_clear_flag(time_cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_align(time_cont, LV_ALIGN_CENTER, 0, 0);

    // 创建分钟roller
    vw.roller_min = lv_roller_create(time_cont);
    lv_obj_set_size(vw.roller_min, 80, 200);
    lv_obj_set_style_bg_opa(vw.roller_min, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(vw.roller_min, 0, 0);
    lv_obj_clear_flag(vw.roller_min, LV_OBJ_FLAG_SCROLLABLE);
    lv_roller_set_options(vw.roller_min, MINUTE_OPTIONS, LV_ROLLER_MODE_INFINITE);
    lv_roller_set_visible_row_count(vw.roller_min, 8);
    // 设置字体和颜色
    lv_obj_set_style_text_font(vw.roller_min, &font_noto_num_32_4, 0);
    lv_obj_set_style_text_font(vw.roller_min, &font_noto_num_48_4, LV_PART_SELECTED);
    lv_obj_set_style_text_color(vw.roller_min, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_color(vw.roller_min, lv_color_hex(0xFFFFFF), LV_PART_SELECTED);
    // 设置选中区域背景为黑色
    lv_obj_set_style_bg_color(vw.roller_min, lv_color_hex(0x000000), LV_PART_SELECTED);
    lv_obj_set_style_bg_opa(vw.roller_min, LV_OPA_COVER, LV_PART_SELECTED);
    // 设置选项之间的行间距（上下间距）
    lv_obj_set_style_text_line_space(vw.roller_min, 32, LV_PART_MAIN);
    lv_obj_align(vw.roller_min, LV_ALIGN_CENTER, 0, 0);
    lv_obj_add_event_cb(vw.roller_min, roller_event_cb, LV_EVENT_PRESSED, NULL);

    // 创建第一个冒号
    lv_obj_t *label_colon1 = lv_label_create(time_cont);
    lv_obj_set_style_text_font(label_colon1, &font_noto_num_32_4, 0);
    lv_obj_set_style_text_color(label_colon1, lv_color_hex(0xFFFFFF), 0);
    lv_label_set_text(label_colon1, ":");
    lv_obj_align_to(label_colon1, vw.roller_min, LV_ALIGN_OUT_LEFT_MID, 0, 0);


    // 创建小时roller
    vw.roller_hour = lv_roller_create(time_cont);
    lv_obj_set_size(vw.roller_hour, 70, 200);
    lv_obj_set_style_bg_opa(vw.roller_hour, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(vw.roller_hour, 0, 0);
    lv_obj_clear_flag(vw.roller_hour, LV_OBJ_FLAG_SCROLLABLE);
    lv_roller_set_options(vw.roller_hour, HOUR_OPTIONS, LV_ROLLER_MODE_INFINITE);
    lv_roller_set_visible_row_count(vw.roller_hour, 8);
    // 设置字体和颜色
    lv_obj_set_style_text_font(vw.roller_hour, &font_noto_num_32_4, 0);
    lv_obj_set_style_text_font(vw.roller_hour, &font_noto_num_48_4, LV_PART_SELECTED);
    lv_obj_set_style_text_color(vw.roller_hour, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_color(vw.roller_hour, lv_color_hex(0xFFFFFF), LV_PART_SELECTED);
    // 设置选中区域背景为黑色
    lv_obj_set_style_bg_color(vw.roller_hour, lv_color_hex(0x000000), LV_PART_SELECTED);
    lv_obj_set_style_bg_opa(vw.roller_hour, LV_OPA_COVER, LV_PART_SELECTED);
    // 设置选项之间的行间距（上下间距）
    lv_obj_set_style_text_line_space(vw.roller_hour, 32, LV_PART_MAIN);
    lv_obj_add_event_cb(vw.roller_hour, roller_event_cb, LV_EVENT_PRESSED, NULL);
    lv_obj_align_to(vw.roller_hour, label_colon1, LV_ALIGN_OUT_LEFT_MID, -5, 0);
    // create_fade_overlay(time_cont, vw.roller_hour);

    

    // 创建第二个冒号
    lv_obj_t *label_colon2 = lv_label_create(time_cont);
    lv_obj_set_style_text_font(label_colon2, &font_noto_num_32_4, 0);
    lv_obj_set_style_text_color(label_colon2, lv_color_hex(0xFFFFFF), 0);
    lv_label_set_text(label_colon2, ":");
    lv_obj_align_to(label_colon2, vw.roller_min, LV_ALIGN_OUT_RIGHT_MID, 0, 0);

    // 创建秒roller
    vw.roller_sec = lv_roller_create(time_cont);
    lv_obj_set_size(vw.roller_sec, 80, 200);
    lv_obj_set_style_bg_opa(vw.roller_sec, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(vw.roller_sec, 0, 0);
    lv_obj_clear_flag(vw.roller_sec, LV_OBJ_FLAG_SCROLLABLE);
    lv_roller_set_options(vw.roller_sec, SECOND_OPTIONS, LV_ROLLER_MODE_INFINITE);
    lv_roller_set_visible_row_count(vw.roller_sec, 8);
    // 设置字体和颜色
    lv_obj_set_style_text_font(vw.roller_sec, &font_noto_num_32_4, 0);
    lv_obj_set_style_text_font(vw.roller_sec, &font_noto_num_48_4, LV_PART_SELECTED);
    lv_obj_set_style_text_color(vw.roller_sec, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_color(vw.roller_sec, lv_color_hex(0xFFFFFF), LV_PART_SELECTED);
    // 设置选中区域背景为黑色
    lv_obj_set_style_bg_color(vw.roller_sec, lv_color_hex(0x000000), LV_PART_SELECTED);
    lv_obj_set_style_bg_opa(vw.roller_sec, LV_OPA_COVER, LV_PART_SELECTED);
    // 设置选项之间的行间距（上下间距）
    lv_obj_set_style_text_line_space(vw.roller_sec, 32, LV_PART_MAIN);
    lv_obj_align_to(vw.roller_sec, label_colon2, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
    lv_obj_add_event_cb(vw.roller_sec, roller_event_cb, LV_EVENT_PRESSED, NULL);
    create_fade_overlay(time_cont, vw.roller_sec);


    lv_obj_t *btn_confirm = lv_btn_create(cont);
    lv_obj_set_size(btn_confirm, 70, 70);
    lv_obj_align(btn_confirm, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_set_style_shadow_width(btn_confirm, 0, 0);
    lv_obj_set_style_bg_color(btn_confirm, lv_color_hex(0x0080FF), 0);
    lv_obj_set_style_bg_opa(btn_confirm, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(btn_confirm, LV_RADIUS_CIRCLE, 0);

    lv_obj_t *icon_confirm = lv_img_create(btn_confirm);
    lv_obj_center(icon_confirm);
    lv_img_set_src(icon_confirm, &icon_confirm_24);

    lv_obj_add_event_cb(btn_confirm, on_confirm_btn, LV_EVENT_CLICKED, NULL);
    return cont;
}

static void on_pause_click(lv_event_t *e){
    if (vw.timer_ui != NULL)
    {
        lv_timer_delete(vw.timer_ui);
        vw.timer_ui = NULL;
        lv_label_set_text(vw.countdown_title, "暂停计时");
        lv_obj_t *icon = lv_obj_get_child(lv_event_get_target(e), 0);
        lv_img_set_src(icon, &icon_play_24);
    }else{
        lv_label_set_text(vw.countdown_title, "正在计时");
        vw.last_time = (uint32_t)lv_tick_get();
        vw.timer_ui = lv_timer_create(_countdown_timer, 20, NULL);
        lv_obj_t *icon = lv_obj_get_child(lv_event_get_target(e), 0);
        lv_img_set_src(icon, &icon_pause_24);
    }
}

static void on_cancel_click(lv_event_t *e){
    if (vw.timer_ui)
    {
        lv_timer_del(vw.timer_ui);
        vw.timer_ui = NULL;
    }
    vw.last_time = 0;
    lv_obj_add_flag(vw.countdown_cont, LV_OBJ_FLAG_HIDDEN);
}

static lv_obj_t *create_countdown_cont(lv_obj_t *parent){
    lv_obj_t *cont = lv_obj_create(parent);
    lv_obj_set_size(cont, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_bg_color(cont, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(cont, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(cont, 0, 0);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *title = lv_label_create(cont);
    vw.countdown_title = title;
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 30);
    lv_obj_set_style_text_font(title, &font_noto_28_4, 0);
    lv_label_set_text(title, vw.timer_ui==NULL?"暂停计时":"正在计时");
    lv_obj_set_style_text_color(title, lv_color_hex(0x0080FF), 0);

    lv_obj_t *label_countdown = lv_label_create(cont);
    vw.countdown_time = label_countdown;
    lv_obj_align(vw.countdown_time, LV_ALIGN_CENTER, 0, -16);
    lv_obj_set_style_text_color(label_countdown, lv_color_white(), 0);
    lv_obj_set_style_text_font(label_countdown, &font_noto_num_48_4, 0);
    lv_label_set_text_fmt(label_countdown, "%02ld:%02ld:%02ld", vw.curr_time/3600000, (vw.curr_time/60000)%60, ((vw.curr_time/1000)%60)+1);

    lv_obj_t *bar = lv_arc_create(cont);
    vw.countdown_arc = bar;
    lv_obj_set_size(bar, 360, 360);
    lv_obj_center(bar);
    lv_arc_set_bg_angles(bar, 0, 360);
    lv_arc_set_rotation(bar, -90);
    lv_arc_set_range(bar, 0, 1000);
    uint16_t percent = 0;
    if (vw.set_time > 0)
    {
        percent = vw.curr_time*1000/vw.set_time;
    }
    lv_arc_set_value(bar, percent);
    
    lv_obj_remove_style(bar, NULL, LV_PART_KNOB);
    lv_obj_remove_flag(bar, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_style_arc_width(bar, 8, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(bar, lv_color_hex(0x0080FF), LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(bar, lv_color_black(), LV_PART_MAIN);

    lv_obj_t *btn_cancel = lv_btn_create(cont);
    lv_obj_set_size(btn_cancel, 80, 80);
    lv_obj_align(btn_cancel, LV_ALIGN_BOTTOM_MID, -50, -30);
    lv_obj_set_style_shadow_width(btn_cancel, 0, 0);
    lv_obj_set_style_bg_color(btn_cancel, lv_color_hex(0x1a1a1a), 0);
    lv_obj_set_style_bg_opa(btn_cancel, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(btn_cancel, LV_RADIUS_CIRCLE, 0);
    lv_obj_add_event_cb(btn_cancel, on_cancel_click, LV_EVENT_CLICKED, NULL);
    lv_obj_t *icon_cancel = lv_img_create(btn_cancel);
    lv_obj_center(icon_cancel);
    lv_img_set_src(icon_cancel, &icon_del_24);

    lv_obj_t *btn_pause = lv_btn_create(cont);
    lv_obj_set_size(btn_pause, 80, 80);
    lv_obj_align(btn_pause, LV_ALIGN_BOTTOM_MID, 50, -30);
    lv_obj_set_style_shadow_width(btn_pause, 0, 0);
    lv_obj_set_style_bg_color(btn_pause, lv_color_hex(0x0080FF), 0);
    lv_obj_set_style_bg_opa(btn_pause, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(btn_pause, LV_RADIUS_CIRCLE, 0);
    lv_obj_add_event_cb(btn_pause, on_pause_click, LV_EVENT_CLICKED, NULL);
    lv_obj_t *icon_pause = lv_img_create(btn_pause);
    lv_obj_center(icon_pause);
    lv_img_set_src(icon_pause, vw.timer_ui==NULL?&icon_play_24:&icon_pause_24);

    if (vw.timer_ui == NULL)
    {
        lv_obj_add_flag(cont, LV_OBJ_FLAG_HIDDEN);
    }
    
    return cont;
}

static void _root_ges_cb(lv_event_t *e){
    lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_active());
    if (dir == LV_DIR_RIGHT)
    {
        lv_indev_wait_release(lv_indev_active());
        page_change("home");
    }
}

countdown_view_t* countdown_view_create(lv_obj_t *root)
{
    lv_obj_remove_style_all(root);
    lv_obj_set_size(root, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_bg_color(root, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(root, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(root, 0, 0);
    lv_obj_clear_flag(root, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *set_cont = create_set_cont(root);
    vw.set_cont = set_cont;
    vw.countdown_cont = create_countdown_cont(root);


    lv_obj_add_event_cb(root, _on_btn_cb, CL_UI_EVENT_BUTTON, NULL);
    lv_obj_add_event_cb(root, _root_ges_cb, LV_EVENT_GESTURE, NULL);
    lv_obj_clear_flag(root, LV_OBJ_FLAG_GESTURE_BUBBLE);
    vw.is_act=1;
    return &vw;
}

void countdown_view_delete(void)
{
    /* 如需释放资源在此处理 */
}
