#ifndef __APP_LIST_VIEW_H__
#define __APP_LIST_VIEW_H__

#include "lvgl.h"

typedef struct
{
    u8 is_act; /* 如果没有 u8，改成 uint8_t 并 #include <stdint.h> */
} app_list_view_t;

app_list_view_t* app_list_view_create(lv_obj_t *root);
void app_list_view_delete(void);

#endif /* __APP_LIST_VIEW_H__ */
