#include "system_info_view.h"
#include "lvgl.h"
#include "cl_ui.h"

static system_info_view_t vw;
LV_FONT_DECLARE(font_puhui_18_4)
LV_IMG_DECLARE(icon_back_14)


// 创建信息项
static lv_obj_t* create_info_item(lv_obj_t * parent, const char* label_text, const char* value_text)
{
    lv_obj_t * cont = lv_obj_create(parent);
    lv_obj_set_size(cont, 200, 35);
    lv_obj_set_style_pad_all(cont, 5, 0);
    lv_obj_set_style_border_width(cont, 1, 0);
    lv_obj_set_style_border_color(cont, lv_color_hex(0x535353), 0);
    lv_obj_set_style_border_side(cont, LV_BORDER_SIDE_BOTTOM, 0);
    lv_obj_set_style_bg_color(cont, lv_color_hex(0x1a1a1a), 0);
    lv_obj_set_style_radius(cont, 5, 0);

    if (lv_text_get_width(value_text, strlen(value_text), &font_puhui_18_4, 0) >= 140)
    {
        
    }else{
        lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_ROW);
        lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_SPACE_BETWEEN, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    }
    

    // 标签
    lv_obj_t * label = lv_label_create(cont);
    lv_label_set_text(label, label_text);
    lv_obj_set_style_text_color(label, lv_color_hex(0xcccccc), 0);
    lv_obj_set_style_text_font(label, &font_puhui_18_4, 0);

    // 值
    lv_obj_t * value = lv_label_create(cont);
    lv_label_set_text(value, value_text);
    lv_obj_set_style_text_color(value, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_text_font(value, &font_puhui_18_4, 0);

    if (lv_text_get_width(value_text, strlen(value_text), &font_puhui_18_4, 0) >= 140)
    {

        lv_obj_set_size(cont, 200, 50);
        lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_set_width(value, 190);
        lv_label_set_long_mode(value, LV_LABEL_LONG_SCROLL);
        lv_obj_set_style_pad_bottom(cont, 2, 0);
        lv_obj_align_to(value, label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 5);
    }

    return cont;
}
// 解析 __DATE__ 宏并转换为 yyyy-mm-dd 格式
static const char* get_build_date_iso(void) {
    static char date_str[11];
    const char* month_names[] = {
        "Jan", "Feb", "Mar", "Apr", "May", "Jun",
        "Jul", "Aug", "Sep", "Oct", "Nov", "Dec"
    };
    
    // __DATE__ 格式: "Mmm dd yyyy"
    int month = 1;
    for (int i = 0; i < 12; i++) {
        if (__DATE__[0] == month_names[i][0] && 
            __DATE__[1] == month_names[i][1] && 
            __DATE__[2] == month_names[i][2]) {
            month = i + 1;
            break;
        }
    }
    
    snprintf(date_str, sizeof(date_str), "%c%c%c%c-%02d-%c%c", 
             __DATE__[7], __DATE__[8], __DATE__[9], __DATE__[10],
             month,
             __DATE__[4] == ' ' ? '0' : __DATE__[4], 
             __DATE__[5]);
    
    return date_str;
}

static void _on_back_click(lv_event_t *e){
    page_change("setting");
}


system_info_view_t* system_info_view_create(lv_obj_t *root)
{
    lv_obj_remove_style_all(root);
    lv_obj_set_size(root, LV_HOR_RES, LV_VER_RES);
    // 初始化视图结构
    memset(&vw, 0, sizeof(system_info_view_t));
    
    // 创建主容器
    lv_obj_t * main_cont = lv_obj_create(root);
    lv_obj_set_size(main_cont, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_pad_all(main_cont, 0, 0);
    lv_obj_remove_flag(main_cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_border_width(main_cont, 0, 0);
    lv_obj_set_style_bg_color(main_cont, lv_color_hex(0x000000), 0);
    lv_obj_center(main_cont);

    vw.status_bar = lv_obj_create(main_cont);
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
    lv_label_set_text(title_label, LT_INFO);
    lv_obj_set_style_text_color(title_label, lv_color_hex(0xe9feff), 0);
    lv_obj_align_to(title_label, btn_back, LV_ALIGN_OUT_RIGHT_MID, 5, 0);

    // 信息容器
    vw.info_cont = lv_obj_create(main_cont);
    lv_obj_set_size(vw.info_cont, 230, 196);
    lv_obj_set_style_pad_all(vw.info_cont, 10, 0);
    lv_obj_set_style_border_width(vw.info_cont, 0, 0);
    lv_obj_set_style_bg_color(vw.info_cont, lv_color_hex(0x1a1a1a), 0);
    lv_obj_set_style_radius(vw.info_cont, 10, 0);
    lv_obj_set_flex_flow(vw.info_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(vw.info_cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_align_to(vw.info_cont, vw.status_bar, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

    // 创建系统信息项
    vw.device_name_label = create_info_item(vw.info_cont, LT_DEV_NAME, "AI-S2-C");
    vw.firmware_version_label = create_info_item(vw.info_cont, LT_SOFT_VER, CONFIG_BUILD_VERSION);
    vw.build_date_label = create_info_item(vw.info_cont, LT_BUILD_DAY, get_build_date_iso());
    vw.build_date_label = create_info_item(vw.info_cont, LT_DEVICE_ID, xz_get_devid());

    return &vw;
}

void system_info_view_delete(void)
{
}
