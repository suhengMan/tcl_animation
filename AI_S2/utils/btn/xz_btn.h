#ifndef __XIAOZHI_BTN_H__
#define __XIAOZHI_BTN_H__

#include "lvgl.h"

typedef enum
{
    USER_BUTTON_UP = LV_KEY_LEFT, // 对应 IoT Board 开发板的 PIN_KEY0
    USER_BUTTON_CENTER = LV_KEY_DOWN,     // 对应 IoT Board 开发板的 PIN_KEY1
    USER_BUTTON_DOWN = LV_KEY_RIGHT,     // 对应 IoT Board 开发板的 PIN_KEY2
    USER_BUTTON_MAX
} user_button_t;


typedef enum
{
    BTN_DOWN,
    BTN_CLICK,
    BTN_DOUBLE_CLICK,
    BTN_REPEAT_CLICK,
    BTN_SHORT_START,
    BTN_SHORT_UP,
    BTN_LONG_START,
    BTN_LONG_UP,
    BTN_LONG_HOLD,
    BTN_LONG_HOLD_UP,
    BTN_MAX,
    BTN_NONE,
} xiaozhi_btn_event_t;

typedef struct
{
    uint16_t scan_cnt;
    uint16_t click_cnt;
    uint8_t id;
    uint8_t event;
} cl_button_t;

typedef void (*xiaozhi_btn_cb)(cl_button_t *btn);

void xiaozhi_btn_init(xiaozhi_btn_cb cb);

#endif