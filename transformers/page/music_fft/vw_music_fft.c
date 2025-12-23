#include "vw_music_fft.h"
#include "lvgl.h"
#include "cl_ui.h"
#include "lv_round_fft/lv_round_fft.h"
#include "lv_bar_spectrum/lv_bar_spectrum.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#ifndef SIMULATOR
#include "vb_adapter.h"
#endif


LV_IMG_DECLARE(icon_play_40)
LV_IMG_DECLARE(icon_pause_40)
LV_IMG_DECLARE(icon_next_48)
LV_IMG_DECLARE(icon_priv_48)
LV_IMG_DECLARE(icon_more_32)
LV_IMG_DECLARE(icon_exit_32)
LV_IMG_DECLARE(icon_volume_32)

LV_IMG_DECLARE(icon_tf_28);
LV_IMG_DECLARE(icon_bt_28);

#define TAG "vw_music"
/* ---------------- 频谱参数配置 ---------------- */
#define SPEC_BAND_NUM          32          /* 频点数量 */
#define SPEC_TIMER_PERIOD_MS   20          /* 定时器周期，模拟输入刷新频率 */
#define SPEC_BAR_MAX_HEIGHT    120         /* 柱状条最大高度 */
#define SPEC_BAR_MIN_HEIGHT    5           /* 柱状条最小可见高度 */
#define SPEC_PEAK_DECAY_STEP   2           /* 小横条每次下落步长（比柱子变化慢） */
#define SPEC_REFLECT_SCALE     60          /* 倒影高度比例(百分比) */
#define SPEC_REFLECT_OPA       LV_OPA_40   /* 倒影透明度 */

static music_fft_view_t vw = {0};
static lv_obj_t *g_tv = NULL;           // tileview 对象
static lv_obj_t *g_tv_ctrl_0 = NULL;    // ctrl tile 在 (0, 0)
static lv_obj_t *g_tv_ctrl_1 = NULL;    // ctrl tile 在 (0, 1)
static lv_obj_t *g_ctrl_content = NULL; // ctrl 内容对象（在 tile 之间移动）
static lv_obj_t *g_tv_fft_1 = NULL;      // fft1 tile 对象
static lv_obj_t *g_tv_fft_2 = NULL;      // fft2 tile 对象
static uint8_t g_ctrl_row = 0;          // 当前 ctrl 内容所在的 row
static lv_obj_t *g_more_cont = NULL;    // more 容器对象
static lv_obj_t *g_volume_cont = NULL;  // volume 容器对象

static void _on_play_btn(lv_event_t *e){
#ifndef SIMULATOR
    vb_music_mode_t mode = vb_api_get_music_mode();
    uint8_t playing = vb_api_get_play_status();
    lv_obj_t *btn = lv_event_get_target(e);
    lv_obj_t *img = lv_obj_get_child(btn, 0);
    if(playing == 1){
        vb_api_set_music_play(0);
        // if (mode == VB_MUSIC_MODE_TF)
        // {
            lv_img_set_src(img, &icon_play_40);
        // }
    }else{
        vb_api_set_music_play(1);
        // if (mode == VB_MUSIC_MODE_TF)
        // {
            lv_img_set_src(img, &icon_pause_40);
        // }
    }
#endif
}

static void _on_play_prev(lv_event_t *e){
#ifndef SIMULATOR
    vb_api_set_music_next_prev(0);
#endif
}

static void _on_play_next(lv_event_t *e){
#ifndef SIMULATOR
    vb_api_set_music_next_prev(1);
#endif
}

static void _on_btn_cb(lv_event_t *e)
{
    if (!vw.is_act) return;

    cl_button_t *btn = (cl_button_t*)lv_event_get_param(e);
    if (!btn) return;

    switch (btn->id)
    {
        case CL_UI_KEY_MODE:
            if (btn->event == CL_BTN_CLICK)
            {
                page_change("home");
            }
            break;
        default:
            break;
    }
}
#define FFT_VAL_RANGE 300
static void fft_timer_cb(lv_timer_t *timer)
{
    lv_obj_t *fft = lv_timer_get_user_data(timer);

    int16_t val[32];
#ifdef SIMULATOR
    for(int i = 0; i < 32; ++i) {
        val[i] = (rand() % 80); // 5~44 随机值
    }
    lv_bar_spectrum_set_val(vw.spectrum_bar, val);
    for (size_t i = 0; i < 32; i++)
    {
        val[i] = val[i] + FFT_VAL_RANGE; // 5~44 随机值
    }
    
    lv_round_fft_set_val(fft, val);
#else
    cl_ui_get_fft_data(val, sizeof(val));
    lv_bar_spectrum_set_val(vw.spectrum_bar, val);
    for(int i = 0; i < 32; ++i) {
        val[i] = val[i] + FFT_VAL_RANGE; // 5~44 随机值
        // val[i] = 100; // 5~44 随机值
    }
    lv_round_fft_set_val(fft, val);

    
#endif
}


