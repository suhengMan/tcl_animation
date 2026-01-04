#include "lvgl.h"
#include "cl_ui.h"
#include <time.h>
#include "lv_toast/lv_toast.h"
#include <string.h>
#include "vw_alarm.h"

static alarm_view_t vw;

#define TAG "vw_alarm"
LV_FONT_DECLARE(font_noto_num_32_4)
LV_FONT_DECLARE(font_noto_num_48_4)
LV_FONT_DECLARE(font_noto_28_4)
LV_IMG_DECLARE(icon_delete_28)
LV_IMG_DECLARE(icon_accept_28)
LV_IMG_DECLARE(icon_cancel_28)

#define HOUR_OPTIONS "00\n01\n02\n03\n04\n05\n06\n07\n08\n09\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23\n24"
#define MINUTE_OPTIONS "00\n01\n02\n03\n04\n05\n06\n07\n08\n09\n10\n11\n12\n13\n14\n15\n16\n17\n18\n19\n20\n21\n22\n23\n24\n25\n26\n27\n28\n29\n30\n31\n32\n33\n34\n35\n36\n37\n38\n39\n40\n41\n42\n43\n44\n45\n46\n47\n48\n49\n50\n51\n52\n53\n54\n55\n56\n57\n58\n59"
static const uint8_t btn_mode_bit[7] = {
    ALARM_MODE_EVERY_SUNDAY, // 周日 - bit7
    ALARM_MODE_EVERY_MONDAY, // 周一 - bit1
    ALARM_MODE_EVERY_TUESDAY, // 周二 - bit2
    ALARM_MODE_EVERY_WEDNESDAY, // 周三 - bit3
    ALARM_MODE_EVERY_THURSDAY, // 周四 - bit4
    ALARM_MODE_EVERY_FRIDAY, // 周五 - bit5
    ALARM_MODE_EVERY_SATURDAY  // 周六 - bit6
};

static void show_info_cont();
static void show_loop_edit_cont();
static void show_time_edit_cont();
void refresh_alarm();

static void _back_del_cont(lv_event_t *e){
    lv_obj_t *cont = lv_event_get_user_data(e);
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_GESTURE)
    {
        if (lv_indev_get_gesture_dir(lv_indev_active()) != LV_DIR_RIGHT)
        {
            return;
        }else{
            lv_indev_wait_release(lv_indev_active());
        }
    }
    
    if (cont)
    {
        if (cont == vw.cont_info)
        {
#ifndef SIMULATOR
            vb_alarm_set(&vw.alarm_edit);
#endif  
            refresh_alarm();
        }
        lv_obj_add_flag(cont, LV_OBJ_FLAG_HIDDEN);
    }
}

static void _on_del_btn(lv_event_t *e){
#ifndef SIMULATOR
    vb_alarm_del(vw.alarm_edit.index);
#endif
    refresh_alarm();
    lv_obj_add_flag(vw.cont_info, LV_OBJ_FLAG_HIDDEN);
}

static void _btn_time_accept(lv_event_t *e){
    lv_obj_t *cont = lv_event_get_user_data(e);
    vw.alarm_edit.hour = lv_roller_get_selected(vw.roller_hour);
    vw.alarm_edit.min = lv_roller_get_selected(vw.roller_min);
    if (vw.mode == ALARM_MODE_ADD)
    {
        show_loop_edit_cont();
    }else{
        lv_obj_add_flag(cont, LV_OBJ_FLAG_HIDDEN);
        show_info_cont();
    }
}

static void _btn_loop_save(lv_event_t *e){
    lv_obj_t *cont = lv_event_get_user_data(e);

    // 将vw.btn_week的checked状态映射回mode
    uint8_t mode = 0;
    for (uint i = 0; i < 7; i++)
    {
        if (lv_obj_get_state(vw.btn_week[i]) & LV_STATE_CHECKED)
        {
            mode |= btn_mode_bit[i];
        }
    }
    // 如果7个bit全为1，则设置为每天
    if ((mode & 0xFC) == 0xFC)
    {
        mode = ALARM_MODE_EVERY_DAY;
    }
    vw.alarm_edit.mode = mode;
    

    if (vw.mode == ALARM_MODE_EDIT)
    {
        show_info_cont();
        lv_obj_add_flag(vw.cont_loop_edit, LV_OBJ_FLAG_HIDDEN);
    }else{
        vw.alarm_edit.sw = 1;
        lv_obj_add_flag(vw.cont_time_edit, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(vw.cont_loop_edit, LV_OBJ_FLAG_HIDDEN);
#ifndef SIMULATOR
        vb_alarm_add(&vw.alarm_edit);
#endif
        refresh_alarm();
    }
}

static void _on_item_time(lv_event_t *e){
    vw.mode = ALARM_MODE_EDIT;
    show_time_edit_cont();
}

static void _on_item_loop(lv_event_t *e){
    vw.mode = ALARM_MODE_EDIT;
    show_loop_edit_cont();
}


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
            }
            break;
        default:
            break;
    }
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

