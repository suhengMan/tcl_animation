#ifndef TRANSFORMERS_LV_ARC_MENU_H
#define TRANSFORMERS_LV_ARC_MENU_H

#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

lv_obj_t * lv_arc_menu_create(lv_obj_t * parent);

lv_obj_t *lv_arc_menu_btn_crtate(lv_obj_t * parent, void *lv_img_set_src, lv_event_cb_t cb);

lv_obj_t *lv_arc_menu_add_btn(lv_obj_t * obj, void * image, lv_event_cb_t event_cb);

void lv_arc_menu_set_rotate(lv_obj_t *obj, int32_t angles)
;
#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* TRANSFORMERS_LV_RING_MENU_H */


