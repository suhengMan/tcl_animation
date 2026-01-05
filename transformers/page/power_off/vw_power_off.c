#include "lvgl.h"
#include "cl_ui.h"
#include "vw_power_off.h"
#include "lv_vpg/lv_vpg.h"
#ifndef SIMULATOR
#include <driver/gpio.h>
#endif



static power_off_view_t vw;

#define TAG "vw_power_off"

static void _on_vpg_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t *vpg = lv_event_get_target(e);
    if (code == LV_EVENT_CANCEL)
    {
#ifndef SIMULATOR
        gpio_set_direction(GPIO_NUM_0, GPIO_MODE_OUTPUT);
        gpio_set_level(GPIO_NUM_0, 0);
#endif
    }else{
        uint32_t *is_progress = (uint32_t *)lv_event_get_param(e);
        lv_vpg_set_src(vpg, NULL);
        *is_progress = 1;
#ifndef SIMULATOR
        gpio_set_direction(GPIO_NUM_0, GPIO_MODE_OUTPUT);
        gpio_set_level(GPIO_NUM_0, 0);
#endif
    }
    
}

power_off_view_t* power_off_view_create(lv_obj_t *root)
{
    lv_obj_remove_style_all(root);
    lv_obj_set_size(root, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_bg_color(root, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(root, LV_OPA_COVER, 0);
    lv_obj_clear_flag(root, LV_OBJ_FLAG_SCROLLABLE);
    
    lv_obj_t *vpg = lv_vpg_create(root);
    lv_obj_add_event_cb(vpg, _on_vpg_event_cb, LV_EVENT_READY, NULL);
    lv_obj_add_event_cb(vpg, _on_vpg_event_cb, LV_EVENT_CANCEL, NULL);
    lv_vpg_set_src(vpg, "P:/SYS/poweroff.vpg");
    lv_obj_center(vpg);

    return &vw;
}

void power_off_view_delete(void)
{
    /* 如需释放资源在此处理 */
}
