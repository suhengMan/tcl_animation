#ifndef __ANIM_VIEW_H__
#define __ANIM_VIEW_H__

#include "lvgl.h"

typedef struct
{
    uint8_t is_act; /* 如果没有 u8，改成 uint8_t 并 #include <stdint.h> */
    lv_obj_t *anim;
    lv_obj_t *label;
    char **vpg_list;
    int vpg_count;
    int vpg_index;
} anim_view_t;

anim_view_t* anim_view_create(lv_obj_t *root);
void anim_view_delete(void);

#endif /* __ANIM_VIEW_H__ */
