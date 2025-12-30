#include "cl_ui.h"
#include "lvgl.h"
#include <time.h>
#include <stdio.h>
#include "cl_ui_event.h"
#include <string.h>
#ifndef SIMULATOR
#include "vb_adapter.h"
#include "esp_lvgl_port.h"
#else 
#include "xiaozhi_btn.h"
#endif
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

#ifdef SIMULATOR
LV_FONT_DECLARE(font_puhui_18_4)
#endif
static lv_style_t s_global_font_style;
static lv_font_t *s_font = NULL;
static lv_font_t *s_icon_font = NULL;
static lv_font_t *s_large_icon_font = NULL;

#ifndef SIMULATOR
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
static int16_t s_fft_data[32];
static SemaphoreHandle_t s_fft_data_rwlock = NULL;

static void fft_data_lock_init()
{
    if (s_fft_data_rwlock == NULL) {
        s_fft_data_rwlock = xSemaphoreCreateMutex();
    }
}

static void _update_fft_data(int16_t *data, uint32_t len){
    if (s_fft_data_rwlock == NULL) fft_data_lock_init();
    xSemaphoreTake(s_fft_data_rwlock, portMAX_DELAY);
    memcpy(s_fft_data, data, len*sizeof(int16_t));
    xSemaphoreGive(s_fft_data_rwlock);
}

void cl_ui_get_fft_data(int16_t *data, uint32_t len){
    if (s_fft_data_rwlock == NULL) fft_data_lock_init();
    xSemaphoreTake(s_fft_data_rwlock, portMAX_DELAY);
    memcpy(data, s_fft_data, len);
    xSemaphoreGive(s_fft_data_rwlock);
}
#else
static int16_t s_fft_data[32];

static void _update_fft_data(int16_t *data, uint32_t len){
    memcpy(s_fft_data, data, len);
}

void cl_ui_get_fft_data(int16_t *data, uint32_t len){
    memcpy(data, s_fft_data, len);
}
#endif

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

char *cl_ui_get_curr_page(){
    page_base_t *page = get_stack_top(g_page_manager);
    return page==NULL?NULL:page->name;
}

int page_change(const char* name){
    
    page_base_t *page = pm_find_page(g_page_manager, name);
    if (page == NULL) {
        LOGE("page_change: 页面未找到");
        return -1;
    }
    if (cl_ui_get_curr_page() && strcmp(name, cl_ui_get_curr_page()) == 0) {
        return 1;
    }
    LOGD("切换页面:%s", name);
    pm_pop(g_page_manager);
    pm_push(g_page_manager, name, NULL);
    return 1;
}

