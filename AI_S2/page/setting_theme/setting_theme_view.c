#include "setting_theme_view.h"
#include "lvgl.h"
#include "cl_ui.h"

static setting_theme_view_t vw;
LV_FONT_DECLARE(font_puhui_18_4)
LV_IMG_DECLARE(icon_back_14)

static lv_style_t style_radio;
static lv_style_t style_radio_chk;
static u8 active_index = 0;


// 点击时回调
static void _on_item_click_cb(lv_event_t * e)
{
    lv_obj_t *parent = lv_event_get_user_data(e);
    lv_obj_t * cont = lv_event_get_current_target(e);
    lv_obj_t * curr_cont = lv_event_get_target_obj(e);
    
    lv_obj_t *ole_theme = vw.radios[vw.act_id];
    lv_obj_t *new_theme = curr_cont;
    vw.act_id = lv_obj_get_index(curr_cont);
    
    lv_obj_set_style_bg_color(ole_theme, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_color(new_theme, lv_color_hex(0xFF5353), 0);
    
    xz_set_ai_theme(vw.act_id);

}

// 主题单选项，左文字右checkbox
static lv_obj_t* create_theme_item(lv_obj_t *parent, const char *title, int idx)
{
    lv_obj_t *cont = lv_button_create(parent);
    lv_obj_set_size(cont, 220, 40);
    lv_obj_set_style_shadow_width(cont, 0, 0);
    lv_obj_set_style_border_width(cont, 0, 0);
    lv_obj_set_style_bg_color(cont, lv_color_black(), 0);
    lv_obj_set_style_radius(cont, 10, 0); // 圆角半径可根据需要调整

    lv_obj_t *label = lv_label_create(cont);
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_label_set_text(label, title);
    // 设置文本左对齐
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_LEFT, 0);
    // 设置label左对齐，垂直居中
    lv_obj_align(label, LV_ALIGN_LEFT_MID, 20, 0);
   
    if (vw.act_id == idx)
    {
        lv_obj_set_style_bg_color(cont, lv_color_hex(0xFF5353), 0);
    }
    
    
    lv_obj_add_event_cb(cont, _on_item_click_cb, LV_EVENT_CLICKED, parent);
    
    return cont;
}

static void _on_back_click(lv_event_t *e){
    page_change("setting_genera");
}

setting_theme_view_t* setting_theme_view_create(lv_obj_t *root)
{

    lv_style_init(&style_radio);
    lv_style_set_radius(&style_radio, LV_RADIUS_CIRCLE);
    lv_style_set_border_color(&style_radio, lv_color_hex(0xcccccc));
    lv_style_set_bg_color(&style_radio, lv_color_hex(0x1a1a1a));

    lv_style_init(&style_radio_chk);
    lv_style_set_bg_image_src(&style_radio_chk, NULL);
    lv_style_set_bg_color(&style_radio_chk, lv_color_hex(0xFF5353));
    lv_style_set_border_color(&style_radio_chk, lv_color_hex(0xcccccc));
    lv_style_set_border_width(&style_radio_chk, 5);
    // lv_style_set_bg_color(&style_radio, lv_color_hex(0xffffff));

    lv_obj_remove_style_all(root);
    lv_obj_set_size(root, LV_HOR_RES, LV_VER_RES);

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

    // 状态栏
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
    lv_obj_set_style_text_font(title_label, &font_puhui_18_4, 0);
    lv_label_set_text(title_label, LT_THEME_SET);
    lv_obj_set_style_text_color(title_label, lv_color_hex(0xe9feff), 0);
    lv_obj_align_to(title_label, btn_back, LV_ALIGN_OUT_RIGHT_MID, 5, 0);

    // 主内容区
    lv_obj_t *cont = lv_obj_create(container);
    lv_obj_align(cont, LV_ALIGN_TOP_MID, 0, 45);
    lv_obj_set_size(cont, 220, 100);
    lv_obj_set_style_border_width(cont, 0, 0);
    lv_obj_set_style_bg_color(cont, lv_color_hex(0X000000), 0);
    lv_obj_set_style_radius(cont, 10, 0);
    lv_obj_set_style_pad_all(cont, 0, 0);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(cont, 2, 0);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_scrollbar_mode(cont, LV_SCROLLBAR_MODE_OFF);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);

    vw.act_id = xz_get_ai_theme();
    // 两个主题项
    vw.radios[0] = create_theme_item(cont, LT_THEME_1, 0);
    vw.radios[1] = create_theme_item(cont, LT_THEME_2, 1);

    return &vw;
}

void setting_theme_view_delete(void) {} 