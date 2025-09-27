 #ifndef __HELP_VIEW_H__
#define __HELP_VIEW_H__

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

typedef struct {
    lv_obj_t *status_bar;
    lv_obj_t *qr_code_img;
    lv_obj_t *help_text_label;
    int is_act;
} help_view_t;

help_view_t* help_view_create(lv_obj_t *root);
void help_view_delete(void);
void help_view_appear_anim_start(bool reverse);

#endif