// 圆形频谱
static lv_obj_t *_craete_round_spectrum(lv_obj_t *parent){
    
    LV_IMAGE_DECLARE(img_lv_demo_music_cover_1);
    lv_obj_t *fft = lv_round_fft_create(parent);
    vw.spectrum_round = fft;
    lv_round_fft_set_count(fft, 32);
    
    lv_obj_set_size(fft, 200, 200);
    lv_round_fft_set_val_range(fft, FFT_VAL_RANGE + 80);
    lv_round_fft_set_rotation(fft, 90);
    lv_round_fft_set_colors(fft, lv_color_hex(0xe9dbfc), lv_color_hex(0x6f8af6));
    lv_obj_center(fft);
    
    lv_obj_t *cover = lv_img_create(fft);
    lv_img_set_src(cover, &img_lv_demo_music_cover_1);
    lv_obj_center(cover);

    lv_obj_t *label_lyrc = lv_label_create(parent);
    vw.lyrc_round = label_lyrc;
    lv_obj_set_style_max_width(label_lyrc, 200, 0);
    lv_label_set_long_mode(label_lyrc, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_style_text_font(label_lyrc, cl_ui_get_font(), 0);
    lv_obj_set_style_text_color(label_lyrc, lv_color_hex(0xffffff), 0);
    lv_label_set_text(label_lyrc, "");
    lv_obj_align(label_lyrc, LV_ALIGN_CENTER, 0, 120);

#ifdef SIMULATOR
    lv_label_set_text(label_lyrc, "songs ge ci ge ci");
#endif
    return fft;
}


static lv_obj_t *_create_bar_spectrum(lv_obj_t* parent){
    lv_obj_t *bar_spectrum = lv_bar_spectrum_create(parent);
    vw.spectrum_bar = bar_spectrum;
    lv_obj_set_size(bar_spectrum, 300, 200);
    lv_obj_align(bar_spectrum, LV_ALIGN_CENTER, 0, -20);

    lv_bar_spectrum_set_count(bar_spectrum, SPEC_BAND_NUM);
    lv_bar_spectrum_set_max_height(bar_spectrum, SPEC_BAR_MAX_HEIGHT);
    lv_bar_spectrum_set_min_height(bar_spectrum, SPEC_BAR_MIN_HEIGHT);
    lv_bar_spectrum_set_reflect_scale(bar_spectrum, SPEC_REFLECT_SCALE);
    lv_bar_spectrum_set_reflect_opa(bar_spectrum, SPEC_REFLECT_OPA);
    lv_bar_spectrum_set_peak_decay_step(bar_spectrum, SPEC_PEAK_DECAY_STEP);
    lv_bar_spectrum_set_colors(bar_spectrum, 
                                lv_color_hex(0x2E7DFF),  /* 左侧蓝色 */
                                lv_color_hex(0xFF4081)); /* 右侧粉色 */
                                
    lv_obj_t *label_lyrc = lv_label_create(parent);
    vw.lyrc_bar = label_lyrc;
    lv_obj_set_style_max_width(label_lyrc, 250, 0);
    lv_label_set_long_mode(label_lyrc, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_font(label_lyrc, cl_ui_get_font(), 0);
    lv_obj_set_style_text_color(label_lyrc, lv_color_hex(0xffffff), 0);
    lv_label_set_text(label_lyrc, "");
    lv_obj_align(label_lyrc, LV_ALIGN_CENTER, 0, 80);
#ifdef SIMULATOR
    lv_label_set_text(label_lyrc, "songs ge ci ge ci");
    lv_bar_spectrum_set_auto_update(bar_spectrum,false, SPEC_TIMER_PERIOD_MS);
#endif
    
    return bar_spectrum;
}

static void _ges_cb(lv_event_t *e){
    ESP_LOGI(TAG, "GES CB");
}

// 关闭容器的通用手势回调
static void _cont_gesture_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_GESTURE)
    {
        lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_active());
        if (dir == LV_DIR_RIGHT)
        {
            lv_obj_t *cont = lv_event_get_target(e);
            lv_obj_add_flag(cont, LV_OBJ_FLAG_HIDDEN);
        }
    }
}

static void _cont_exit_btn(lv_event_t *e){
    lv_obj_t *cont = lv_event_get_user_data(e);
    lv_obj_add_flag(cont, LV_OBJ_FLAG_HIDDEN);
}

static void _on_bt_btn_click(lv_event_t *e){
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) { 
#ifndef SIMULATOR
        vb_api_set_music_mode(0);
#else
    lv_obj_t *cur_obj = lv_event_get_target(e);
    lv_obj_t *seg = lv_obj_get_parent(cur_obj);
    lv_obj_add_state(cur_obj, LV_STATE_CHECKED);
    int num = lv_obj_get_child_cnt(seg);
    for (int i = 0; i < num; i++)
    {
        lv_obj_t *btn = lv_obj_get_child(seg, i);
        if (btn && btn != cur_obj)
        {
            lv_obj_remove_state(btn, LV_STATE_CHECKED);
        }
    }
#endif 
    }
}

static void _on_tf_btn_click(lv_event_t *e){
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED) { 
#ifndef SIMULATOR
        vb_api_set_music_mode(1);
#else
        lv_obj_t *cur_obj = lv_event_get_target(e);
        lv_obj_t *seg = lv_obj_get_parent(cur_obj);
        lv_obj_add_state(cur_obj, LV_STATE_CHECKED);
        int num = lv_obj_get_child_cnt(seg);
        for (int i = 0; i < num; i++)
        {
            lv_obj_t *btn = lv_obj_get_child(seg, i);
            if (btn && btn != cur_obj)
            {
                lv_obj_remove_state(btn, LV_STATE_CHECKED);
            }
        }
#endif
    }
}

