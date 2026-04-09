#include "lvgl.h"
#include "cl_ui.h"
#include "vw_power.h"

static power_view_t vw;

#define TAG "vw_power"

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

power_view_t* power_view_create(lv_obj_t *root)
{
    lv_obj_remove_style_all(root);
    lv_obj_set_size(root, LV_HOR_RES, LV_VER_RES);

    lv_obj_add_event_cb(root, _on_btn_cb, CL_UI_EVENT_BUTTON, NULL);

    return &vw;
}

void power_view_delete(void)
{
    /* 如需释放资源在此处理 */
}
