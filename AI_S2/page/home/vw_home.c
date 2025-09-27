#include "lvgl.h"
#include "cl_ui.h"
#include "vw_home.h"

static home_view_t vw;

LV_FONT_DECLARE(font_nunito_14_2)
LV_FONT_DECLARE(font_nunito_48_2)

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
                page_change("app_list");
            }
            break;
        case USER_BUTTON_DOWN:
            break;
        default:
            break;
    }
}

static void _on_time_cb(lv_event_t *e){
    lv_obj_t *time_label = lv_event_get_user_data(e);
    cl_time_t *time = lv_event_get_param(e);
    lv_label_set_text_fmt(time_label, "%02d:%02d", time->hour, time->min);
}

home_view_t* home_view_create(lv_obj_t *root)
{
    lv_obj_remove_style_all(root);
    lv_obj_set_size(root, LV_HOR_RES, LV_VER_RES);
    lv_obj_clear_flag(root, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *img_bg = lv_img_create(root);
    lv_img_set_src(img_bg, ASSERT_PREXI"/img/bg2.png");
    lv_obj_center(img_bg);

    lv_font_t * font = lv_tiny_ttf_create_file(ASSERT_PREXI"/font/Orbitron-VariableFont_wght.ttf", 56);
    lv_obj_t *label_time = lv_label_create(root);
    lv_obj_set_style_text_color(label_time, lv_color_hex(0xFCE7C9), 0);
    lv_obj_set_style_text_font(label_time, &font_nunito_48_2, 0);
    cl_time_t time = cl_ui_get_time();
    // time.hour=0; time.min = 0;
    lv_label_set_text_fmt(label_time, "%02d:%02d", time.hour, time.min);
    lv_obj_align(label_time, LV_ALIGN_TOP_MID, 0, 30);

    lv_obj_t *date_label = lv_label_create(root);
    lv_obj_set_style_text_font(date_label, &font_nunito_14_2, 0);
    const char* week_str[] = {LT_WEEK_1, LT_WEEK_2, LT_WEEK_3, LT_WEEK_4, LT_WEEK_5, LT_WEEK_6, LT_WEEK_7};
    lv_label_set_text_fmt(date_label,"%02d/%02d   %s", time.month, time.day, week_str[time.wday]);
    lv_obj_set_style_text_color(date_label, lv_color_hex(0xFCE7C9), 0);
    lv_obj_align_to(date_label, label_time, LV_ALIGN_OUT_BOTTOM_MID, 0, 5);

    ctrl_center_create(root, NULL);

    lv_obj_add_event_cb(root, _on_btn_cb, CL_UI_EVENT_BTN, NULL);
    lv_obj_add_event_cb(root, _on_time_cb, CL_UI_EVENT_TIME_CHANGE, label_time);

    return &vw;
}

void home_view_delete(void)
{
    /* 如需释放资源在此处理 */
}
