#ifndef __COUNTDOWN_DONE_VIEW_H__
#define __COUNTDOWN_DONE_VIEW_H__

#include "lvgl.h"

typedef struct
{
    uint8_t is_act; /* 如果没有 u8，改成 uint8_t 并 #include <stdint.h> */
    uint32_t time;
} countdown_done_view_t;

countdown_done_view_t* countdown_done_view_create(lv_obj_t *root, uint32_t time);
void countdown_done_view_delete(void);

#endif /* __COUNTDOWN_DONE_VIEW_H__ */
