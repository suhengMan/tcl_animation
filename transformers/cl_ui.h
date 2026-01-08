#ifndef _CL_UI_H_
#define _CL_UI_H_

#include "page_manager.h"
#include "cl_ui.h"
#include "assert/language.h"
#ifdef __cplusplus
extern "C"
{
#endif

#ifndef SIMULATOR
#include "esp_log.h"
#else
typedef enum{
    USER_BUTTON_CENTER,
    USER_BUTTON_BACK,
}cl_ui_btn_t;
typedef enum{
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
}cl_key_event_t;
#define u32 uint32_t
#define u16 uint16_t
#define u8 uint8_t
#define CL_UI_EVENT_BTN CL_UI_EVENT_BUTTON
#define ESP_LOGI(tag, format, ...)  printf("\033[0;32m" format "\033[0m\n", ##__VA_ARGS__)
#define ESP_LOGW(tag, format, ...)  printf("\033[0;33m" format "\033[0m\n", ##__VA_ARGS__)
#define ESP_LOGE(tag, format, ...)  printf("\033[0;31m" format "\033[0m\n", ##__VA_ARGS__)
#define lvgl_port_unlock()
#define lvgl_port_lock(a)
#endif

typedef enum{
    CL_UI_EVENT_MUSIC_STATUS = LV_EVENT_LAST + 1,
    CL_UI_EVENT_MODE_CHANGE,
    CL_UI_EVENT_BT_STATUS,
    CL_UI_EVENT_MUSIC_FFT,
    CL_UI_EVENT_MUSIC_TITLE,
    CL_UI_EVENT_MUSIC_LYRC,
    CL_UI_EVENT_MUSIC_TIME,
    CL_UI_EVENT_MUSIC_IDX,
    CL_UI_EVENT_MUSIC_VOL,
    CL_UI_EVENT_BUTTON,
    CL_UI_EVENT_PAGE_CHANGE,
    CL_UI_EVENT_PHONE_CALL_HANGUP,
    CL_UI_EVENT_SET_COUNTDOWN,
}cl_ui_event_t;

typedef enum{
    CL_UI_KEY_POWER,
    CL_UI_KEY_MODE,
    CL_UI_KEY_VOL_UP,
    CL_UI_KEY_VOL_DOWN,
    CL_UI_KEY_VOL_MAX,
}cl_ui_key_t;

typedef enum{
    CL_BTN_DOWN,
    CL_BTN_CLICK,
    CL_BTN_DOUBLE_CLICK,
    CL_BTN_REPEAT_CLICK,
    CL_BTN_SHORT_START,
    CL_BTN_SHORT_UP,
    CL_BTN_LONG_START,
    CL_BTN_LONG_UP,
    CL_BTN_LONG_HOLD,
    CL_BTN_LONG_HOLD_UP,
    CL_BTN_MAX,
    CL_BTN_NONE,
}cl_ui_key_event_t;

typedef struct
{
    uint16_t scan_cnt;
    uint16_t click_cnt;
    cl_ui_key_t id;
    cl_ui_key_event_t event;
    //停止向后传播
    uint8_t stop_propagate;
} cl_button_t;

void ui_init(const char* page);
int page_change(const char* name);
int page_change_with_arg(const char* name, void *data, uint32_t len);
lv_obj_t *ui_get_home();
void ui_key_msg(cl_button_t *e, void *arg);
void wakeup_ai();
char *cl_ui_get_curr_page();

lv_font_t *cl_ui_get_font();
void cl_ui_set_font(const lv_font_t *font);
lv_font_t *cl_ui_get_icon_font();
void cl_ui_set_icon_font(const lv_font_t *font);
lv_font_t *cl_ui_get_large_icon_font();
void cl_ui_set_large_icon_font(const lv_font_t *font);

void cl_set_emoji(const char *emoji);
void cl_set_chat_message(const char *message);
void cl_set_status(const char *status);

void cl_ui_get_fft_data(int16_t *data, uint32_t len);

void cl_arc_menu_show(bool show);
void cl_init_arc_menu();

void cl_ui_show_vol_bar(int show_time);
void cl_ui_vol_set_vol(uint8_t vol);
uint8_t cl_ui_vol_bar_is_show();
void cl_ui_vol_bar_hide();
void cl_ui_init_vol_bar();

#ifndef SIMULATOR
void reset_wifi_configuation();
extern void set_shutdown_time(int time);
extern char* xz_sys_get_mac();
extern int xz_setting_get_int(char *key, int def);
extern bool xz_setting_get_bool(char *key, bool def);
extern char* xz_setting_get_string(char *key, char* def);
extern void xz_setting_set_int(char *key, int val);
extern void xz_setting_set_bool(char *key, bool val);
extern void xz_setting_set_string(char *key, char* val);
#else
#define reset_wifi_configuation() 
#define xz_sys_get_mac()    "112233445566" 
#define set_shutdown_time(a) 
#define xz_setting_get_int(a,b) b
#define xz_setting_get_bool(a,b) b
#define xz_setting_get_string(a,b) b==NULL?NULL:strdup(b)
#define xz_setting_set_int(a,b)
#define xz_setting_set_bool(a,b)
#define xz_setting_set_string(a,b)
#endif


#ifdef __cplusplus
}
#endif

#endif