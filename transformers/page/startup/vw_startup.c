#include "lvgl.h"
#include "cl_ui.h"
#include "vw_startup.h"

#include "lv_vpg/lv_vpg.h"

static startup_view_t vw;

#define TAG "vw_startup"

static void _on_vpg_event_cb(lv_event_t *e)
{
    lv_obj_t *vpg = lv_event_get_target(e);
    uint32_t *is_progress = (uint32_t *)lv_event_get_param(e);
    lv_vpg_set_src(vpg, NULL);
    *is_progress = 1;
    page_change("home");
}

startup_view_t* startup_view_create(lv_obj_t *root)
{
    lv_obj_remove_style_all(root);
    lv_obj_set_size(root, LV_HOR_RES, LV_VER_RES);

    lv_obj_t *vpg = lv_vpg_create(root);
    lv_obj_add_event_cb(vpg, _on_vpg_event_cb, LV_EVENT_READY, NULL);
    lv_vpg_set_src(vpg, "P:/poweron.vpg");
    lv_obj_center(vpg);

    return &vw;
}

void startup_view_delete(void)
{
    /* 如需释放资源在此处理 */
}
