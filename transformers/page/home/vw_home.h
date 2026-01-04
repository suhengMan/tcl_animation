#ifndef __HOME_VIEW_H__
#define __HOME_VIEW_H__

#include "lvgl.h"

typedef struct
{
    uint8_t is_act; /* 如果没有 u8，改成 uint8_t 并 #include <stdint.h> */
    bool loop;
    bool standby;
    lv_obj_t *cont_menu;

    lv_obj_t *emoji;
    lv_obj_t *chat_message;
    lv_obj_t *status;
    lv_obj_t *listen;
} home_view_t;

home_view_t* home_view_create(lv_obj_t *root);
void home_view_delete(void);

#endif /* __HOME_VIEW_H__ */
