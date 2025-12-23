#ifndef SIMULATOR
#include "gpio.h"
#include "app_config.h"
#include "app_task.h"
#include "app_main.h"
#include "xiaozhi.h"
#endif
#include "cl_ui.h"
#include "flexible_button.h"
#include "xiaozhi_btn.h"

#define TAG "btn"

#ifdef SIMULATOR
#include "lvgl/src/drivers/sdl/lv_sdl_private.h"

static void button_keyboard_read(lv_indev_t * indev, lv_indev_data_t * data);
static uint8_t last_key_state[4] = {0}; // 存储上、左、下、右的按键状态
static const SDL_Keycode key_map[4] = {SDLK_LEFT, SDLK_RIGHT, SDLK_UP, SDLK_DOWN}; // 对应上、左、下、右
static const cl_ui_key_t button_map[4] = {CL_UI_KEY_POWER, CL_UI_KEY_MODE, CL_UI_KEY_VOL_UP, CL_UI_KEY_VOL_DOWN}; // 映射到按钮


/**
 * 自定义键盘输入设备读取函数
 * 检测方向键并映射为按钮事件
 */
static void button_keyboard_read(lv_indev_t * indev, lv_indev_data_t * data)
{
    (void)indev; /*Unused*/
    // LOGW("button_keyboard_read");
    // 更新 SDL 事件状态（确保键盘状态是最新的）
    SDL_PumpEvents();
    // LOGW("button_keyboard_read");
    
    // 获取 SDL 键盘状态
    const Uint8 *keyboard_state = SDL_GetKeyboardState(NULL);
    // LOGW("button_keyboard_read");
    
    // 检查每个方向键
    for (int i = 0; i < 4; i++) {
        SDL_Scancode scancode = SDL_GetScancodeFromKey(key_map[i]);
        uint8_t current_state = keyboard_state[scancode] ? 1 : 0;
        // 更新按键状态
        last_key_state[i] = current_state;
    }
    
    // 设置输入数据（这里不需要实际的按键数据，因为我们直接发送事件）
    data->state = LV_INDEV_STATE_RELEASED;
    data->key = 0;
}

#endif


static flex_button_t user_button[CL_UI_KEY_VOL_MAX];


static void button_scan(lv_timer_t *t)
{
    int32_t tick = lv_tick_get();
    static int32_t last_tick = 0;
    int32_t delta_tick = tick - last_tick;
    if (last_tick == 0)
    {
        delta_tick = 20;
    }
    last_tick = tick;
    flex_button_scan(delta_tick);
}

static u8 common_btn_read(void *arg)
{
    flex_button_t *btn = (flex_button_t *)arg;
#ifndef SIMULATOR
    return gpio_read(btn_list[btn->id]);
#else
    SDL_PumpEvents();
    const Uint8 *keyboard_state = SDL_GetKeyboardState(NULL);
    SDL_Scancode scancode = SDL_GetScancodeFromKey(key_map[btn->id]);
    uint8_t current_state = keyboard_state[scancode] ? 1 : 0;
    return current_state;
#endif
}


static void common_btn_evt_cb(void *arg)
{
    flex_button_t *btn = (flex_button_t *)arg;
    static cl_button_t cl_btn = {0};
    cl_btn.id = btn->id;
    cl_btn.click_cnt = btn->click_cnt;
    cl_btn.scan_cnt = btn->scan_cnt;
    cl_btn.event = btn->event;
    // LOGD("btn id:%d, event:%d, click_cnt:%d\n", btn->id, btn->event, btn->click_cnt);
    xz_ui_evt_send(CL_UI_EVENT_BTN, &cl_btn, sizeof(cl_button_t));
}

void xiaozhi_btn_init(void)
{
    for (u8 i = 0; i < sizeof(button_map)/sizeof(button_map[0]); i++)
    {
        user_button[i].id = i;
        user_button[i].usr_button_read = common_btn_read;
        user_button[i].cb = common_btn_evt_cb;
        user_button[i].pressed_logic_level = 1;
        user_button[i].short_press_start_tick = FLEX_MS_TO_SCAN_CNT(800);
        user_button[i].long_press_start_tick = FLEX_MS_TO_SCAN_CNT(1100);
        user_button[i].long_hold_start_tick = FLEX_MS_TO_SCAN_CNT(1600);
        flex_button_register(&user_button[i]);
    }

    lv_timer_create(button_scan, 20, NULL);
    
}