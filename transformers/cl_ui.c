#include "cl_ui.h"
#include "lvgl.h"
#include <time.h>
#include <stdio.h>
#include "vb6824.h"
#include "cl_ui_event.h"
#include <string.h>
#include "esp_lvgl_port.h"
#include "page_manager/inc/page_manager_private.h"

#define TAG "CL_UI"

#define LOGD(format, ...) printf("\033[0;36m" "["TAG"]" format "\033[0m\n", ##__VA_ARGS__)
#define LOGI(format, ...) printf("\033[0;32m" "["TAG"]" format "\033[0m\n", ##__VA_ARGS__)
#define LOGW(format, ...) printf("\033[0;33m" "["TAG"]" format "\033[0m\n", ##__VA_ARGS__)
#define LOGE(format, ...) printf("\033[0;31m" "["TAG"]" format "\033[0m\n", ##__VA_ARGS__)


static page_manager_t *g_page_manager = NULL;
extern page_handle_t __start_cl_ui_page[];
extern page_handle_t __stop_cl_ui_page[];

extern page_base_t *get_stack_top(page_manager_t *self);

LV_FONT_DECLARE(font_puhui_18_4)
static lv_style_t s_global_font_style;

static void _page_install(){
    g_page_manager = page_manager_create();

#ifdef SIMULATOR
    for (uint8_t* t = (uint8_t*)__start_cl_ui_page; t < (uint8_t*)__stop_cl_ui_page; t+=32)
#else
    for (uint8_t* t = (uint8_t*)__start_cl_ui_page; t < (uint8_t*)__stop_cl_ui_page; t+=sizeof(page_handle_t))
#endif
    {
        page_handle_t *p = (page_handle_t*)t;
        LOGD("%s %p", p->name, t);
        pm_install(g_page_manager, p->name, p->create(p->name));
    }
}

int page_change(const char* name){
    
    page_base_t *page = pm_find_page(g_page_manager, name);
    if (page == NULL) {
        LOGE("page_change: 页面未找到");
        return -1;
    }
    LOGD("切换页面:%s", name);
    pm_pop(g_page_manager);
    pm_push(g_page_manager, name, NULL);
    return 1;
}

static void gesture_event_cb(lv_event_t * e)
{
    lv_obj_t * obj = lv_event_get_target(e);
    lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_active());

    lv_event_code_t code = lv_event_get_code(e);
    static lv_point_t point_start;
    static lv_point_t point_end;
    if(code == LV_EVENT_GESTURE) {
        // lv_indev_wait_release(lv_indev_active());
        page_base_t *page = get_stack_top(g_page_manager);
        lv_obj_send_event(page->root, code, e);    
    }    
}


lv_obj_t * home_ui = NULL;
lv_obj_t *ui_get_home(){
    if (home_ui == NULL) {
        home_ui = lv_obj_create(lv_scr_act());
        lv_obj_remove_style_all(home_ui);
        lv_obj_set_size(home_ui, LV_HOR_RES, LV_VER_RES);
        lv_obj_clear_flag(home_ui, LV_OBJ_FLAG_SCROLLABLE);
        lv_obj_add_flag(home_ui, LV_OBJ_FLAG_HIDDEN);
    }
    return home_ui;
}

static void _send_event(lv_event_code_t code, void *param, uint32_t len){
    lvgl_port_lock(0);
    page_base_t *base = get_stack_top(g_page_manager);
    if (base)
    {
        lv_obj_send_event(base->root, code, param);
    }
    lvgl_port_unlock();
}

void xz_ui_evt_send(cl_ui_event_t code, void *param, uint32_t len){
    lv_event_system_send_async((lv_event_code_t)code, param, len, (lv_event_system_route_cb_t )_send_event, NULL);
}


static void _vb_api_evt_handle(vb_api_event_t evt, void *data, void *arg){
    switch (evt)
    {
    case VB_EVT_MODE_CHANGE:{
        uint32_t mode = *((uint8_t*)data);
        xz_ui_evt_send(CL_UI_EVENT_MODE_CHANGE, &mode, sizeof(mode));
    }
    break;
    case VB_EVT_PLAY_STATUS_CHANGE:{
        uint32_t status = *((uint8_t*)data);
        xz_ui_evt_send(CL_UI_EVENT_MUSIC_STATUS, &status, sizeof(status));
    }
        break;
    case VB_EVT_MUSIC_TITLE:{
        uint32_t status = *((uint8_t*)data);
        xz_ui_evt_send(CL_UI_EVENT_MUSIC_TITLE, data, strlen(data)+1);
    }
        break;
    case VB_EVT_MUSIC_LYRC:{
        uint32_t status = *((uint8_t*)data);
        xz_ui_evt_send(CL_UI_EVENT_MUSIC_LYRC, data, strlen(data)+1);
    }
        break;
    case VB_EVT_MUSIC_TIME:{
        xz_ui_evt_send(CL_UI_EVENT_MUSIC_TIME, data, 2*sizeof(uint32_t));
    }
        break;
    default:
        break;
    }
} 



void ui_init(){
    lv_event_system_init(4096, 3);
    lv_style_init(&s_global_font_style);
    lv_style_set_bg_color(&s_global_font_style, lv_color_hex(0xFFFFFF));
    lv_style_set_text_font(&s_global_font_style, &font_puhui_18_4);
    lv_obj_add_style(lv_screen_active(), &s_global_font_style, LV_PART_MAIN);
    
    lv_obj_add_event_cb(lv_scr_act(), gesture_event_cb, LV_EVENT_GESTURE, NULL);
    vb_api_register_evt(_vb_api_evt_handle, NULL);

    _page_install();
    ui_get_home();
#ifdef SIMULATOR
    page_change("music");
#else
    page_change("startup");
#endif
}