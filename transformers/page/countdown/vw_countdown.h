#ifndef __COUNTDOWN_VIEW_H__
#define __COUNTDOWN_VIEW_H__

#include "lvgl.h"

typedef struct
{
    uint8_t is_act; /* 如果没有 u8，改成 uint8_t 并 #include <stdint.h> */
    uint32_t curr_time;  //当前计时
    uint32_t set_time;   //设定时间
    uint32_t last_time;
    lv_obj_t *set_cont;
    lv_obj_t *roller_hour;
    lv_obj_t *roller_min;
    lv_obj_t *roller_sec;
    lv_timer_t *timer_ui;
    lv_obj_t *countdown_cont;
    lv_obj_t *countdown_arc;
    lv_obj_t *countdown_title;
    lv_obj_t *countdown_time;
} countdown_view_t;

countdown_view_t* countdown_view_create(lv_obj_t *root);
void countdown_view_delete(void);
void start_countdown(uint32_t time);

#endif /* __COUNTDOWN_VIEW_H__ */
