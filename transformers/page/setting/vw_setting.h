#ifndef __SETTING_VIEW_H__
#define __SETTING_VIEW_H__

#include "lvgl.h"

typedef struct
{
    uint8_t is_act; /* 如果没有 u8，改成 uint8_t 并 #include <stdint.h> */
    lv_obj_t *label_auto_play_time;
    lv_obj_t *cont_autoplay;
    lv_obj_t *cont_pair;
    lv_obj_t *cont_about;
    lv_obj_t *slider_img;
    int auto_play_time;
    int auto_play_anim_time;
    
} setting_view_t;

setting_view_t* setting_view_create(lv_obj_t *root);
void setting_view_delete(void);

#endif /* __SETTING_VIEW_H__ */
