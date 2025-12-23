#ifndef __HOME_VIEW_H__
#define __HOME_VIEW_H__

#include "lvgl.h"

typedef struct
{
    uint8_t is_act; /* 如果没有 u8，改成 uint8_t 并 #include <stdint.h> */
    bool loop;
    lv_obj_t *emoji;
    lv_obj_t *chat_message;
    lv_obj_t *status;
    lv_obj_t *listen;
    // lv_obj_t *notification;
    // lv_obj_t *power;
    // lv_obj_t *mode;
    // lv_obj_t *volume;
    // lv_obj_t *brightness;
    // lv_obj_t *language;
} home_view_t;

home_view_t* home_view_create(lv_obj_t *root);
void home_view_delete(void);

#endif /* __HOME_VIEW_H__ */
