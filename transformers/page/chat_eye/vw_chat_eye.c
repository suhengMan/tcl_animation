#include "lvgl.h"
#include "cl_ui.h"
#include "vw_chat_eye.h"
#include "lv_vpg/lv_vpg.h"

static chat_eye_view_t vw;

#define TAG "vw_chat_eye"

#ifndef SIMULATOR
#define ASSERT_PREXI "P:/SYS/"
#else
#define ASSERT_PREXI "P:/home/arzhe/Proj/xiaozhi/simulator/sim_sd"
#endif

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
                #ifndef SIMULATOR
                extern void toggleChatState();
                toggleChatState();
                #endif
            }
            break;
        case CL_UI_KEY_MODE:
            if (btn->event == CL_BTN_CLICK)
            {
                page_change("music");
            }    
            break;
        default:
            break;
    }
}

chat_eye_view_t* chat_eye_view_create(lv_obj_t *root)
{
    lv_obj_remove_style_all(root);
    lv_obj_set_style_bg_color(root, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_size(root, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_bg_opa(root, LV_OPA_COVER, 0);

    lv_obj_t *emoji_vpg = lv_vpg_create(root);
    lv_vpg_set_src(emoji_vpg, ASSERT_PREXI"/happy.vpg");
    lv_obj_align(emoji_vpg, LV_ALIGN_CENTER, 0, 0);
    
    lv_obj_add_event_cb(root, _on_btn_cb, CL_UI_EVENT_BUTTON, NULL);
    return &vw;
}

void chat_eye_view_delete(void)
{
    /* 如需释放资源在此处理 */
}
