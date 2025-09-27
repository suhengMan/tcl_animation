#include "setting_genera_view.h"
#include "lvgl.h"
#include "cl_ui.h"

static setting_genera_view_t vw;
LV_IMG_DECLARE(icon_setting_theme)
LV_IMG_DECLARE(icon_setting_phone)
LV_IMG_DECLARE(icon_setting_reset)
LV_IMG_DECLARE(icon_setting_power_off)
LV_IMG_DECLARE(icon_back_14)
LV_IMG_DECLARE(icon_warning_18)



static lv_obj_t *creat_setting_genera_item(lv_obj_t * parent, void *icon_src, const char *title,  void (*on_click)(lv_event_t* e))
{
    // 创建容器，布局水平排列，宽度和父容器一样
    lv_obj_t *cont = lv_button_create(parent);
    lv_obj_set_size(cont, 220, 40);
    lv_obj_set_style_shadow_width(cont, 0, 0);
    lv_obj_set_style_border_width(cont, 0, 0);
    lv_obj_set_style_bg_color(cont, lv_color_black(), 0);
    lv_obj_set_style_radius(cont, 5, 0);

    lv_obj_t *icon = lv_img_create(cont);
    lv_img_set_src(icon, icon_src);
    lv_obj_align(icon, LV_ALIGN_LEFT_MID, 10, 0);

    lv_obj_t *label = lv_label_create(cont);
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_label_set_text(label, title);
    lv_obj_align_to(label, icon, LV_ALIGN_OUT_RIGHT_MID, 15, 0);

 
    if (on_click != NULL)
    {
        lv_obj_add_event_cb(cont, on_click, LV_EVENT_CLICKED, cont);
    }
    return cont;
}


static void _on_connect_menu(lv_event_t *e){
    // page_change(UI_PG_CONNECT);
}

static void _on_back_click(lv_event_t *e){
    page_change("setting");
}

// 实验室
static void _on_laboratory_click(lv_event_t *e){
    page_change("laboratory");
}

static void _on_theme_menu(lv_event_t *e){
    page_change("setting_theme");
}


static void _on_dialog_cancel(lv_event_t *e){
    lv_obj_t *dialog = lv_event_get_user_data(e);
    lv_obj_del(dialog);
}

static void _on_dialog_confirm(lv_event_t *e){
    lv_obj_t *dialog = lv_event_get_user_data(e);
    lv_obj_del(dialog);
    xz_recover_fact();
}

