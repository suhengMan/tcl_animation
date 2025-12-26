#ifndef __PHONE_CALL_VIEW_H__
#define __PHONE_CALL_VIEW_H__

#include "lvgl.h"

typedef struct
{
    uint8_t is_act; /* 如果没有 u8，改成 uint8_t 并 #include <stdint.h> */
    lv_obj_t *btn_hangup;
    lv_obj_t *btn_answer;
} phone_call_view_t;

phone_call_view_t* phone_call_view_create(lv_obj_t *root, const char *phone);
void phone_call_view_delete(void);

#endif /* __PHONE_CALL_VIEW_H__ */
