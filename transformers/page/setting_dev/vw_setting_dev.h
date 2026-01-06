#ifndef __SETTING_DEV_VIEW_H__
#define __SETTING_DEV_VIEW_H__

#include "lvgl.h"

typedef struct
{
    uint8_t is_act; /* 如果没有 u8，改成 uint8_t 并 #include <stdint.h> */
} setting_dev_view_t;

setting_dev_view_t* setting_dev_view_create(lv_obj_t *root);
void setting_dev_view_delete(void);

#endif /* __SETTING_DEV_VIEW_H__ */
