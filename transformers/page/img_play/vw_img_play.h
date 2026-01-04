#ifndef __IMG_PLAY_VIEW_H__
#define __IMG_PLAY_VIEW_H__

#include "lvgl.h"

typedef struct
{
    uint8_t is_act; /* 如果没有 u8，改成 uint8_t 并 #include <stdint.h> */
    lv_obj_t *img;
    lv_obj_t *label;
    lv_obj_t *cont_menu;
    lv_timer_t *play_timer;

    char **jpg_list;
    int jpg_count;
    int jpg_index;
} img_play_view_t;

img_play_view_t* img_play_view_create(lv_obj_t *root);
void img_play_view_delete(void);

#endif /* __IMG_PLAY_VIEW_H__ */
