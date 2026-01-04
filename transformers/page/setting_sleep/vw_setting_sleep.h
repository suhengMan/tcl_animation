#ifndef __SETTING_SLEEP_VIEW_H__
#define __SETTING_SLEEP_VIEW_H__

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

typedef struct {
    lv_obj_t *root;
    lv_obj_t *set_target;
    lv_obj_t *set_curr;
    uint8_t is_act;
} setting_sleep_view_t;

setting_sleep_view_t* setting_sleep_view_create(lv_obj_t *root);
void setting_sleep_view_delete(void);

#endif
