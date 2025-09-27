#include "lvgl.h"
#include "cl_ui.h"
#include "vw_app_list.h"

static app_list_view_t vw;
static lv_font_t *s_main_font = NULL;


LV_FONT_DECLARE(font_puhui_18_4)
#define TAG "vw_app_list"

static on_app_click_cb(lv_event_t *e){
    lv_event_code_t code = lv_event_get_code(e);
    const char* name = lv_event_get_user_data(e);
    if(code == LV_EVENT_CLICKED) {
        if (strcmp(name, LT_XIAOLE)==0)
        {
            page_change("xiaozhi_eye");
        }else if (strcmp(name, LT_MUSIC)==0)
        {
            page_change("music");
        }else if (strcmp(name, LT_ALARM)==0)
        {
            page_change("alarm");
        }else if (strcmp(name, LT_SETTING)==0)
        {
            page_change("setting");
        }else if (strcmp(name, LT_LIGHT)==0)
        {
            page_change("flash_light");
        }else if (strcmp(name, LT_ANIM)==0)
        {
            page_change("animation");
        }
    }
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


static lv_obj_t *lv_circle_btn_create(lv_obj_t *parent, char* name, void *src, lv_color_t color, int size, lv_event_cb_t event_cb)
{
    
    lv_obj_t *btn = lv_btn_create(parent);
    lv_obj_set_size(btn, size, size+35);
    lv_obj_set_style_radius(btn, size/2, 0);
    lv_obj_set_style_bg_color(btn, color, 0);
    lv_obj_set_align(btn, LV_ALIGN_CENTER);
    lv_obj_set_style_pad_all(btn, 0 ,0);
    lv_obj_set_style_border_width(btn, 0, 0);
    lv_obj_set_style_shadow_width(btn, 0, 0);

    if (name)
    {
        lv_obj_t *label = lv_label_create(btn);
        lv_obj_set_style_text_font(label, s_main_font, 0);
        lv_obj_set_style_text_color(label, lv_color_hex3(0xfff), 0);
        lv_obj_align(label, LV_ALIGN_BOTTOM_MID, 0, 0);
        lv_label_set_text(label, name);
    }
    

    if (src)
    {
        lv_obj_t *img = lv_img_create(btn);
        // lv_obj_center(img);
        lv_obj_align(img, LV_ALIGN_TOP_MID, 0, 0);
        lv_img_set_src(img, src);
    }

    if (event_cb)
    {
        lv_obj_add_event_cb(btn, event_cb, LV_EVENT_ALL, name);
    }
    
    return btn;
}

app_list_view_t* app_list_view_create(lv_obj_t *root)
{
    if (s_main_font == NULL)
    {
        s_main_font = &font_puhui_18_4;
    }

    lv_obj_remove_style_all(root);
    lv_obj_set_size(root, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_bg_opa(root, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(root, lv_color_black(), 0);
    lv_obj_set_style_pad_all(root, 0, 0);
    lv_obj_set_style_border_width(root, 0, 0);

    lv_obj_t *btn_ai=       lv_circle_btn_create(root, LT_XIAOLE, ASSERT_PREXI"/icon/AI.png", lv_color_hex(0x0), 64, on_app_click_cb);
    lv_obj_t *btn_music =   lv_circle_btn_create(root, LT_MUSIC, ASSERT_PREXI"/icon/music.png", lv_color_hex(0x0), 64, on_app_click_cb);
    lv_obj_t *btn_alarm =   lv_circle_btn_create(root, LT_ALARM, ASSERT_PREXI"/icon/alarm.png", lv_color_hex(0x0), 64, on_app_click_cb);
    lv_obj_t *btn_setting = lv_circle_btn_create(root, LT_SETTING, ASSERT_PREXI"/icon/setting.png", lv_color_hex(0x0), 64, on_app_click_cb);
    // lv_obj_t *btn_light =   lv_circle_btn_create(root, LT_LIGHT, &icon_light_64, lv_color_hex(0x0), 64, on_app_click_cb);
    // lv_obj_t *btn_video =   lv_circle_btn_create(root, LT_ANIM, &icon_anim_64, lv_color_hex(0x0), 64, on_app_click_cb);

    lv_obj_align(btn_ai, LV_ALIGN_TOP_MID, -50, 40);
    lv_obj_align(btn_music, LV_ALIGN_TOP_MID, 50, 40);
    lv_obj_align(btn_alarm, LV_ALIGN_TOP_MID, -50, 170);
    lv_obj_align(btn_setting, LV_ALIGN_TOP_MID, 50, 170);

    lv_obj_add_event_cb(root, _on_btn_cb, CL_UI_EVENT_BTN, NULL);
    vw.is_act = 0;

    return &vw;
}

void app_list_view_delete(void)
{
    /* 如需释放资源在此处理 */
}