// 为roller创建渐变覆盖层
static void create_fade_overlay(lv_obj_t *parent)
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

lv_obj_t *_create_loop_cont(lv_obj_t *loop){
    // 创建重复模式设置的容器
    lv_obj_t *cont = lv_obj_create(loop);
    lv_obj_set_size(cont, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_pad_all(cont, 0, 0);
    lv_obj_set_style_bg_color(cont, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(cont, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(cont, 0, 0);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);

    // 标题 "重复模式"
    lv_obj_t *title_label = lv_label_create(cont);
    lv_obj_set_style_text_font(title_label, &font_noto_28_4, 0);
    lv_obj_set_style_text_color(title_label, lv_color_hex(0xFFFFFF), 0);
    lv_label_set_text(title_label, "重复模式");
    lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, 30);

    // 圆形按钮容器
    static const char *week_text[7] = {"日", "一", "二", "三", "四", "五", "六"};
    lv_obj_t *btnmat_cont = lv_obj_create(cont);
    lv_obj_set_size(btnmat_cont, LV_HOR_RES, 200);
    lv_obj_set_style_bg_opa(btnmat_cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(btnmat_cont, 0, 0);
    lv_obj_set_style_pad_all(btnmat_cont, 0, 0);
    lv_obj_align(btnmat_cont, LV_ALIGN_TOP_MID, 0, 60);

    // 创建4+3按钮（两排, 稍微宽一点）
    int x0[4] = {-125, -42, 42, 125}; // 上排4按钮x偏移，稍微宽一点
    int x1[3] = {-85, 0, 85};         // 下排3按钮x偏移，稍微宽一点
    int y0 = 30;
    int y1 = 110;

    // 上排四个按钮
    for(int i = 0; i < 4; ++i) {
        vw.btn_week[i] = lv_btn_create(btnmat_cont);
        lv_obj_set_size(vw.btn_week[i], 75, 75);
        lv_obj_align(vw.btn_week[i], LV_ALIGN_TOP_MID, x0[i], y0);
        lv_obj_set_style_radius(vw.btn_week[i], LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_border_width(vw.btn_week[i], 0, 0);
        lv_obj_set_style_shadow_width(vw.btn_week[i], 0, 0);
        lv_obj_add_flag(vw.btn_week[i], LV_OBJ_FLAG_CHECKABLE); // 设置为可切换

        // 默认灰色，选中蓝色
        lv_obj_set_style_bg_color(vw.btn_week[i], lv_color_hex(0x333333), 0); // 默认灰色
        lv_obj_set_style_bg_opa(vw.btn_week[i], LV_OPA_COVER, 0);
        lv_obj_set_style_bg_color(vw.btn_week[i], lv_palette_main(LV_PALETTE_BLUE), LV_STATE_CHECKED); // 选中蓝色
        lv_obj_set_style_bg_opa(vw.btn_week[i], LV_OPA_COVER, LV_STATE_CHECKED);

        lv_obj_t *label = lv_label_create(vw.btn_week[i]);
        lv_obj_set_style_text_font(label, cl_ui_get_font(), 0);
        lv_obj_set_style_text_color(label, lv_color_white(), 0);
        lv_label_set_text(label, week_text[i]);
        lv_obj_center(label);
    }
    // 下排三个按钮
    for(int i = 0; i < 3; ++i) {
        vw.btn_week[i+4] = lv_btn_create(btnmat_cont);
        lv_obj_set_size(vw.btn_week[i+4], 75, 75);
        lv_obj_align(vw.btn_week[i+4], LV_ALIGN_TOP_MID, x1[i], y1);
        lv_obj_set_style_radius(vw.btn_week[i+4], LV_RADIUS_CIRCLE, 0);
        lv_obj_set_style_border_width(vw.btn_week[i+4], 0, 0);
        lv_obj_set_style_shadow_width(vw.btn_week[i+4], 0, 0);
        lv_obj_add_flag(vw.btn_week[i+4], LV_OBJ_FLAG_CHECKABLE); // 设置为可切换

        // 默认灰色，选中蓝色
        lv_obj_set_style_bg_color(vw.btn_week[i+4], lv_color_hex(0x333333), 0); // 默认灰色
        lv_obj_set_style_bg_opa(vw.btn_week[i+4], LV_OPA_COVER, 0);
        lv_obj_set_style_bg_color(vw.btn_week[i+4], lv_palette_main(LV_PALETTE_BLUE), LV_STATE_CHECKED); // 选中蓝色
        lv_obj_set_style_bg_opa(vw.btn_week[i+4], LV_OPA_COVER, LV_STATE_CHECKED);

        lv_obj_t *label = lv_label_create(vw.btn_week[i+4]);
        lv_obj_set_style_text_font(label, cl_ui_get_font(), 0);
        lv_obj_set_style_text_color(label, lv_color_white(), 0);
        lv_label_set_text(label, week_text[i+4]);
        lv_obj_center(label);
    }
       
    // 底部右侧默认颜色按钮
    lv_obj_t *btn_save = lv_btn_create(cont);
    lv_obj_set_size(btn_save, 180, 55);
    lv_obj_set_style_bg_color(btn_save, lv_color_hex(0x333333), 0);
    lv_obj_set_style_radius(btn_save, LV_RADIUS_CIRCLE, 0); // 圆形
    lv_obj_set_style_bg_opa(btn_save, LV_OPA_COVER, 0);
    lv_obj_set_style_shadow_width(btn_save, 8, 0);

    lv_obj_t *label_save = lv_label_create(btn_save);
    lv_obj_set_style_text_font(label_save, cl_ui_get_font(), 0);
    lv_obj_set_style_text_color(label_save, lv_color_white(), 0);
    lv_label_set_text(label_save, "保存");
    lv_obj_center(label_save);

    lv_obj_add_event_cb(btn_save, _btn_loop_save, LV_EVENT_SHORT_CLICKED, cont);

    // 排布底部两个按钮，左边靠左，右边靠右，底部对齐
    lv_obj_align(btn_save, LV_ALIGN_BOTTOM_MID, 0, -38);
    lv_obj_add_event_cb(cont, _back_del_cont, LV_EVENT_GESTURE, cont); // 增加short click事件
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_GESTURE_BUBBLE);
    
    lv_obj_add_flag(cont, LV_OBJ_FLAG_HIDDEN);
    return cont;
}

// 时间选择层
static lv_obj_t *_create_edit_cont(lv_obj_t *parent){
    // 全屏黑色背景，无圆角无边框，roller无边框
    lv_obj_t *cont = lv_obj_create(parent);
    lv_obj_set_size(cont, LV_HOR_RES, LV_VER_RES);  // 全屏
    lv_obj_set_style_pad_all(cont, 0, 0);
    lv_obj_set_style_bg_color(cont, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(cont, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(cont, 0, 0);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);

    // 增加标题 "设置时间" (Set Time)
    lv_obj_t *title_label = lv_label_create(cont);
    lv_obj_set_style_text_font(title_label, &font_noto_28_4, 0);
    lv_obj_set_style_text_color(title_label, lv_color_hex(0xFFFFFF), 0);
    lv_label_set_text(title_label, "设置时间");
    lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, 40);

    lv_obj_t *time_cont = lv_obj_create(cont);
    lv_obj_set_size(time_cont, 300, 200);
    lv_obj_set_style_pad_all(time_cont, 0, 0);
    lv_obj_set_style_bg_opa(time_cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(time_cont, 0, 0);
    lv_obj_clear_flag(time_cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_align(time_cont, LV_ALIGN_CENTER, 0, -10);

    // 创建冒号
    lv_obj_t *label_colon = lv_label_create(time_cont);
    lv_obj_set_style_text_font(label_colon, &font_noto_num_32_4, 0);
    lv_obj_set_style_text_color(label_colon, lv_color_hex(0xFFFFFF), 0);
    lv_label_set_text(label_colon, ":");
    lv_obj_center(label_colon);

    // 创建小时roller
    lv_obj_t *roller_hour = lv_roller_create(time_cont);
    vw.roller_hour = roller_hour;
    lv_obj_set_size(roller_hour, 70, 200);
    lv_obj_set_style_bg_opa(roller_hour, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(roller_hour, 0, 0);
    lv_obj_clear_flag(roller_hour, LV_OBJ_FLAG_SCROLLABLE);
    lv_roller_set_options(roller_hour, HOUR_OPTIONS, LV_ROLLER_MODE_INFINITE);
    lv_roller_set_visible_row_count(roller_hour, 8);
    // 设置字体和颜色
    lv_obj_set_style_text_font(roller_hour, &font_noto_num_32_4, 0);
    lv_obj_set_style_text_font(roller_hour, &font_noto_num_48_4, LV_PART_SELECTED);
    lv_obj_set_style_text_color(roller_hour, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_color(roller_hour, lv_color_hex(0xFFFFFF), LV_PART_SELECTED);
    // 设置选中区域背景为黑色
    lv_obj_set_style_bg_color(roller_hour, lv_color_hex(0x000000), LV_PART_SELECTED);
    lv_obj_set_style_bg_opa(roller_hour, LV_OPA_COVER, LV_PART_SELECTED);
    // 设置选项之间的行间距（上下间距）
    lv_obj_set_style_text_line_space(roller_hour, 32, LV_PART_MAIN);
    lv_obj_align_to(roller_hour, label_colon, LV_ALIGN_OUT_LEFT_MID, -15, 0);

    // 创建分钟roller，对齐到冒号右边
    lv_obj_t *roller_min = lv_roller_create(time_cont);
    vw.roller_min = roller_min;
    lv_obj_set_size(roller_min, 70, 200);
    lv_obj_set_style_bg_opa(roller_min, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(roller_min, 0, 0);
    lv_obj_clear_flag(roller_min, LV_OBJ_FLAG_SCROLLABLE);
    lv_roller_set_options(roller_min, MINUTE_OPTIONS, LV_ROLLER_MODE_INFINITE);
    lv_roller_set_visible_row_count(roller_min, 8);
    // 设置字体和颜色
    lv_obj_set_style_text_font(roller_min, &font_noto_num_32_4, 0);
    lv_obj_set_style_text_font(roller_min, &font_noto_num_48_4, LV_PART_SELECTED);
    lv_obj_set_style_text_color(roller_min, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_color(roller_min, lv_color_hex(0xFFFFFF), LV_PART_SELECTED);
    // 设置选中区域背景为黑色
    lv_obj_set_style_bg_color(roller_min, lv_color_hex(0x000000), LV_PART_SELECTED);
    lv_obj_set_style_bg_opa(roller_min, LV_OPA_COVER, LV_PART_SELECTED);
    // 设置选项之间的行间距（上下间距）
    lv_obj_set_style_text_line_space(roller_min, 32, LV_PART_MAIN);
    lv_obj_align_to(roller_min, label_colon, LV_ALIGN_OUT_RIGHT_MID, 15, 0);


    create_fade_overlay(time_cont);

    // 底部左侧灰色按钮
    lv_obj_t *btn_left = lv_btn_create(cont);
    lv_obj_set_size(btn_left, 65, 65);
    lv_obj_set_style_radius(btn_left, LV_RADIUS_CIRCLE, 0); // 圆形
    lv_obj_set_style_bg_color(btn_left, lv_color_hex(0x444444), 0); // 灰色
    lv_obj_set_style_bg_opa(btn_left, LV_OPA_COVER, 0);
    lv_obj_set_style_shadow_width(btn_left, 8, 0);
    lv_obj_set_style_shadow_color(btn_left, lv_color_hex(0x222222), 0);
    lv_obj_set_style_border_width(btn_left, 0, 0);
    lv_obj_set_style_pad_all(btn_left, 0, 0);
    
    lv_obj_t *icon_left = lv_img_create(btn_left);
    lv_img_set_src(icon_left, &icon_cancel_28);
    lv_obj_center(icon_left);
    
    // 底部右侧默认颜色按钮
    lv_obj_t *btn_right = lv_btn_create(cont);
    lv_obj_set_size(btn_right, 65, 65);
    lv_obj_set_style_radius(btn_right, LV_RADIUS_CIRCLE, 0); // 圆形
    lv_obj_set_style_bg_color(btn_right, lv_palette_main(LV_PALETTE_BLUE), 0); // 默认主题色
    lv_obj_set_style_bg_opa(btn_right, LV_OPA_COVER, 0);
    lv_obj_set_style_shadow_width(btn_right, 8, 0);
    lv_obj_set_style_shadow_color(btn_right, lv_palette_darken(LV_PALETTE_BLUE, 2), 0);
    lv_obj_set_style_border_width(btn_right, 0, 0);
    lv_obj_set_style_pad_all(btn_right, 0, 0);
    
    lv_obj_t *icon_right = lv_img_create(btn_right);
    lv_img_set_src(icon_right, &icon_accept_28);
    lv_obj_center(icon_right);
    
    // 排布底部两个按钮，左边靠左，右边靠右，底部对齐
    lv_obj_align(btn_left, LV_ALIGN_BOTTOM_MID, -48, -38);
    lv_obj_align(btn_right, LV_ALIGN_BOTTOM_MID, 48, -38);
    
    lv_obj_add_event_cb(btn_right, _btn_time_accept, LV_EVENT_SHORT_CLICKED, cont);
    lv_obj_add_event_cb(btn_left, _back_del_cont, LV_EVENT_SHORT_CLICKED, cont); // 增加short click事件
    lv_obj_add_event_cb(cont, _back_del_cont, LV_EVENT_GESTURE, cont); // 增加short click事件
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_GESTURE_BUBBLE);

    lv_obj_add_flag(cont, LV_OBJ_FLAG_HIDDEN);
    return cont;
}

static void show_time_edit_cont(){
    lv_roller_set_selected(vw.roller_hour, vw.alarm_edit.hour, LV_ANIM_OFF);
    lv_roller_set_selected(vw.roller_min, vw.alarm_edit.min, LV_ANIM_OFF);
    lv_obj_clear_flag(vw.cont_time_edit, LV_OBJ_FLAG_HIDDEN);
}

static void show_loop_edit_cont(){
    uint8_t mode = vw.alarm_edit.mode;
    for (int i = 0; i < 7; ++i) {
        if((mode == ALARM_MODE_EVERY_DAY) || (mode & btn_mode_bit[i])) {
            lv_obj_add_state(vw.btn_week[i], LV_STATE_CHECKED);
        } else {
            lv_obj_clear_state(vw.btn_week[i], LV_STATE_CHECKED);
        }
    }
    lv_obj_clear_flag(vw.cont_loop_edit, LV_OBJ_FLAG_HIDDEN);
}

static void show_info_cont(){
    lv_label_set_text_fmt(vw.label_time_set, "%02d:%02d", vw.alarm_edit.hour, vw.alarm_edit.min);
    char repeat_text[32] = {0};
    uint8_t mode = vw.alarm_edit.mode;
    if(mode & ALARM_MODE_EVERY_DAY) {
        strcpy(repeat_text, "每天");
    } else if(mode == ALARM_MODE_ONCE) {
        strcpy(repeat_text, "仅一次");
    } else {
        int first = 1;
        static const char* week_list[7] = {"日","一","二","三","四","五","六"};
        for(int i=0; i<7; ++i) {
            if(mode & btn_mode_bit[i]) {
                if(!first) strcat(repeat_text, "、");
                strcat(repeat_text, week_list[i]);
                first = 0;
            }
        }
    }
    lv_label_set_text(vw.label_loop, repeat_text);
    lv_obj_clear_flag(vw.cont_info, LV_OBJ_FLAG_HIDDEN);
}

lv_obj_t *_create_info_cont(lv_obj_t *parent){
    // 全屏黑色背景，无圆角无边框，roller无边框，flex纵向布局
    lv_obj_t *cont = lv_obj_create(parent);
    lv_obj_center(cont);
    lv_obj_set_size(cont, LV_HOR_RES, LV_VER_RES);  // 全屏
    lv_obj_set_style_pad_all(cont, 0, 0);
    lv_obj_set_style_pad_top(cont, 40, 0);
    lv_obj_set_style_bg_color(cont, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(cont, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(cont, 0, 0);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);


    // 增加标题 "设置时间" (Set Time)，居中在上面
    lv_obj_t *title_label = lv_label_create(cont);
    lv_obj_set_style_text_font(title_label, &font_noto_28_4, 0);
    lv_obj_set_style_text_color(title_label, lv_color_hex(0xFFFFFF), 0);
    lv_label_set_text(title_label, "编辑闹钟");
    lv_obj_align(title_label, LV_ALIGN_TOP_MID, 0, 0);

    // 新建时间按钮，上面“时间”，下面具体时间，下方对齐且居中
    lv_obj_t *menu_time = lv_btn_create(cont);
    lv_obj_set_size(menu_time, 260, 70);
    lv_obj_set_style_radius(menu_time, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_shadow_width(menu_time, 0, 0);
    lv_obj_set_style_pad_left(menu_time, 30, 0);
    lv_obj_set_style_bg_color(menu_time, lv_color_hex(0x333333), 0);
    lv_obj_align(menu_time, LV_ALIGN_TOP_MID, 0, 50); // 紧跟标题下方居中

    // label: 时间
    lv_obj_t *label_menu_time_title = lv_label_create(menu_time);
    lv_label_set_text(label_menu_time_title, "时间");
    lv_obj_set_style_text_font(label_menu_time_title, cl_ui_get_font(), 0);
    lv_obj_set_style_text_color(label_menu_time_title, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(label_menu_time_title, LV_ALIGN_TOP_LEFT, 0, 0);

    // label: 具体时间
    lv_obj_t *label_menu_time_value = lv_label_create(menu_time);
    vw.label_time_set = label_menu_time_value;
    lv_label_set_text_fmt(label_menu_time_value, "%02d:%02d", vw.alarm_edit.hour, vw.alarm_edit.min);
    lv_obj_set_style_text_font(label_menu_time_value, cl_ui_get_font(), 0);
    lv_obj_set_style_text_color(label_menu_time_value, lv_color_hex(0x888888), 0);
    lv_obj_align(label_menu_time_value, LV_ALIGN_BOTTOM_LEFT, 0, 0);

    // 新建重复按钮，上面“重复”，下面具体重复信息，也居中排布，跟在 menu_time 下方
    lv_obj_t *menu_loop = lv_btn_create(cont);
    lv_obj_set_size(menu_loop, 260, 70);
    lv_obj_set_style_radius(menu_loop, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_shadow_width(menu_loop, 0, 0);
    lv_obj_set_style_pad_left(menu_loop, 30, 0);
    lv_obj_set_style_bg_color(menu_loop, lv_color_hex(0x333333), 0);
    lv_obj_align_to(menu_loop, menu_time, LV_ALIGN_OUT_BOTTOM_MID, 0, 14); // 紧跟menu_time居中排布

    // label: 重复
    lv_obj_t *label_menu_loop_title = lv_label_create(menu_loop);
    lv_label_set_text(label_menu_loop_title, "重复");
    lv_obj_set_style_text_font(label_menu_loop_title, cl_ui_get_font(), 0);
    lv_obj_set_style_text_color(label_menu_loop_title, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(label_menu_loop_title, LV_ALIGN_TOP_LEFT, 0, 0);

    // label: 具体重复信息
    lv_obj_t *label_menu_loop_value = lv_label_create(menu_loop);
    // 构造具体重复信息字符串
    vw.label_loop = label_menu_loop_value;
    lv_label_set_text(label_menu_loop_value, "");
    lv_obj_set_style_text_font(label_menu_loop_value, cl_ui_get_font(), 0);
    lv_obj_set_style_text_color(label_menu_loop_value, lv_color_hex(0x888888), 0);
    lv_obj_align(label_menu_loop_value, LV_ALIGN_BOTTOM_LEFT, 0, 0);

    lv_obj_t *del_btn = lv_btn_create(cont);
    lv_obj_set_size(del_btn, 65, 65);
    lv_obj_set_style_radius(del_btn, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(del_btn, lv_color_hex(0x333333), 0);
    // lv_obj_align(del_btn, LV_ALIGN_BOTTOM_MID, 0, -28);
    lv_obj_set_style_shadow_width(del_btn, 0, 0);
    // 按钮里新建图片用icon_delete_32
    lv_obj_t *img_icon = lv_img_create(del_btn);
    lv_img_set_src(img_icon, &icon_delete_28);
    lv_obj_center(img_icon);

    lv_obj_t *btn_accept = lv_btn_create(cont);
    lv_obj_set_size(btn_accept, 65, 65);
    lv_obj_set_style_radius(btn_accept, LV_RADIUS_CIRCLE, 0);
    // lv_obj_set_style_bg_color(btn_accept, lv_color_hex(0x333333), 0);
    // lv_obj_align(del_btn, LV_ALIGN_BOTTOM_MID, 0, -28);
    lv_obj_set_style_shadow_width(btn_accept, 0, 0);
    // 按钮里新建图片用icon_delete_32
    lv_obj_t *icon_accept = lv_img_create(btn_accept);
    lv_img_set_src(icon_accept, &icon_accept_28);
    lv_obj_center(icon_accept);
    

    lv_obj_align(del_btn, LV_ALIGN_BOTTOM_MID, -48, -28);
    lv_obj_align(btn_accept, LV_ALIGN_BOTTOM_MID, 48, -28);

    lv_obj_add_event_cb(del_btn, _on_del_btn, LV_EVENT_SHORT_CLICKED, NULL);
    lv_obj_add_event_cb(menu_time, _on_item_time, LV_EVENT_SHORT_CLICKED, NULL);
    lv_obj_add_event_cb(menu_loop, _on_item_loop, LV_EVENT_SHORT_CLICKED, NULL);

    
    lv_obj_add_event_cb(cont, _back_del_cont, LV_EVENT_GESTURE, cont); // 增加short click事件
    lv_obj_add_event_cb(btn_accept, _back_del_cont, LV_EVENT_SHORT_CLICKED, cont); // 增加short click事件
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_GESTURE_BUBBLE);
    lv_obj_add_flag(cont, LV_OBJ_FLAG_HIDDEN);
    return cont;
}

static void _on_add_btn(lv_event_t *e){
    lv_obj_t *root = lv_event_get_user_data(e);

    int32_t num = lv_obj_get_child_cnt(vw.cont_list);
    if (num >= 7)
    {
        lv_toast_show("已达最大闹钟数", 1000);
        return;
    }
    

    vw.mode = ALARM_MODE_ADD;
    if (root)
    {
        // 获取当前的系统时间时和分
        time_t now = time(NULL);
        struct tm *tm_info = localtime(&now);
        int hour = tm_info->tm_hour;
        int min = tm_info->tm_min;
        vw.alarm_edit.hour = hour;
        vw.alarm_edit.min = min;
        vw.alarm_edit.mode = ALARM_MODE_ONCE;
        vw.alarm_edit.sec = 0;
        vw.alarm_edit.sw = 1;
        show_time_edit_cont();
    }    
}

static void _root_ges_cb(lv_event_t *e){
    lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_active());
    if (dir == LV_DIR_RIGHT)
    {
        lv_indev_wait_release(lv_indev_active());
        page_change("home");
    }
}

static void _on_alarm_item_click(lv_event_t *e){
    alarm_transfer_t *alarm = lv_event_get_user_data(e);
    if (!alarm)
    {
        return;
    }
    memcpy(&vw.alarm_edit, alarm, sizeof(alarm_transfer_t));
    show_info_cont();
}

static void _on_alarm_sw_click(lv_event_t *e){
    alarm_transfer_t *alarm = lv_event_get_user_data(e);
    lv_obj_t *sw = lv_event_get_target(e);
    if (!alarm || !sw)
    {
        return;
    }

    memcpy(&vw.alarm_edit, alarm, sizeof(alarm_transfer_t));
    vw.alarm_edit.sw = lv_obj_has_state(sw, LV_STATE_CHECKED)?1:0;

#ifndef SIMULATOR
    vb_alarm_set(&vw.alarm_edit);
#endif

}

static lv_obj_t *_create_alarm_item(lv_obj_t *parent, alarm_transfer_t *alarm){
    // 创建容器，布局水平排列，宽度和父容器一样
    lv_obj_t *cont = lv_obj_create(parent);
    lv_obj_set_size(cont, 260, 80);
    lv_obj_set_style_radius(cont, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_pad_all(cont, 0, 0);
    lv_obj_set_style_pad_left(cont, 20, 0);
    lv_obj_set_style_pad_top(cont, 15, 0);
    lv_obj_set_style_pad_bottom(cont, 15, 0);
    lv_obj_set_style_pad_right(cont, 20, 0);
    lv_obj_set_style_border_width(cont, 0, 0);
    lv_obj_set_style_bg_color(cont, lv_color_hex(0x333333), 0);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);

    // 时间label - 左边
    lv_obj_t *time_label = lv_label_create(cont);
    lv_label_set_text_fmt(time_label, "%02d:%02d", alarm->hour, alarm->min);
    lv_obj_set_style_text_font(time_label, &font_noto_num_32_4, 0);
    lv_obj_set_style_text_color(time_label, lv_color_hex(0xffffff), 0);
    lv_obj_align(time_label, LV_ALIGN_TOP_LEFT, 0, 0);

    char repeat_text[32] = {0};
    uint8_t mode = alarm->mode;
    if(mode & ALARM_MODE_EVERY_DAY) {
        strcpy(repeat_text, "每天");
    } else if(mode == ALARM_MODE_ONCE) {
        strcpy(repeat_text, "仅一次");
    } else {
        int first = 1;
        static const char* week_list[7] = {"日","一","二","三","四","五","六"};
        for(int i=0; i<7; ++i) {
            if(mode & btn_mode_bit[i]) {
                if(!first) strcat(repeat_text, "、");
                strcat(repeat_text, week_list[i]);
                first = 0;
            }
        }
    }
    
    lv_obj_t *week_label = lv_label_create(cont);
    // lv_obj_set_size(alarm->week_label, 200, 30);
    lv_label_set_text(week_label, repeat_text);
    lv_obj_set_style_text_color(week_label, lv_color_hex(0x808080), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_text_font(week_label, cl_ui_get_font(), 0);
    lv_obj_align(week_label, LV_ALIGN_BOTTOM_LEFT, 0, 4);


    // Switch - 右边
    lv_obj_t *sw = lv_switch_create(cont);
    lv_obj_set_size(sw, 50, 25);
    lv_obj_align(sw, LV_ALIGN_RIGHT_MID, 0, 0);

    if (alarm->sw)
    {
        lv_obj_add_state(sw, LV_STATE_CHECKED);
    }

    lv_obj_add_flag(cont, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(cont, _on_alarm_item_click, LV_EVENT_CLICKED, alarm);
    lv_obj_add_event_cb(sw, _on_alarm_sw_click, LV_EVENT_CLICKED, alarm);
    return cont;
}

void refresh_alarm(){
    // 删除vw.cont_list第二个之后所有子控件
    if (vw.cont_list) {
        uint32_t child_count = lv_obj_get_child_cnt(vw.cont_list);
        for (uint32_t i = child_count; i > 2; i--) {
            lv_obj_del(lv_obj_get_child(vw.cont_list, - 1));
        }
    }
#ifdef SIMULATOR
    static alarm_transfer_t alarm = {
        .sw = 1,
        .index = 0xff,
        .hour = 17,
        .min = 47,
        .mode = 0
    };
    _create_alarm_item(vw.cont_list, &alarm);
#else
    alarm_transfer_t *alarm = NULL;
    int alarm_num = vb_alarm_get_info(&alarm);

    for (int i = 0; i < alarm_num; i++)
    {
        _create_alarm_item(vw.cont_list, &alarm[i]);
    }
#endif
}

alarm_view_t* alarm_view_create(lv_obj_t *root)
{
    lv_obj_remove_style_all(root);
    lv_obj_set_size(root, LV_HOR_RES, LV_VER_RES);
    lv_obj_add_event_cb(root, _on_btn_cb, CL_UI_EVENT_BUTTON, NULL);
    lv_obj_set_style_bg_color(root, lv_color_black(), 0); // 设置根背景为黑色
    lv_obj_set_style_bg_opa(root, LV_OPA_COVER, 0);

    // 创建一个主容器，使用flex垂直布局
    lv_obj_t *cont = lv_obj_create(root);
    vw.cont_list = cont;
    lv_obj_set_size(cont, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_top(cont, 24, 0);
    lv_obj_set_style_bg_opa(cont, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(cont, 0, 0);

    // 时间Label（显示时间，放最上面），保存到vw.label_time
    vw.label_time = lv_label_create(cont);
    lv_obj_set_style_text_font(vw.label_time, cl_ui_get_font(), 0); // 可按需更换字体
    lv_obj_set_style_text_color(vw.label_time, lv_color_white(), 0); // 文字白色
    lv_label_set_text(vw.label_time, "00:00");

    // Title Label（显示“闹钟”）
    lv_obj_t *label_title = lv_label_create(cont);
    lv_label_set_text(label_title, "闹钟");
    lv_obj_set_style_text_font(label_title, cl_ui_get_font(), 0);
    lv_obj_set_style_text_color(label_title, lv_color_white(), 0); // 文字白色

    // 创建并启动定时器
    vw.timer = lv_timer_create(_update_time_cb, 1000, NULL);
    _update_time_cb(NULL); // 启动时主动刷新一次
    // 下面将来放闹钟列表

    // 添加按钮：圆形按钮，内有加号
    lv_obj_t *btn_add = lv_btn_create(root);
    lv_obj_set_size(btn_add, 56, 56);
    lv_obj_set_style_radius(btn_add, LV_RADIUS_CIRCLE, 0); // 圆形
    lv_obj_set_style_bg_color(btn_add, lv_palette_main(LV_PALETTE_BLUE), 0);
    lv_obj_set_style_bg_opa(btn_add, LV_OPA_COVER, 0);
    lv_obj_set_style_shadow_width(btn_add, 12, 0);
    lv_obj_set_style_shadow_color(btn_add, lv_palette_darken(LV_PALETTE_BLUE, 2), 0);
    lv_obj_set_style_border_width(btn_add, 0, 0);
    lv_obj_set_style_pad_all(btn_add, 0, 0);
    lv_obj_set_flex_grow(btn_add, 0);

    LV_IMG_DECLARE(add)
    lv_obj_t *icon_add = lv_img_create(btn_add);
    lv_img_set_src(icon_add, &add);
    lv_obj_center(icon_add);

    // 放在底部中间（可根据需要调整位置）
    lv_obj_set_style_align(btn_add, LV_ALIGN_BOTTOM_MID, 0);
    lv_obj_align(btn_add, LV_ALIGN_BOTTOM_MID, 0, -28);
    lv_obj_add_event_cb(btn_add, _on_add_btn, LV_EVENT_SHORT_CLICKED, root);
    lv_obj_add_event_cb(cont, _root_ges_cb, LV_EVENT_GESTURE, NULL);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_GESTURE_BUBBLE);

    vw.cont_info = _create_info_cont(root);
    vw.cont_time_edit = _create_edit_cont(root);
    vw.cont_loop_edit = _create_loop_cont(root);
    refresh_alarm();
    return &vw;
}

void alarm_view_delete(void)
{
    if (vw.timer)
    {
        lv_timer_del(vw.timer);
        vw.timer = NULL;
    }
    
}
