#include "lvgl.h"
#include "cl_ui.h"
#include "vw_home.h"
#include "../../lv_vpg/lv_vpg.h"
static home_view_t vw;

#define TAG "vw_home"

// static void _on_btn_cb(lv_event_t *e)
// {
//     if (!vw.is_act) return;

//     cl_button_t *btn = (cl_button_t*)lv_event_get_param(e);
//     if (!btn) return;

//     switch (btn->id)
//     {
//         case USER_BUTTON_UP:
//             break;
//         case USER_BUTTON_CENTER:
//             break;
//         case USER_BUTTON_DOWN:
//             break;
//         default:
//             break;
//     }
// }

home_view_t* home_view_create(lv_obj_t *root)
{
    lv_obj_remove_style_all(root);
    lv_obj_center(root);
    lv_obj_set_size(root, LV_HOR_RES, LV_VER_RES);
    // lv_obj_set_pos(root, 0, 0); 
    lv_obj_clear_flag(root, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(root, LV_SCROLLBAR_MODE_OFF);
    lv_obj_center(root);

    lv_obj_clear_flag(ui_get_home(), LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_parent(ui_get_home(), root);
    lv_obj_center(ui_get_home());

    return &vw;
}

void home_view_delete(void)
{
    /* 如需释放资源在此处理 */
}
