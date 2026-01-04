#ifndef __ALARM_VIEW_H__
#define __ALARM_VIEW_H__

#include "lvgl.h"

#ifndef SIMULATOR
#include "vb_adapter.h"
#else
enum {
    ALARM_MODE_ONCE            = 0x00,
    ALARM_MODE_EVERY_DAY       = 0x01,
    ALARM_MODE_EVERY_MONDAY    = 0x02,
    ALARM_MODE_EVERY_TUESDAY   = 0x04,
    ALARM_MODE_EVERY_WEDNESDAY = 0x08,
    ALARM_MODE_EVERY_THURSDAY  = 0x10,
    ALARM_MODE_EVERY_FRIDAY    = 0x20,
    ALARM_MODE_EVERY_SATURDAY  = 0x40,
    ALARM_MODE_EVERY_SUNDAY    = 0x80,
};


typedef struct {
    uint8_t index;      // 闹钟索引
    uint8_t sw;         // 开关状态 (0:关, 1:开)
    uint8_t mode;       // 重复模式 (见alarm.h中的E_ALARM_MODE_xxx)
    uint8_t hour;       // 时 (0-23)
    uint8_t min;        // 分 (0-59)
    uint8_t sec;        // 秒 (0-59)
} __attribute__((packed)) alarm_transfer_t;
#endif

typedef enum{
    ALARM_MODE_EDIT,
    ALARM_MODE_ADD,
}alarm_mode_t;

typedef struct
{
    uint8_t is_act; /* 如果没有 u8，改成 uint8_t 并 #include <stdint.h> */
    alarm_transfer_t alarm_edit;
    alarm_mode_t mode;
    lv_obj_t *label_time;
    lv_obj_t *cont_list;
    lv_obj_t *cont_info;
    lv_obj_t *cont_time_edit;
    lv_obj_t *cont_loop_edit;

    lv_obj_t *roller_hour;
    lv_obj_t *roller_min;

    lv_obj_t *label_time_set;
    lv_obj_t *label_loop;

    lv_obj_t *btn_week[7];

    lv_timer_t *timer;
} alarm_view_t;

alarm_view_t* alarm_view_create(lv_obj_t *root);
void alarm_view_delete(void);

#endif /* __ALARM_VIEW_H__ */
