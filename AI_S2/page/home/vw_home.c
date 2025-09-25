#include "lvgl.h"
#include "cl_ui.h"
#include "vw_home.h"

static home_view_t vw;

LV_FONT_DECLARE(font_orbitron_38_2)

#define TAG "vw_home"

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
            break;
        case USER_BUTTON_DOWN:
            break;
        default:
            break;
    }
}

home_view_t* home_view_create(lv_obj_t *root)
{
    lv_obj_remove_style_all(root);
    lv_obj_set_size(root, LV_HOR_RES, LV_VER_RES);  
    lv_obj_clear_flag(root, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *img_bg = lv_img_create(root);
    lv_img_set_src(img_bg, ASSERT_PREXI"/img/bg1.png");
    lv_obj_center(img_bg);
    // lv_font_t * font = lv_tiny_ttf_create_file(ASSERT_PREXI"/font/Orbitron-VariableFont_wght.ttf", 38);

    lv_obj_t *label = lv_label_create(root);
    lv_obj_set_style_text_color(label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(label, &font_orbitron_38_2, 0);
    lv_label_set_text(label, "12:48");
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 35);

    ctrl_center_create(root, NULL);

    lv_obj_add_event_cb(root, _on_btn_cb, CL_UI_EVENT_BTN, NULL);

    return &vw;
}

void home_view_delete(void)
{
    /* 如需释放资源在此处理 */
}
