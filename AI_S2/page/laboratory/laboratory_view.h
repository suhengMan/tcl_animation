#ifndef __LABORATORY_VIEW_H__
#define __LABORATORY_VIEW_H__

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

typedef struct 
{
    lv_obj_t *menu_cont;
    int is_act;
}laboratory_view_t;



laboratory_view_t* laboratory_view_create(lv_obj_t *root);
void laboratory_view_delete(void);

#endif