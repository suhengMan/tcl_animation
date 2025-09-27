#ifndef __SETTING_THEME_VIEW_H__
#define __SETTING_THEME_VIEW_H__

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

typedef struct setting_theme_view_t {
    lv_obj_t *status_bar;
    lv_obj_t *radios[2];
    u8 act_id;
    int is_act;
    int selected;
} setting_theme_view_t;

setting_theme_view_t* setting_theme_view_create(lv_obj_t *root);
void setting_theme_view_delete(void);

#endif 