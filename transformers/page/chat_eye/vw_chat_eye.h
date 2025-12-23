#ifndef __CHAT_EYE_VIEW_H__
#define __CHAT_EYE_VIEW_H__

#include "lvgl.h"

typedef struct
{
    uint8_t is_act; /* 如果没有 u8，改成 uint8_t 并 #include <stdint.h> */
} chat_eye_view_t;

chat_eye_view_t* chat_eye_view_create(lv_obj_t *root);
void chat_eye_view_delete(void);

#endif /* __CHAT_EYE_VIEW_H__ */
