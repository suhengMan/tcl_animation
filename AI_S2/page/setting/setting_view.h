#ifndef __SETTING_VIEW_H__
#define __SETTING_VIEW_H__

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

typedef struct {
    lv_obj_t *status_bar;
    lv_obj_t *brightness_bar;
    lv_obj_t *brightness_label;
    lv_obj_t *btn_tone;
    lv_obj_t *btn_about;
    int is_act;
    int brightness_editing;
} setting_view_t;

setting_view_t* setting_view_create(lv_obj_t *root);
void setting_view_delete(void);
#endif