int page_change_with_arg(const char* name, void *data, uint32_t len){
    page_base_t *page = pm_find_page(g_page_manager, name);
    if (page == NULL) {
        LOGE("page_change: 页面未找到");
        return -1;
    }
    if (cl_ui_get_curr_page() && strcmp(name, cl_ui_get_curr_page()) == 0) {
        return 1;
    }

    page_stash_t stash = {
        .ptr = data,
        .size = len,
    };

    LOGD("切换页面:%s", name);
    pm_pop(g_page_manager);
    pm_push(g_page_manager, name, &stash);
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

lv_font_t *cl_ui_get_font(){
#ifdef SIMULATOR
    return &font_puhui_18_4;
#else
    return s_font;
#endif
}

void cl_ui_set_font(const lv_font_t *font){
    s_font = font;
}

lv_font_t *cl_ui_get_icon_font(){
    return s_icon_font;
}

void cl_ui_set_icon_font(const lv_font_t *font){
    s_icon_font = font;
}

lv_font_t *cl_ui_get_large_icon_font(){
    return s_large_icon_font;
}

void cl_ui_set_large_icon_font(const lv_font_t *font){
    s_large_icon_font = font;
}


static void _send_event(lv_event_code_t code, void *param, uint32_t len){
#ifndef SIMULATOR
    lvgl_port_lock(0);
#else
    lv_lock();
#endif
    page_base_t *base = get_stack_top(g_page_manager);
    if (base)
    {
        lv_obj_send_event(base->root, code, param);
    }
    if (code == (lv_event_code_t)CL_UI_EVENT_BUTTON)
    {
        cl_button_t *btn = (cl_button_t*)param;
        if (btn->stop_propagate == 0)
        {
            switch (btn->id)
            {
            case CL_UI_KEY_POWER:
                if (btn->event == CL_BTN_LONG_START)
                {
                    page_change("power_off");
                }
                break;
            
            default:
                break;
            }
        }
    }
    
#ifndef SIMULATOR
    lvgl_port_unlock();
#else
    lv_unlock();
#endif
}

void xz_ui_evt_send(cl_ui_event_t code, void *param, uint32_t len){
    lv_event_system_send_async((lv_event_code_t)code, param, len, (lv_event_system_route_cb_t )_send_event, NULL);
}

#ifndef SIMULATOR
static void _vb_evt_handle(uint32_t event_id, void *data, uint16_t len, void *user_arg){
    switch (event_id)
    {
    case VB_EVT_MODE_CHANGE:{
        uint32_t mode = *((uint8_t*)data);
        xz_ui_evt_send(CL_UI_EVENT_MODE_CHANGE, &mode, sizeof(mode));
        break;
    }
    case VB_EVT_STATUS_CHANGE:{
        uint32_t status = *((uint8_t*)data);
        extern void setPowerSaveModeUi(bool enabled);
        if (status == 0)
        {
            setPowerSaveModeUi(true);
        }else
        {
#ifndef SIMULATOR
    // extern void closeChat();
    // closeChat();
#endif
            if((strcmp(cl_ui_get_curr_page(), "startup") != 0 && strcmp(cl_ui_get_curr_page(), "power_off") != 0)&& vb_api_get_music_mode() == VB_MUSIC_MODE_BT){
                lvgl_port_lock(0);
                page_change("music_fft");
                lvgl_port_unlock();
            }
            setPowerSaveModeUi(false);
        }
        
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
    case VB_EVT_PLAY_INDEX:{
        xz_ui_evt_send(CL_UI_EVENT_MUSIC_IDX, data, sizeof(uint32_t));
    }
        break;
    case VB_EVT_FFT:{
        // ESP_LOGW(TAG, "fft data: %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d %d", ((int16_t*)data)[0], ((int16_t*)data)[1], ((int16_t*)data)[2], ((int16_t*)data)[3], ((int16_t*)data)[4], ((int16_t*)data)[5], ((int16_t*)data)[6], ((int16_t*)data)[7], ((int16_t*)data)[8], ((int16_t*)data)[9], ((int16_t*)data)[10], ((int16_t*)data)[11], ((int16_t*)data)[12], ((int16_t*)data)[13], ((int16_t*)data)[14], ((int16_t*)data)[15]);
        xz_ui_evt_send(CL_UI_EVENT_MUSIC_FFT, data, len);
        _update_fft_data(data, len);
    }
        break;
    case VB_EVT_VOL_CHANGE:{
        uint8_t *pvol = (uint8_t*)data;
        // map pvol[0] from 0-30 to 0-100
        uint8_t vol = (uint8_t)pvol[0];
        xz_ui_evt_send(CL_UI_EVENT_MUSIC_VOL, &vol, sizeof(vol));
    }
    break;
    case VB_EVT_PHONE_CALL:{
        char number[32] = {0};
        memcpy(number, data, len);
        lvgl_port_lock(0);
        page_change_with_arg("phone_call", number, sizeof(number));
        lvgl_port_unlock();
    }
    break;
    case VB_EVT_PHONE_CALL_HANGUP:{
        xz_ui_evt_send(CL_UI_EVENT_PHONE_CALL_HANGUP, data, len);
    }
    break;
    default:
        break;
    }
}


void vb_evt_register(){
    vb_event_register(VB_EVT_MODE_CHANGE, _vb_evt_handle, NULL);
    vb_event_register(VB_EVT_STATUS_CHANGE, _vb_evt_handle, NULL);
    vb_event_register(VB_EVT_MUSIC_TITLE, _vb_evt_handle, NULL);
    vb_event_register(VB_EVT_MUSIC_LYRC, _vb_evt_handle, NULL);
    vb_event_register(VB_EVT_MUSIC_TIME, _vb_evt_handle, NULL);
    vb_event_register(VB_EVT_PLAY_INDEX, _vb_evt_handle, NULL);
    vb_event_register(VB_EVT_FFT, _vb_evt_handle, NULL);
    vb_event_register(VB_EVT_VOL_CHANGE, _vb_evt_handle, NULL);
    vb_event_register(VB_EVT_PHONE_CALL, _vb_evt_handle, NULL);
    vb_event_register(VB_EVT_PHONE_CALL_HANGUP, _vb_evt_handle, NULL);
}
#endif


void ui_key_msg(cl_button_t *e, void *arg){
    xz_ui_evt_send(CL_UI_EVENT_BUTTON, e, sizeof(cl_button_t));
}

void wakeup_ai(){
#ifndef SIMULATOR    
    lvgl_port_lock(0);
    page_base_t *page = get_stack_top(g_page_manager);
    vb_api_set_music_play(0);
    if (strcmp(page->name, "home") != 0)
    {
        page_change("home");
    }
    lvgl_port_unlock();
#endif
}




void ui_init(const char* page){
    lv_event_system_init(4096, 3);
    lv_style_init(&s_global_font_style);
#ifdef SIMULATOR
    xiaozhi_btn_init();
#endif
    lv_style_set_bg_color(&s_global_font_style, lv_color_hex(0xFFFFFF));
    if (s_font) {
        lv_style_set_text_font(&s_global_font_style, s_font);
        lv_obj_add_style(lv_screen_active(), &s_global_font_style, LV_PART_MAIN);
    }

    cl_init_arc_menu();
    lv_obj_add_event_cb(lv_scr_act(), gesture_event_cb, LV_EVENT_GESTURE, NULL);
#ifndef SIMULATOR
    vb_evt_register();
#endif
    _page_install();
    // ui_get_home();
#ifdef SIMULATOR
    page_change("alarm");
    // page_change("home");
#else
    page_change(page==NULL?"startup":page);
#endif
}
