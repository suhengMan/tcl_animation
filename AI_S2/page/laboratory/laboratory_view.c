#include "laboratory_view.h"
#include "lvgl.h"
#include "cl_ui.h"

static laboratory_view_t vw;
LV_IMG_DECLARE(icon_back_14)

LV_FONT_DECLARE(font_puhui_18_4)

// Switch切换事件
static void switch_event_cb(lv_event_t * e)
{
    lv_obj_t * sw = lv_event_get_target(e);
    bool checked = lv_obj_has_state(sw, LV_STATE_CHECKED);
    xz_enable_global_wake(checked);
}

static void create_global_wakeup(lv_obj_t * parent)
{
    // 创建容器，布局水平排列，宽度和父容器一样
    lv_obj_t* cont = lv_obj_create(parent);
    lv_point_t size;
    lv_text_get_size(&size, LT_GLOBAL_WAKE_MSG, &font_puhui_18_4, 0, 0, 120, 0);
    lv_obj_set_size(cont, 200, 36+size.y);
    // lv_obj_set_size(cont, 200, 84);
    lv_obj_set_style_pad_all(cont, 5, 0);
    lv_obj_set_style_pad_left(cont, 2, 0);
    lv_obj_set_style_border_width(cont, 0, 0);
    lv_obj_set_style_bg_color(cont, lv_color_hex(0x1a1a1a), 0);
    lv_obj_set_style_radius(cont, 5, 0);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);

    

    lv_obj_t* fun_label = lv_label_create(cont);
    // lv_obj_set_size(laboratory->week_label, 200, 30);
    lv_label_set_text(fun_label, LT_GLOBAL_WAKE);
    lv_obj_set_style_text_color(fun_label, lv_color_hex(0xFFFFFF), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align(fun_label, LV_ALIGN_TOP_LEFT, 5, 6);

    lv_obj_t* des_label = lv_label_create(cont);
    // lv_obj_set_size(laboratory->week_label, 200, 30);
    lv_obj_set_width(des_label, 120);
    lv_label_set_text(des_label, LT_GLOBAL_WAKE_MSG);
    lv_label_set_long_mode(des_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_color(des_label, lv_color_hex(0x808080), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_align_to(des_label, fun_label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 4);

    // Switch - 右边
    lv_obj_t* sw = lv_switch_create(cont);
    lv_obj_set_size(sw, 50, 25);
    lv_obj_align(sw, LV_ALIGN_RIGHT_MID, -8, 0);
    lv_group_remove_obj(sw);
    lv_obj_set_style_bg_color(sw, lv_color_hex(0xFF5353), LV_PART_INDICATOR | LV_STATE_CHECKED);
    lv_obj_set_style_bg_color(sw, lv_color_hex(0x535353), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_add_event_cb(sw, switch_event_cb, LV_EVENT_VALUE_CHANGED, cont);

    if (xz_is_global_wake_enable())
    {
        lv_obj_add_state(sw, LV_STATE_CHECKED);
    }
}

static void _on_back_click(lv_event_t *e){
    page_change("setting_genera");
}

laboratory_view_t* laboratory_view_create(lv_obj_t *root)
{
    lv_obj_remove_style_all(root);
    lv_obj_set_size(root, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_size(root, LV_HOR_RES, LV_VER_RES);  // 设置容器大小为屏幕大小
    lv_obj_set_style_bg_opa(root, LV_OPA_COVER, 0);              // 设置背景不透明度为不透明
    lv_obj_set_style_border_width(root, 0, 0);                   // 清除边框
    lv_obj_set_style_pad_all(root, 0, 0);                         // 清除内边距
    lv_obj_remove_flag(root, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_center(root);    
    lv_obj_set_style_bg_color(root, lv_color_hex(0x000000), 0);  // 设置背景色为黑色
    
    // 状态栏
    lv_obj_t *status_bar_ = lv_obj_create(root);
    lv_obj_set_size(status_bar_, LV_HOR_RES, 40);
    lv_obj_set_style_pad_all(status_bar_, 0, 0);
    lv_obj_set_style_border_width(status_bar_, 0, 0);
    lv_obj_set_style_bg_color(status_bar_, lv_color_hex(0x000000), 0);    
    lv_obj_set_style_radius(status_bar_, 0, 0);
    lv_obj_set_scrollbar_mode(status_bar_, LV_SCROLLBAR_MODE_OFF);


    lv_obj_t *btn_back = lv_button_create(status_bar_);
    lv_obj_align(btn_back, LV_ALIGN_LEFT_MID, 20, 5);
    lv_obj_set_style_shadow_width(btn_back, 0, NULL);
    lv_obj_set_style_bg_color(btn_back, lv_color_black(),0);
    lv_obj_clear_flag(btn_back, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(btn_back, _on_back_click, LV_EVENT_CLICKED, NULL);
    lv_obj_t *btn_img =  lv_img_create(btn_back);
    lv_img_set_src(btn_img, &icon_back_14);
    lv_obj_center(btn_img);

    // 状态栏标题
    lv_obj_t* status_bar_label = lv_label_create(status_bar_);
    lv_label_set_long_mode(status_bar_label, LV_LABEL_LONG_SCROLL);
    lv_label_set_text(status_bar_label, LT_LAB_FUNC);
    lv_obj_set_style_text_align(status_bar_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_color(status_bar_label, lv_color_hex(0xe9feff), 0);
    lv_obj_align_to(status_bar_label, btn_back, LV_ALIGN_OUT_RIGHT_MID, 5, 0);
    
    vw.menu_cont = lv_obj_create(root);
    lv_obj_align_to(vw.menu_cont, status_bar_, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
    lv_obj_set_size(vw.menu_cont, LV_HOR_RES, LV_VER_RES-30);
    lv_obj_set_style_border_width(vw.menu_cont, 0, 0);  
    lv_obj_set_style_bg_color(vw.menu_cont, lv_color_hex(0x000000), 0);
    lv_obj_set_style_radius(vw.menu_cont, 0, 0);
    lv_obj_set_flex_flow(vw.menu_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(vw.menu_cont, 10,0);
    lv_obj_set_flex_align(vw.menu_cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    create_global_wakeup(vw.menu_cont);
    return &vw;
}

void laboratory_view_delete(void)
{
    
}

void laboratory_view_appear_anim_start(bool reverse)
{
    // 如果需要反向动画，可以在这里实现
}
