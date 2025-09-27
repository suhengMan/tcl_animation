#include "flexible_button.h"
#include "cl_ui.h"
#include "xz_btn.h"

#define TAG "xiaozhi_btn"


static flex_button_t user_button[USER_BUTTON_MAX];
static xiaozhi_btn_cb s_btn_cb = NULL;


static user_button_t btn_list[] = {
    USER_BUTTON_UP,
    USER_BUTTON_CENTER,
    USER_BUTTON_DOWN,
};


static uint8_t common_btn_read(void *arg)
{
    flex_button_t *btn = (flex_button_t *)arg;
    if (sdl_port_event_get_current_key() == btn_list[btn->id] && sdl_port_event_get_current_key_state())
    {
        return 0;
    }
    return 1;
    // return button_scan(btn_list[btn->id]);
}

static void common_btn_evt_cb(void *arg)
{
    flex_button_t *btn = (flex_button_t *)arg;
    static cl_button_t cl_btn = {0};
    cl_btn.id = btn_list[btn->id];
    cl_btn.click_cnt = btn->click_cnt;
    cl_btn.scan_cnt = btn->scan_cnt;
    cl_btn.event = btn->event;
    // LOGD("btn id:%d, event:%d, click_cnt:%d\n", btn->id, btn->event, btn->click_cnt);
    if (s_btn_cb)
    {
        s_btn_cb(&cl_btn);
    }
}

void xiaozhi_btn_init(xiaozhi_btn_cb cb)
{
    s_btn_cb = cb;
    for (u8 i = 0; i < sizeof(btn_list)/sizeof(btn_list[0]); i++)
    {       
        user_button[i].id = i;
        user_button[i].usr_button_read = common_btn_read;
        user_button[i].cb = common_btn_evt_cb;
        user_button[i].pressed_logic_level = 0;
        user_button[i].short_press_start_tick = FLEX_MS_TO_SCAN_CNT(800);
        user_button[i].long_press_start_tick = FLEX_MS_TO_SCAN_CNT(1100);
        user_button[i].long_hold_start_tick = FLEX_MS_TO_SCAN_CNT(1600);
        flex_button_register(&user_button[i]);
    }    
}