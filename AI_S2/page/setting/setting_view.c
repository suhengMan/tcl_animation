#include "setting_view.h"
#include "lvgl.h"
#include "cl_ui.h"
#include "page_manager.h"

static setting_view_t vw;
LV_FONT_DECLARE(font_puhui_18_4)
// 在文件开头添加图片声明
LV_IMG_DECLARE(icon_back_14)
LV_IMG_DECLARE(icon_setting_genera)
LV_IMG_DECLARE(icon_timbre)
LV_IMG_DECLARE(icon_setting_alarm)
LV_IMG_DECLARE(icon_setting_help)
LV_IMG_DECLARE(icon_setting_info)

// 亮度调节接口（你可以替换为实际硬件接口）
static int s_brightness = 50;
static void set_brightness(int val) {
    s_brightness = val;
    lv_bar_set_value(vw.brightness_bar, val, LV_ANIM_OFF);
    static char buf[8];
    snprintf(buf, sizeof(buf), "%d%%", val);
    lv_label_set_text(vw.brightness_label, buf);
}

// 获取焦点时回调，改变时间label颜色
static void _on_item_focus_cb(lv_event_t * e)
{
    lv_obj_t * obj = lv_event_get_target(e);
    // 获取焦点时，时间label颜色变深
    lv_obj_set_style_bg_color(obj, lv_color_hex(0x484848), 0);
}

// 失去焦点时回调，恢复时间label颜色
static void _on_item_defocus_cb(lv_event_t * e)
{
    lv_obj_t * obj = lv_event_get_target(e);
    // 恢复默认颜色
    lv_obj_set_style_bg_color(obj, lv_color_hex(0x1a1a1a), 0);
}


static void _on_btn_cb(lv_event_t *e)
{
    if (!vw.is_act) return;

    cl_button_t *btn = (cl_button_t*)lv_event_get_param(e);
    if (!btn) return;

    switch (btn->id)
    {
        case USER_BUTTON_UP:
            break;
        case USER_BUTTON_CENTER:
            if (btn->event == BTN_CLICK)
            {
                page_change("home");
            }
            break;
        case USER_BUTTON_DOWN:
            break;
        default:
            break;
    }
}


static lv_obj_t* creat_setting_item(lv_obj_t * parent, void *icon_src, const char *title, void (*on_click)(lv_event_t* e))
{
    // 创建容器，布局水平排列，宽度和父容器一样
    lv_obj_t *cont = lv_button_create(parent);
    lv_obj_set_size(cont, 220, 40);
    lv_obj_set_style_shadow_width(cont, 0, 0);
    lv_obj_set_style_border_width(cont, 0, 0);
    lv_obj_set_style_outline_color(cont, lv_color_black(), 0);
    // lv_obj_set_style_outline_color(cont, lv_color_black(), LV_STATE_FOCUSED);
    // lv_obj_set_style_outline_width(cont, 0, LV_STATE_FOCUSED);
    lv_obj_set_style_outline_width(cont, 0, LV_STATE_FOCUS_KEY);
    lv_obj_set_style_bg_color(cont, lv_color_black(), 0);
    // lv_obj_set_style_bg_color(cont, lv_color_hex(0xff5353), LV_STATE_FOCUSED);
    lv_obj_set_style_radius(cont, 5, 0);

    lv_obj_t *icon = lv_img_create(cont);
    lv_img_set_src(icon, icon_src);
    lv_obj_align(icon, LV_ALIGN_LEFT_MID, 10, 0);

    lv_obj_t *label = lv_label_create(cont);
    lv_obj_set_style_text_font(label, &font_puhui_18_4, 0);
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_label_set_text(label, title);
    lv_obj_align_to(label, icon, LV_ALIGN_OUT_RIGHT_MID, 15, 0);

    // 使容器可聚焦
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
    // lv_obj_add_flag(cont, LV_OBJ_FLAG_SCROLL_ON_FOCUS);
    // lv_group_t *group = lv_group_get_default();
    // lv_group_add_obj(group, cont);

    if (on_click != NULL)
    {
        lv_obj_add_event_cb(cont, on_click, LV_EVENT_CLICKED, cont);
    }
    return cont;
}

static void _on_genera_menu(lv_event_t *e){
    page_change("setting_genera");
}

static void _on_alarm_menu(lv_event_t *e){
    // page_change(UI_PG_ALARM);
}

static void _on_system_info_menu(lv_event_t *e){
    page_change("system_info");
}

static void _on_role_menu(lv_event_t *e){
    // page_change(UI_PG_ROLE);
}

static void _on_help_menu(lv_event_t *e){
    page_change("help");
}

static void _on_back_click(lv_event_t *e){
    page_change("home");
}



setting_view_t* setting_view_create(lv_obj_t *root)
{
    lv_obj_remove_style_all(root);
    lv_obj_set_size(root, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_bg_opa(root, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(root, 0, 0);
    lv_obj_set_style_pad_all(root, 0, 0);
    lv_obj_remove_flag(root, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_center(root);
    lv_obj_set_style_bg_color(root, lv_color_hex(0x000000), 0);

    // 状态栏
    vw.status_bar = lv_obj_create(root);
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
    // lv_obj_set_style_bg_color(btn_back, lv_color_hex(0xff5353), LV_STATE_FOCUSED);
    lv_obj_clear_flag(btn_back, LV_OBJ_FLAG_SCROLLABLE);
    // lv_obj_set_style_outline_color(btn_back, lv_color_hex(0x1a1a1a), LV_STATE_FOCUS_KEY);
    lv_obj_add_event_cb(btn_back, _on_back_click, LV_EVENT_CLICKED, NULL);
    lv_obj_t *btn_img =  lv_img_create(btn_back);
    lv_img_set_src(btn_img, &icon_back_14);
    lv_obj_center(btn_img);

    lv_obj_t *title_label = lv_label_create(vw.status_bar);
    lv_obj_set_style_text_font(title_label, &font_puhui_18_4, 0);
    lv_label_set_text(title_label, "设置");
    lv_obj_set_style_text_color(title_label, lv_color_hex(0xe9feff), 0);
    lv_obj_align_to(title_label, btn_back, LV_ALIGN_OUT_RIGHT_MID, 5, 0);

    // 主内容区
    lv_obj_t *cont = lv_obj_create(root);
    lv_obj_align_to(cont, vw.status_bar, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);
    lv_obj_set_size(cont, LV_HOR_RES, LV_VER_RES-40);
    lv_obj_set_style_border_width(cont, 0, 0);
    lv_obj_set_style_bg_color(cont, lv_color_hex(0x000000), 0);
    lv_obj_set_style_radius(cont, 0, 0);
    lv_obj_set_flex_flow(cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_row(cont, 5, 0);
    lv_obj_set_flex_align(cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_scrollbar_mode(cont, LV_SCROLLBAR_MODE_OFF);

    // Start of Selection
    lv_obj_t* genera_menu =  creat_setting_item(cont, &icon_setting_genera, LT_SETTING, _on_genera_menu);
    creat_setting_item(cont, &icon_timbre, LT_TIMBRE, _on_role_menu);
    // creat_setting_item(cont, &icon_setting_alarm, LT_ALARM, _on_alarm_menu);
    creat_setting_item(cont, &icon_setting_help, LT_HELP, _on_help_menu);
    creat_setting_item(cont, &icon_setting_info, LT_SYS_INFO, _on_system_info_menu);

    lv_obj_add_event_cb(root, _on_btn_cb, CL_UI_EVENT_BTN, NULL);

    vw.is_act = 0;
    vw.brightness_editing = 0;

    return &vw;
}

void setting_view_delete(void) {}