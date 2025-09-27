#ifndef _CL_UI_H_
#define _CL_UI_H_

#include "AI_S2/assert/language.h"
#include "utils/btn/xz_btn.h"
#include "hal/hal_port.h"
#include "page_manager.h"

#ifndef u8
#define u8 uint8_t
#define u16 uint16_t
#define u32 uint32_t
#endif

#define ASSERT_PREXI "P:/home/arzhe/Proj/baji/AI_S2/assert"
#define SIMULATOR 1

#define LOGD(format, ...) printf("\033[0;36m" "["TAG"]" format "\033[0m\n", ##__VA_ARGS__)
#define LOGI(format, ...) printf("\033[0;32m" "["TAG"]" format "\033[0m\n", ##__VA_ARGS__)
#define LOGW(format, ...) printf("\033[0;33m" "["TAG"]" format "\033[0m\n", ##__VA_ARGS__)
#define LOGE(format, ...) printf("\033[0;31m" "["TAG"]" format "\033[0m\n", ##__VA_ARGS__)

typedef enum{
    CL_UI_EVENT_SYSTEM = LV_EVENT_LAST + 1,
    CL_UI_EVENT_BTN,
    CL_UI_EVENT_BAT_CHANGE,
    CL_UI_EVENT_TIME_CHANGE,
    CL_UI_EVENT_VOLUME_CHANGE,
    CL_UI_EVENT_MODE_CHANGE,
    CL_UI_EVENT_CHAT_MSG,
    CL_UI_EVENT_CHAT_STATUS,
    CL_UI_EVENT_CHAT_EMOJI,
    CL_UI_EVENT_MUSIC_TITLE,
    CL_UI_EVENT_MUSIC_LYRC,
    CL_UI_EVENT_MUSIC_TIME,
    CL_UI_EVENT_WEATHER,
    CL_UI_EVENT_MUSIC_CHANGE,
    CL_UI_EVENT_MUSIC_PLAY_STATUS,
    CL_UI_GLOBAL_EVT_DOWNLOAD_JPG,
    CL_UI_GLOBAL_EVT_DOWNLOAD_JPGS,
    CL_UI_GLOBAL_EVT_DEL_JPG,
    CL_UI_GLOBAL_EVT_DOWNLOAD_VPG,
}cl_ui_event_t;

typedef enum{
    CL_MSG_PAGE_CHANGE,
    CL_MSG_LV_EVENT,
    CL_MSG_CALL_BACK,
    CL_MSG_OTHER,
}cl_ui_msg_t;

typedef enum{
    CL_UI_STA_IDLE,
    CL_UI_STA_CONNECTTING,
    CL_UI_STA_LISTENING,
    CL_UI_STA_SPEAKING,
    CL_UI_STA_NETWORK_DISCONNECT,
    CL_UI_STA_BT_DISCONNECT,
    CL_UI_STA_BT_CONNECTED,
    CL_UI_STA_MAX,
}cl_ui_status_code_t;


typedef enum{
    CL_OPRET_SUCCESS = 0,   //下载成功
    CL_OPRET_FAIL = 1,      //下载失败，可能下一半网络断了，也可能是APP发的数据有问题，或者设备内存不够了
    CL_OPRET_BUSY = 2,      //设备忙，可能正在下载其他资源
    CL_OPRET_NET_ERR = 3,   //设备未联网
    CL_OPRET_BT_ERR = 4,    //设备蓝牙未连接
}cl_operation_code_t;

typedef struct
{
    u16 year;
    u8 month;
    u8 day;
    u8 wday;
    u8 hour;
    u8 min;
    u8 sec;
}cl_time_t;

typedef enum{
    CTRL_CENTER_PRESS,
    CTRL_CENTER_Y_CHANGE,
    CTRL_CENTER_RELEASE,
    CTRL_CENTER_OPEN,
    CTRL_CENTER_CLOSED,
}ctrl_center_evt_t;

typedef void (*ctrl_center_cb_t)(ctrl_center_evt_t code, void* arg);
lv_obj_t *ctrl_center_create(lv_obj_t *parent, ctrl_center_cb_t ctrl_evt_cb);
cl_time_t cl_ui_get_time(void);
int page_change(const char* name);
#endif