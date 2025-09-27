#ifndef __setting_genera_VIEW_H__
#define __setting_genera_VIEW_H__

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

typedef struct {
    lv_obj_t *root;
    lv_obj_t *status_bar;
    lv_obj_t *btn_tone;
    lv_obj_t *btn_about;
    int is_act;
} setting_genera_view_t;

setting_genera_view_t* setting_genera_view_create(lv_obj_t *root);
void setting_genera_view_delete(void);

#endif
