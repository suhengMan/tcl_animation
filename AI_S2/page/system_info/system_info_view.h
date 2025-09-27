#ifndef __SYSTEM_INFO_VIEW_H__
#define __SYSTEM_INFO_VIEW_H__

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
#include "lvgl.h"
#else
#include "lvgl/lvgl.h"
#endif

typedef struct {
    lv_obj_t *status_bar;
    lv_obj_t *title_label;
    
    // 系统信息容器
    lv_obj_t *info_cont;
    
    // 系统信息项目
    lv_obj_t *device_name_label;
    lv_obj_t *firmware_version_label;
    lv_obj_t *build_date_label;
    lv_obj_t *cpu_info_label;
    lv_obj_t *memory_info_label;
    lv_obj_t *storage_info_label;
    lv_obj_t *uptime_label;
    
    // 返回按钮
    lv_obj_t *btn_back;
    
    int is_act;
} system_info_view_t;

system_info_view_t* system_info_view_create(lv_obj_t *root);
void system_info_view_delete(void);
void system_info_view_appear_anim_start(bool reverse);

#endif 