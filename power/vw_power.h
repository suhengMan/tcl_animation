#ifndef __POWER_VIEW_H__
#define __POWER_VIEW_H__

#include "lvgl.h"

typedef struct
{
    uint8_t is_act; /* 如果没有 u8，改成 uint8_t 并 #include <stdint.h> */
} power_view_t;

power_view_t* power_view_create(lv_obj_t *root);
void power_view_delete(void);

#endif /* __POWER_VIEW_H__ */