static void _on_recover_menu(lv_event_t *e){

    lv_obj_t *container = lv_obj_create(vw.root);

    lv_obj_remove_style_all(container);
    lv_obj_set_size(container, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_bg_color(container, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(container, LV_OPA_50, 0);
    lv_obj_set_style_border_width(container, 0, 0);
    lv_obj_set_style_pad_all(container, 0, 0);
    lv_obj_remove_flag(container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_center(container);

    lv_obj_t *dialog = lv_obj_create(container);
    lv_obj_set_size(dialog, 220, 100);
    lv_obj_align(dialog, LV_ALIGN_BOTTOM_MID, 0, -20);
    lv_obj_set_style_radius(dialog, 20,0);
    lv_obj_set_style_bg_color(dialog, lv_color_hex(0x333333), 0);
    lv_obj_set_style_bg_opa(dialog, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(dialog, 0, 0);
    lv_obj_set_style_pad_all(dialog, 5, 0);

    lv_obj_t *title = lv_label_create(dialog);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 10);
    lv_obj_set_style_text_color(title, lv_color_white(), 0);
    lv_label_set_text(title, LT_RECOVER"?");

    lv_obj_t *btn_cancel = lv_btn_create(dialog);
    lv_obj_set_size(btn_cancel, 80, 30);
    lv_obj_align(btn_cancel, LV_ALIGN_BOTTOM_MID, -50, -5);
    lv_obj_set_style_bg_color(btn_cancel, lv_color_hex(0x535353), 0);
    lv_obj_set_style_border_width(btn_cancel, 0, 0);
    lv_obj_set_style_radius(btn_cancel, 10, 0);
    lv_obj_set_style_shadow_width(btn_cancel, 0, 0);
    lv_obj_set_style_shadow_color(btn_cancel, lv_color_hex(0x000000), 0);
    lv_obj_set_style_shadow_opa(btn_cancel, 0, 0);
    lv_obj_t *btn_cancel_label = lv_label_create(btn_cancel);
    lv_obj_set_style_text_color(btn_cancel_label, lv_color_white(), 0);
    lv_label_set_text(btn_cancel_label, LT_CANCEL);
    lv_obj_center(btn_cancel_label);
    
    // 确定按钮
    lv_obj_t *btn_confirm = lv_btn_create(dialog);
    lv_obj_set_size(btn_confirm, 80, 30);
    // 取消按钮在左，确定按钮在右
    lv_obj_align(btn_confirm, LV_ALIGN_BOTTOM_MID, 50, -5);
    lv_obj_set_style_bg_color(btn_confirm, lv_color_hex(0xff5353), 0); // 红色
    lv_obj_set_style_border_width(btn_confirm, 0, 0);
    lv_obj_set_style_radius(btn_confirm, 10, 0);
    lv_obj_set_style_shadow_width(btn_confirm, 0, 0);
    lv_obj_set_style_shadow_color(btn_confirm, lv_color_hex(0x000000), 0);
    lv_obj_set_style_shadow_opa(btn_confirm, 0, 0);
    lv_obj_t *btn_confirm_label = lv_label_create(btn_confirm);
    lv_obj_set_style_text_color(btn_confirm_label, lv_color_white(), 0);
    lv_label_set_text(btn_confirm_label, LT_ACCEPT);
    lv_obj_center(btn_confirm_label);

    lv_obj_add_event_cb(btn_cancel, _on_dialog_cancel, LV_EVENT_CLICKED, container);
    lv_obj_add_event_cb(btn_confirm, _on_dialog_confirm, LV_EVENT_CLICKED, container);
}



setting_genera_view_t* setting_genera_view_create(lv_obj_t *root)
{
    lv_obj_remove_style_all(root);
    lv_obj_set_size(root, LV_HOR_RES, LV_VER_RES);
    vw.root = root;
    // 容器
    lv_obj_t *container = lv_obj_create(root);


    lv_obj_remove_style_all(container);
    lv_obj_set_size(container, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_bg_opa(container, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(container, 0, 0);
    lv_obj_set_style_pad_all(container, 0, 0);
    lv_obj_remove_flag(container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_center(container);
    lv_obj_set_style_bg_color(container, lv_color_hex(0x000000), 0);

    vw.status_bar = lv_obj_create(container);
    lv_obj_set_size(vw.status_bar, LV_HOR_RES, 40);
    lv_obj_set_style_pad_all(vw.status_bar, 0, 0);
    lv_obj_set_style_border_width(vw.status_bar, 0, 0);
    lv_obj_set_style_bg_color(vw.status_bar, lv_color_hex(0x000000), 0);
    lv_obj_set_style_radius(vw.status_bar, 0, 0);
    lv_obj_set_scrollbar_mode(vw.status_bar, LV_SCROLLBAR_MODE_OFF);

    lv_obj_t *btn_back = lv_button_create(vw.status_bar);
    lv_obj_align(btn_back, LV_ALIGN_LEFT_MID, 20, 5);
    lv_obj_set_style_shadow_width(btn_back, 0, NULL);
    lv_obj_set_style_bg_color(btn_back, lv_color_black(),0);
    lv_obj_clear_flag(btn_back, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(btn_back, _on_back_click, LV_EVENT_CLICKED, NULL);
    lv_obj_t *btn_img =  lv_img_create(btn_back);
    lv_img_set_src(btn_img, &icon_back_14);
    lv_obj_center(btn_img);

    lv_obj_t *title_label = lv_label_create(vw.status_bar);
    lv_label_set_text(title_label, LT_SYSTEM);
    lv_obj_set_style_text_color(title_label, lv_color_hex(0xe9feff), 0);
    lv_obj_align_to(title_label, btn_back, LV_ALIGN_OUT_RIGHT_MID, 5, 0);

    // 主内容区
    lv_obj_t *cont = lv_obj_create(container);
    lv_obj_align_to(cont, vw.status_bar, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
    lv_obj_set_size(cont, LV_HOR_RES, LV_VER_RES-30);
    lv_obj_set_style_border_width(cont, 0, 0);
    lv_obj_set_style_bg_color(cont, lv_color_hex(0x000000), 0);
    lv_obj_set_style_radius(cont, 0, 0);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(cont, 5, 0);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    creat_setting_genera_item(cont, &icon_setting_theme, LT_AI_THEME, _on_theme_menu);
    creat_setting_genera_item(cont, &icon_setting_phone, LT_CONN_PHONE, _on_connect_menu);
    creat_setting_genera_item(cont, &icon_setting_reset, LT_RECOVER, _on_recover_menu);
    creat_setting_genera_item(cont, &icon_warning_18, LT_LAB, _on_laboratory_click);

    vw.is_act = 0;

    return &vw;
}

void setting_genera_view_delete(void) {

}