// 创建 more 容器
static lv_obj_t* _create_more_cont(lv_obj_t *parent)
{
    lv_obj_t *cont = lv_obj_create(parent);
    lv_obj_remove_style_all(cont);
    lv_obj_set_style_bg_color(cont, lv_color_hex(0x0A0A0A), 0);
    lv_obj_set_size(cont, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_opa(cont, LV_OPA_COVER, 0);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
    
    lv_obj_t *seg_cont = lv_obj_create(cont);
    lv_obj_set_size(seg_cont, 180, 80);
    lv_obj_align(seg_cont, LV_ALIGN_TOP_MID, 0, 30);
    lv_obj_set_style_bg_color(seg_cont, lv_color_hex(0x313133), 0);
    lv_obj_set_style_border_width(seg_cont, 0, 0);
    lv_obj_set_style_bg_opa(seg_cont, LV_OPA_COVER, 0);
    lv_obj_clear_flag(seg_cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(seg_cont, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_pad_all(seg_cont, 6, 0);

    lv_obj_t *btn_mode_bt = lv_btn_create(seg_cont);
    lv_obj_set_style_shadow_width(btn_mode_bt, 0, 0);
    lv_obj_set_style_pad_all(btn_mode_bt, 0, 0);
    lv_obj_set_style_radius(btn_mode_bt, LV_RADIUS_CIRCLE, 0);
    lv_obj_align(btn_mode_bt, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_set_size(btn_mode_bt, 80, 70);
    lv_obj_set_style_bg_color(btn_mode_bt, lv_color_hex(0x313133), 0);
    lv_obj_set_style_bg_color(btn_mode_bt, lv_color_hex(0xff5353), LV_PART_MAIN|LV_STATE_CHECKED);
    lv_obj_t *img_bt = lv_img_create(btn_mode_bt);
    lv_obj_align(img_bt, LV_ALIGN_CENTER, 0, -10);
    lv_obj_add_event_cb(btn_mode_bt, _on_bt_btn_click, LV_EVENT_ALL, NULL);
    lv_img_set_src(img_bt, &icon_bt_28);
    lv_obj_t *label = lv_label_create(btn_mode_bt);
    lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_text_font(label, cl_ui_get_font(), 0);
    lv_label_set_text(label, "蓝牙");
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 20);

    
    lv_obj_t *btn_mode_tf = lv_btn_create(seg_cont);
    lv_obj_set_style_shadow_width(btn_mode_tf, 0, 0);
    lv_obj_set_style_pad_all(btn_mode_tf, 0, 0);
    lv_obj_set_style_radius(btn_mode_tf, LV_RADIUS_CIRCLE, 0);
    lv_obj_align(btn_mode_tf, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_set_size(btn_mode_tf, 80, 70);
    lv_obj_set_style_bg_color(btn_mode_tf, lv_color_hex(0x313133), 0);
    lv_obj_set_style_bg_color(btn_mode_tf, lv_color_hex(0xff5353), LV_PART_MAIN|LV_STATE_CHECKED);
    lv_obj_set_style_bg_opa(btn_mode_tf, LV_OPA_0, LV_PART_MAIN|LV_STATE_DISABLED);
    lv_obj_t *img_tf = lv_img_create(btn_mode_tf);
    lv_obj_add_event_cb(btn_mode_tf, _on_tf_btn_click, LV_EVENT_ALL, NULL);
    lv_obj_align(img_tf, LV_ALIGN_CENTER, 0, -10);
    lv_img_set_src(img_tf, &icon_tf_28);
    lv_obj_t *label1 = lv_label_create(btn_mode_tf);
    lv_obj_set_style_text_color(label1, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_text_font(label1, cl_ui_get_font(), 0);
    lv_label_set_text(label1, "SD卡");
    lv_obj_align(label1, LV_ALIGN_CENTER, 0, 20);

    lv_obj_align(seg_cont, LV_ALIGN_CENTER, 0, 0);

    // 创建 exit 按钮，居中屏幕下方
    lv_obj_t *btn_exit = lv_btn_create(cont);
    lv_obj_set_size(btn_exit, 80, 80);
    lv_obj_align(btn_exit, LV_ALIGN_BOTTOM_MID, 0, -20);
    // lv_obj_set_style_bg_color(btn_exit, lv_color_hex(0x333333), 0);
    lv_obj_set_style_bg_opa(btn_exit, LV_OPA_TRANSP, 0);
    lv_obj_set_style_radius(btn_exit, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_shadow_width(btn_exit, 0, 0);
    lv_obj_add_event_cb(btn_exit, _cont_exit_btn, LV_EVENT_SHORT_CLICKED, cont);
    lv_obj_t *label_exit = lv_img_create(btn_exit);
    lv_img_set_src(label_exit, &icon_exit_32);
    lv_obj_center(label_exit);

    vw.btn_mode_tf = btn_mode_tf;
    vw.btn_mode_bt = btn_mode_bt;

    // 注册手势事件，右滑返回
    lv_obj_add_event_cb(cont, _cont_gesture_cb, LV_EVENT_GESTURE, NULL);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_GESTURE_BUBBLE);
    
    // 暂时留白，后续可以添加内容
#ifndef SIMULATOR
    vb_music_mode_t mode = vb_api_get_music_mode();
    if (mode == VB_MUSIC_MODE_TF)
    {
        lv_obj_add_state(btn_mode_tf, LV_STATE_CHECKED);        
    }else if (mode == VB_MUSIC_MODE_BT)
    {
        lv_obj_add_state(btn_mode_bt, LV_STATE_CHECKED);
    }
#else
    lv_obj_add_state(btn_mode_tf, LV_STATE_CHECKED);        
#endif
    return cont;
}

static void _btn_vol_set(lv_event_t *e){
#ifndef SIMULATOR
    uintptr_t dir = (uintptr_t)lv_event_get_user_data(e);
    int cur_vol = (int)vb_audio_get_volume();

    cur_vol += (dir == 1) ? 10 : -10;
    if (cur_vol > 100) cur_vol = 100;
    if (cur_vol < 0) cur_vol = 0;
    vb_audio_set_volume((uint8_t)cur_vol);
#endif
}

// 创建 volume 容器
static lv_obj_t* _create_volume_cont(lv_obj_t *parent)
{
    lv_obj_t *cont = lv_obj_create(parent);
    lv_obj_remove_style_all(cont);
    lv_obj_set_style_bg_color(cont, lv_color_hex(0x0A0A0A), 0);
    lv_obj_set_size(cont, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_opa(cont, LV_OPA_COVER, 0);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
    
    // 注册手势事件，右滑返回
    lv_obj_add_event_cb(cont, _cont_gesture_cb, LV_EVENT_GESTURE, NULL);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_GESTURE_BUBBLE);
    
    // 创建中间的横条进度条
    lv_obj_t *bar = lv_bar_create(cont);
    vw.vol_bar = bar;
    lv_obj_set_size(bar, 180, 10);
    lv_obj_align(bar, LV_ALIGN_CENTER, 0, 0);
    lv_bar_set_range(bar, 0, 100);
    lv_obj_set_style_bg_color(bar, lv_color_hex(0xcccccc), LV_PART_MAIN);
    lv_obj_set_style_bg_color(bar, lv_color_hex(0xff5353), LV_PART_INDICATOR);
    lv_obj_set_style_radius(bar, 10, LV_PART_MAIN);
    lv_obj_set_style_radius(bar, 10, LV_PART_INDICATOR);
    
    // 创建减号按钮（左侧）
    lv_obj_t *btn_minus = lv_btn_create(cont);
    lv_obj_set_size(btn_minus, 60, 60);
    lv_obj_align(btn_minus, LV_ALIGN_CENTER, -130, 0);
    lv_obj_set_style_bg_color(btn_minus, lv_color_hex(0x333333), 0);
    lv_obj_set_style_bg_opa(btn_minus, LV_OPA_TRANSP, 0);
    lv_obj_set_style_radius(btn_minus, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_shadow_width(btn_minus, 0, 0);
    
    LV_IMG_DECLARE(icon_minus_32)
    lv_obj_t *label_minus = lv_img_create(btn_minus);
    lv_img_set_src(label_minus, &icon_minus_32);
    lv_obj_center(label_minus);
    
    // 创建加号按钮（右侧）
    lv_obj_t *btn_plus = lv_btn_create(cont);
    lv_obj_set_size(btn_plus, 60, 60);
    lv_obj_align(btn_plus, LV_ALIGN_CENTER, 130, 0);
    lv_obj_set_style_bg_color(btn_plus, lv_color_hex(0x333333), 0);
    lv_obj_set_style_bg_opa(btn_plus, LV_OPA_TRANSP, 0);
    lv_obj_set_style_radius(btn_plus, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_shadow_width(btn_plus, 0, 0);
    
    LV_IMG_DECLARE(icon_plus_32)
    lv_obj_t *label_plus = lv_img_create(btn_plus);
    lv_img_set_src(label_plus, &icon_plus_32);
    lv_obj_center(label_plus);
    
    // 创建 exit 按钮，居中屏幕下方
    lv_obj_t *btn_exit = lv_btn_create(cont);
    lv_obj_set_size(btn_exit, 80, 80);
    lv_obj_align(btn_exit, LV_ALIGN_BOTTOM_MID, 0, -10);
    lv_obj_set_style_bg_opa(btn_exit, LV_OPA_TRANSP, 0);
    lv_obj_set_style_radius(btn_exit, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_shadow_width(btn_exit, 0, 0);
    lv_obj_t *label_exit = lv_img_create(btn_exit);
    lv_img_set_src(label_exit, &icon_exit_32);
    lv_obj_center(label_exit);
#ifndef SIMULATOR
    uint8_t vol = vb_audio_get_volume();
#else
    uint8_t vol = 50;
#endif
    lv_bar_set_value(bar, vol, LV_ANIM_OFF);
    lv_obj_add_event_cb(btn_minus, _btn_vol_set, LV_EVENT_SHORT_CLICKED, (void*)0);
    lv_obj_add_event_cb(btn_plus, _btn_vol_set, LV_EVENT_SHORT_CLICKED, (void*)1);
    // 如果有编译警告可以用 (void*)(uintptr_t)X，实际两种方式都可，0/1 不访问地址不会出错
    lv_obj_add_event_cb(btn_exit, _cont_exit_btn, LV_EVENT_SHORT_CLICKED, cont);
    return cont;
}

// more 按钮点击事件回调
static void _on_btn_more_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED)
    {
        if (!g_more_cont)
        {
            // 获取根对象（tileview）
            lv_obj_t *root = lv_obj_get_parent(g_tv);
            if (root)
            {
                g_more_cont = _create_more_cont(root);
                lv_obj_add_flag(g_more_cont, LV_OBJ_FLAG_HIDDEN);
            }
        }
        
        if (g_more_cont)
        {
            // 切换显示/隐藏
            if (lv_obj_has_flag(g_more_cont, LV_OBJ_FLAG_HIDDEN))
            {
                lv_obj_clear_flag(g_more_cont, LV_OBJ_FLAG_HIDDEN);
                lv_obj_move_foreground(g_more_cont);
            }
            else
            {
                lv_obj_add_flag(g_more_cont, LV_OBJ_FLAG_HIDDEN);
            }
        }
    }
}

// volume 按钮点击事件回调
static void _on_btn_volume_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_CLICKED)
    {
        if (!g_volume_cont)
        {
            // 获取根对象（tileview）
            lv_obj_t *root = lv_obj_get_parent(g_tv);
            if (root)
            {
                g_volume_cont = _create_volume_cont(root);
                lv_obj_add_flag(g_volume_cont, LV_OBJ_FLAG_HIDDEN);
            }
        }
        
        if (g_volume_cont)
        {
            // 切换显示/隐藏
            if (lv_obj_has_flag(g_volume_cont, LV_OBJ_FLAG_HIDDEN))
            {
                lv_obj_clear_flag(g_volume_cont, LV_OBJ_FLAG_HIDDEN);
                lv_obj_move_foreground(g_volume_cont);
            }
            else
            {
                lv_obj_add_flag(g_volume_cont, LV_OBJ_FLAG_HIDDEN);
            }
        }
    }
}

// 创建控制面板内容（共享函数，确保两个位置的 ctrl 内容一致）
static lv_obj_t* _create_ctrl_content(lv_obj_t *parent)
{
    // 创建一个容器对象作为 ctrl 内容的根，这样可以在 tile 之间移动
    lv_obj_t *content = lv_obj_create(parent);
    lv_obj_remove_style_all(content);
    lv_obj_set_style_bg_color(content, lv_color_hex(0x0A0A0A), 0);
    lv_obj_set_size(content, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_opa(content, LV_OPA_COVER, 0);
    


    lv_obj_t *label = lv_label_create(content);
    // lv_obj_set_style_bg_color(label, lv_color_hex(0xff0000), 0);
    vw.title = label;
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 50);
    lv_obj_set_style_text_font(label, cl_ui_get_font(), 0);
    lv_obj_set_width(label, 140);
    lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), 0);
    // lv_label_set_text(label, "dsfffffffffffffffffffffff");
    lv_label_set_long_mode(label, LV_LABEL_LONG_SCROLL_CIRCULAR);
    // 放大2倍, 并将缩放原点设置为label的中心
    lv_obj_set_style_transform_zoom(label, 400, 0); // 256=2x LVGL zoom
    lv_obj_set_style_transform_pivot_x(label, 70, 0);
    lv_obj_set_style_transform_pivot_y(label, 10, 0);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);

    // Play Button
    lv_obj_t *btn_play = lv_btn_create(content);
    lv_obj_set_size(btn_play, 100, 100);
    lv_obj_align(btn_play, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_shadow_width(btn_play, 0, 0);
    lv_obj_set_style_radius(btn_play, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(btn_play, lv_color_hex(0x333333), 0);
    lv_obj_set_style_bg_opa(btn_play, LV_OPA_40, 0);

    // 创建外圈圆形进度条
    lv_obj_t *arc = lv_arc_create(content);
    vw.time_arc = arc;
    lv_obj_set_size(arc, 113, 113); // 比按钮大约8px
    lv_obj_align_to(arc, btn_play, LV_ALIGN_CENTER, 0, 0);
    // lv_arc_set_range(arc, 0, 100);
    lv_arc_set_bg_angles(arc, 0, 360);   // 整圈
    lv_arc_set_value(arc, 50); // 设置初始进度为0
    lv_arc_set_rotation(arc, -90);
    // lv_obj_remove_style(arc, NULL, LV_PART_KNOB); // 不显示 knob
    lv_obj_set_style_arc_width(arc, 3, LV_PART_MAIN); // 底条宽度
    lv_obj_set_style_arc_color(arc, lv_color_hex(0x333333), LV_PART_MAIN); // 底色
    lv_obj_set_style_arc_width(arc, 3, LV_PART_INDICATOR); // 前景宽度
    lv_obj_set_style_arc_color(arc, lv_color_hex(0xff5353), LV_PART_INDICATOR); // 前景色
    // lv_obj_set_style_bg_opa(arc, LV_OPA_TRANSP, 0);
    lv_obj_remove_style(arc, NULL, LV_PART_KNOB);
    lv_obj_remove_flag(arc, LV_OBJ_FLAG_CLICKABLE);

    // 保证arc在按钮下层显示
    lv_obj_move_background(arc);

    lv_obj_t *icon_play = lv_img_create(btn_play);
    lv_obj_center(icon_play);
    lv_img_set_src(icon_play, &icon_play_40);

    // Next Button
    lv_obj_t *btn_next = lv_btn_create(content);
    lv_obj_set_size(btn_next, 80, 80);
    lv_obj_align(btn_next, LV_ALIGN_CENTER, 120, 0);
    lv_obj_set_style_bg_opa(btn_next, LV_OPA_0, 0);
    lv_obj_set_style_shadow_width(btn_next, 0, 0);

    lv_obj_t *icon_next = lv_img_create(btn_next);
    lv_obj_center(icon_next);
    lv_img_set_src(icon_next, &icon_next_48);

    // Previous Button
    lv_obj_t *btn_priv = lv_btn_create(content);
    lv_obj_set_size(btn_priv, 80, 80);
    lv_obj_align(btn_priv, LV_ALIGN_CENTER, -120, 0);
    lv_obj_set_style_bg_opa(btn_priv, LV_OPA_0, 0);
    lv_obj_set_style_shadow_width(btn_priv, 0, 0);

    lv_obj_t *icon_priv = lv_img_create(btn_priv);
    lv_obj_center(icon_priv);
    lv_img_set_src(icon_priv, &icon_priv_48);

    // More Button
    lv_obj_t *btn_more = lv_btn_create(content);
    lv_obj_set_size(btn_more, 48, 48);
    lv_obj_align(btn_more, LV_ALIGN_CENTER, 50, 100);
    lv_obj_set_style_bg_opa(btn_more, LV_OPA_0, 0);
    lv_obj_set_style_shadow_width(btn_more, 0, 0);

    lv_obj_t *icon_more = lv_img_create(btn_more);
    lv_obj_center(icon_more);
    lv_img_set_src(icon_more, &icon_more_32);

    // Volume Button
    lv_obj_t *btn_volume = lv_btn_create(content);
    lv_obj_set_size(btn_volume, 48, 48);
    lv_obj_align(btn_volume, LV_ALIGN_CENTER, -50, 100); // 位置与more按钮对称放置
    lv_obj_set_style_bg_opa(btn_volume, LV_OPA_0, 0);
    lv_obj_set_style_shadow_width(btn_volume, 0, 0);

    lv_obj_t *icon_volume = lv_img_create(btn_volume);
    lv_obj_center(icon_volume);
    lv_img_set_src(icon_volume, &icon_volume_32); // 你需要有icon_volume_48图片资源

    // 注册按钮事件
    lv_obj_add_event_cb(btn_more, _on_btn_more_cb, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(btn_volume, _on_btn_volume_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_add_event_cb(btn_play, _on_play_btn, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(btn_next, _on_play_next, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(btn_priv, _on_play_prev, LV_EVENT_CLICKED, NULL);

    vw.btn_play = btn_play;
    vw.btn_prev = btn_priv;
    vw.btn_next = btn_next;
    vw.btn_more = btn_more;
    
    lv_obj_add_event_cb(content, _ges_cb, LV_EVENT_GESTURE, NULL);
    lv_obj_clear_flag(content, LV_OBJ_FLAG_GESTURE_BUBBLE);
    // 返回内容对象的根对象，以便后续可以移动
    return content;
}

// 根据当前显示的页面调整 ctrl 的位置
static void _adjust_ctrl_position(void)
{
    // 检查所有必要的对象是否已初始化
    if (!g_tv || !g_tv_ctrl_0 || !g_tv_ctrl_1 || !g_tv_fft_1 || !g_tv_fft_2) return;
    
    // 获取当前活动的 tile
    lv_obj_t *active_tile = lv_tileview_get_tile_active(g_tv);
    if (!active_tile) return;
    
    // 判断当前显示的是哪个页面，确定 ctrl 应该在哪一行
    uint8_t target_row = 0;  // 默认在 row 0
    if (active_tile == g_tv_fft_2) {
        target_row = 1;  // 如果显示的是 fft2，ctrl 应该在 row 1
    } else if (active_tile == g_tv_fft_1) {
        target_row = 0;  // 如果显示的是 fft1，ctrl 应该在 row 0
    } else if (active_tile == g_tv_ctrl_0) {
        // 如果当前就是 ctrl_0，ctrl 应该在 row 0
        target_row = 0;
    } else if (active_tile == g_tv_ctrl_1) {
        // 如果当前就是 ctrl_1，ctrl 应该在 row 1
        target_row = 1;
    }
    
    // 如果位置不同，需要移动 ctrl 内容
    if (g_ctrl_row != target_row) {
        lv_obj_t *target_tile = (target_row == 0) ? g_tv_ctrl_0 : g_tv_ctrl_1;
        
        // 如果 ctrl 内容还没有创建，先创建
        if (!g_ctrl_content) {
            g_ctrl_content = _create_ctrl_content(target_tile);
        } else {
            // 将 ctrl 内容移动到目标 tile
            lv_obj_set_parent(g_ctrl_content, target_tile);
        }
        
        g_ctrl_row = target_row;  // 更新 ctrl 的 row
    }
}

// 将事件代码转换为字符串名称（用于调试）
static const char* _event_code_to_str(lv_event_code_t code)
{
    switch(code) {
        case LV_EVENT_ALL: return "LV_EVENT_ALL";
        case LV_EVENT_PRESSED: return "LV_EVENT_PRESSED";
        case LV_EVENT_PRESSING: return "LV_EVENT_PRESSING";
        case LV_EVENT_PRESS_LOST: return "LV_EVENT_PRESS_LOST";
        case LV_EVENT_SHORT_CLICKED: return "LV_EVENT_SHORT_CLICKED";
        case LV_EVENT_LONG_PRESSED: return "LV_EVENT_LONG_PRESSED";
        case LV_EVENT_LONG_PRESSED_REPEAT: return "LV_EVENT_LONG_PRESSED_REPEAT";
        case LV_EVENT_CLICKED: return "LV_EVENT_CLICKED";
        case LV_EVENT_RELEASED: return "LV_EVENT_RELEASED";
        case LV_EVENT_SCROLL_BEGIN: return "LV_EVENT_SCROLL_BEGIN";
        case LV_EVENT_SCROLL_THROW_BEGIN: return "LV_EVENT_SCROLL_THROW_BEGIN";
        case LV_EVENT_SCROLL_END: return "LV_EVENT_SCROLL_END";
        case LV_EVENT_SCROLL: return "LV_EVENT_SCROLL";
        case LV_EVENT_GESTURE: return "LV_EVENT_GESTURE";
        case LV_EVENT_KEY: return "LV_EVENT_KEY";
        case LV_EVENT_ROTARY: return "LV_EVENT_ROTARY";
        case LV_EVENT_FOCUSED: return "LV_EVENT_FOCUSED";
        case LV_EVENT_DEFOCUSED: return "LV_EVENT_DEFOCUSED";
        case LV_EVENT_LEAVE: return "LV_EVENT_LEAVE";
        case LV_EVENT_HIT_TEST: return "LV_EVENT_HIT_TEST";
        case LV_EVENT_INDEV_RESET: return "LV_EVENT_INDEV_RESET";
        case LV_EVENT_HOVER_OVER: return "LV_EVENT_HOVER_OVER";
        case LV_EVENT_HOVER_LEAVE: return "LV_EVENT_HOVER_LEAVE";
        case LV_EVENT_COVER_CHECK: return "LV_EVENT_COVER_CHECK";
        case LV_EVENT_REFR_EXT_DRAW_SIZE: return "LV_EVENT_REFR_EXT_DRAW_SIZE";
        case LV_EVENT_DRAW_MAIN_BEGIN: return "LV_EVENT_DRAW_MAIN_BEGIN";
        case LV_EVENT_DRAW_MAIN: return "LV_EVENT_DRAW_MAIN";
        case LV_EVENT_DRAW_MAIN_END: return "LV_EVENT_DRAW_MAIN_END";
        case LV_EVENT_DRAW_POST_BEGIN: return "LV_EVENT_DRAW_POST_BEGIN";
        case LV_EVENT_DRAW_POST: return "LV_EVENT_DRAW_POST";
        case LV_EVENT_DRAW_POST_END: return "LV_EVENT_DRAW_POST_END";
        case LV_EVENT_DRAW_TASK_ADDED: return "LV_EVENT_DRAW_TASK_ADDED";
        case LV_EVENT_VALUE_CHANGED: return "LV_EVENT_VALUE_CHANGED";
        case LV_EVENT_INSERT: return "LV_EVENT_INSERT";
        case LV_EVENT_REFRESH: return "LV_EVENT_REFRESH";
        case LV_EVENT_READY: return "LV_EVENT_READY";
        case LV_EVENT_CANCEL: return "LV_EVENT_CANCEL";
        case LV_EVENT_CREATE: return "LV_EVENT_CREATE";
        case LV_EVENT_DELETE: return "LV_EVENT_DELETE";
        case LV_EVENT_CHILD_CHANGED: return "LV_EVENT_CHILD_CHANGED";
        case LV_EVENT_CHILD_CREATED: return "LV_EVENT_CHILD_CREATED";
        case LV_EVENT_CHILD_DELETED: return "LV_EVENT_CHILD_DELETED";
        case LV_EVENT_SCREEN_UNLOAD_START: return "LV_EVENT_SCREEN_UNLOAD_START";
        case LV_EVENT_SCREEN_LOAD_START: return "LV_EVENT_SCREEN_LOAD_START";
        case LV_EVENT_SCREEN_LOADED: return "LV_EVENT_SCREEN_LOADED";
        case LV_EVENT_SCREEN_UNLOADED: return "LV_EVENT_SCREEN_UNLOADED";
        case LV_EVENT_SIZE_CHANGED: return "LV_EVENT_SIZE_CHANGED";
        case LV_EVENT_STYLE_CHANGED: return "LV_EVENT_STYLE_CHANGED";
        case LV_EVENT_LAYOUT_CHANGED: return "LV_EVENT_LAYOUT_CHANGED";
        case LV_EVENT_GET_SELF_SIZE: return "LV_EVENT_GET_SELF_SIZE";
        case LV_EVENT_INVALIDATE_AREA: return "LV_EVENT_INVALIDATE_AREA";
        case LV_EVENT_RESOLUTION_CHANGED: return "LV_EVENT_RESOLUTION_CHANGED";
        case LV_EVENT_COLOR_FORMAT_CHANGED: return "LV_EVENT_COLOR_FORMAT_CHANGED";
        case LV_EVENT_REFR_REQUEST: return "LV_EVENT_REFR_REQUEST";
        case LV_EVENT_REFR_START: return "LV_EVENT_REFR_START";
        case LV_EVENT_REFR_READY: return "LV_EVENT_REFR_READY";
        case LV_EVENT_RENDER_START: return "LV_EVENT_RENDER_START";
        case LV_EVENT_RENDER_READY: return "LV_EVENT_RENDER_READY";
        case LV_EVENT_FLUSH_START: return "LV_EVENT_FLUSH_START";
        case LV_EVENT_FLUSH_FINISH: return "LV_EVENT_FLUSH_FINISH";
        case LV_EVENT_FLUSH_WAIT_START: return "LV_EVENT_FLUSH_WAIT_START";
        case LV_EVENT_FLUSH_WAIT_FINISH: return "LV_EVENT_FLUSH_WAIT_FINISH";
        case LV_EVENT_VSYNC: return "LV_EVENT_VSYNC";
        default: {
            static char unknown[32];
            snprintf(unknown, sizeof(unknown), "UNKNOWN(%d)", code);
            return unknown;
        }
    }
}

// tileview 滑动结束事件回调
static void _tileview_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    // 监听滑动结束和值改变事件，确保能及时调整 ctrl 位置
    if (code == LV_EVENT_SCROLL_END || code == LV_EVENT_VALUE_CHANGED) {
        _adjust_ctrl_position();
    }
    // ESP_LOGI(TAG, "EVENT:%s", _event_code_to_str(code));
}

static void _on_lyrc(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_user_data(e);
    char *str = (char *)lv_event_get_param(e);
    ESP_LOGI(TAG, "%s", str);
    lv_label_set_text(obj, str);
}

static void _on_time(lv_event_t *e){
    
    uint8_t *time8 = lv_event_get_param(e);
    uint32_t time32[2] = {0};
    memcpy(time32, time8, sizeof(time32));
    uint8_t percent = 100;
    if(time32[1] != 0){
        percent = (time32[0]*100/time32[1]);
    }
    lv_arc_set_value(vw.time_arc, percent);
}

static void _on_play_status(lv_event_t *e){
    lv_obj_t *btn_img = lv_obj_get_child(vw.btn_play, 0);
    if (!btn_img)
    {
        return;
    }
    uint32_t *status = lv_event_get_param(e);
    if (status[0] == 0)
    {
        lv_img_set_src(btn_img, &icon_play_40);
    }else{
        lv_img_set_src(btn_img, &icon_pause_40);
    }
}

static void _on_volume(lv_event_t *e){
    uint8_t *pvol = lv_event_get_param(e);
    uint8_t vol = (uint8_t)pvol[0];
    if (vw.vol_bar!=NULL)
    {
        lv_bar_set_value(vw.vol_bar, vol, LV_ANIM_OFF);
    }
    
}

static void _on_mode_change(lv_event_t *e){
    uint32_t *status = lv_event_get_param(e);
    #ifndef SIMULATOR
    if ((vb_music_mode_t)status[0] == VB_MUSIC_MODE_BT)
    {
        lv_obj_add_state(vw.btn_mode_bt, LV_STATE_CHECKED);
        lv_obj_remove_state(vw.btn_mode_tf, LV_STATE_CHECKED);
    }else{
        lv_obj_add_state(vw.btn_mode_tf, LV_STATE_CHECKED);
        lv_obj_remove_state(vw.btn_mode_bt, LV_STATE_CHECKED);
    }
#endif
}

music_fft_view_t* music_fft_view_create(lv_obj_t* root)
{
    lv_obj_remove_style_all(root);
    lv_obj_set_size(root, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_bg_color(root, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(root, LV_OPA_COVER, 0);
    lv_obj_clear_flag(root, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(root, _on_btn_cb, (lv_event_code_t)CL_UI_EVENT_BUTTON, NULL);

    lv_obj_t *tv = lv_tileview_create(root);
    lv_obj_set_size(tv, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_scrollbar_mode(tv, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(tv, lv_color_black(), 0);
    
    // 保存 tileview 引用
    g_tv = tv;

    // 创建两个 ctrl tile（在 (0, 0) 和 (0, 1)），但只在一个中放置内容
    g_tv_ctrl_0 = lv_tileview_add_tile(tv, 0, 0, LV_DIR_RIGHT);
    g_tv_ctrl_1 = lv_tileview_add_tile(tv, 0, 1, LV_DIR_RIGHT);
    
    // 初始时在 (0, 0) 创建 ctrl 内容
    g_ctrl_row = 0;
    g_ctrl_content = _create_ctrl_content(g_tv_ctrl_0);

    
    g_tv_fft_1 = lv_tileview_add_tile(tv, 1, 0, LV_DIR_LEFT | LV_DIR_BOTTOM);
    lv_obj_t *fft = _craete_round_spectrum(g_tv_fft_1);
    g_tv_fft_2 = lv_tileview_add_tile(tv, 1, 1, LV_DIR_LEFT | LV_DIR_TOP);
    _create_bar_spectrum(g_tv_fft_2);

    
    vw.timer = lv_timer_create(fft_timer_cb, 80, fft);
    
    // 在所有 tile 创建完成后再注册事件回调，避免在创建过程中触发
    lv_obj_add_event_cb(tv, _tileview_event_cb, LV_EVENT_SCROLL_END, NULL);
    lv_obj_add_event_cb(tv, _tileview_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(root, _on_lyrc, CL_UI_EVENT_MUSIC_LYRC, vw.lyrc_bar);
    lv_obj_add_event_cb(root, _on_lyrc, CL_UI_EVENT_MUSIC_LYRC, vw.lyrc_round);
    lv_obj_add_event_cb(root, _on_lyrc, CL_UI_EVENT_MUSIC_TITLE, vw.title);
    lv_obj_add_event_cb(root, _on_time, CL_UI_EVENT_MUSIC_TIME, NULL);
    lv_obj_add_event_cb(root, _on_lyrc, CL_UI_EVENT_MUSIC_TITLE, vw.title);
    lv_obj_add_event_cb(root, _on_play_status, CL_UI_EVENT_MUSIC_STATUS, NULL);
    lv_obj_add_event_cb(root, _on_volume, CL_UI_EVENT_MUSIC_VOL, NULL);
    lv_obj_add_event_cb(root, _on_mode_change, CL_UI_EVENT_MODE_CHANGE, NULL);



    return &vw;
}



void music_fft_view_delete(void)
{
    if (vw.timer) {
        lv_timer_del(vw.timer);
        vw.timer = NULL;
    }
    
    if (g_more_cont) {
        lv_obj_del(g_more_cont);
        g_more_cont = NULL;
    }
    
    if (g_volume_cont) {
        lv_obj_del(g_volume_cont);
        g_volume_cont = NULL;
    }
}
