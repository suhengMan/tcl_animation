#ifndef __ALARM_RING_VIEW_H__
#define __ALARM_RING_VIEW_H__

#include "lvgl.h"

typedef struct
{
    uint8_t is_act; /* 如果没有 u8，改成 uint8_t 并 #include <stdint.h> */
    lv_timer_t *timer;
    lv_obj_t *label_time;
} alarm_ring_view_t;

alarm_ring_view_t* alarm_ring_view_create(lv_obj_t *root);
void alarm_ring_view_delete(void);

#endif /* __ALARM_RING_VIEW_H__ */
