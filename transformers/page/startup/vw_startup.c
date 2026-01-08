#include "lvgl.h"
#include "cl_ui.h"
#include "vw_startup.h"

#include "lv_vpg/lv_vpg.h"

static startup_view_t vw;

#define TAG "vw_startup"

static void _startup_cancel_timer_cb(lv_timer_t *timer)
{
    // 切换到home页面
    page_change("home");
    // 删除timer（虽然lv_timer默认是一次性的，但保险起见）
    lv_timer_del(timer);
}

static void _on_vpg_event_cb(lv_event_t *e)
{
    lv_obj_t *vpg = lv_event_get_target(e);
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CANCEL) {
        // 创建一个3000ms后触发的timer, 一次性
        lv_timer_t *timer = lv_timer_create(_startup_cancel_timer_cb, 3000, NULL);
        lv_timer_set_repeat_count(timer, 1); // 一次性
        lv_obj_t *parent = lv_obj_get_parent(vpg);
        if (parent)
        {
            lv_obj_t *label = lv_label_create(parent);
            lv_label_set_text(label, "system err: 0001"); // 可根据需求替换为合适的文案
            lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
            lv_obj_set_style_text_color(label, lv_color_white(), 0);
            lv_obj_set_style_text_font(label, cl_ui_get_font(), 0);
            lv_obj_center(label);
        }
        return;
    } else {
        uint32_t *is_progress = (uint32_t *)lv_event_get_param(e);
        lv_vpg_set_src(vpg, NULL);
        *is_progress = 1;
        page_change("home");
    }
}

static void _on_btn_cb(lv_event_t *e) {
    if (!vw.is_act) return;
    cl_button_t *btn = (cl_button_t*)lv_event_get_param(e);
    if (!btn) return;
    btn->stop_propagate = 1;
}

startup_view_t* startup_view_create(lv_obj_t *root)
{
    lv_obj_remove_style_all(root);
    lv_obj_set_size(root, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_bg_color(root, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(root, LV_OPA_COVER, 0);
    lv_obj_clear_flag(root, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t *vpg = lv_vpg_create(root);
    lv_obj_add_event_cb(vpg, _on_vpg_event_cb, LV_EVENT_READY, NULL);
    lv_obj_add_event_cb(vpg, _on_vpg_event_cb, LV_EVENT_CANCEL, NULL);
    lv_vpg_set_src(vpg, "P:/SYS/poweron.vpg");
    lv_obj_center(vpg);

    lv_obj_add_event_cb(root, _on_btn_cb, CL_UI_EVENT_BUTTON, NULL);
    return &vw;
}

void startup_view_delete(void)
{
    /* 如需释放资源在此处理 */
}
