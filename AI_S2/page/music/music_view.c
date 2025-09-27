#include "music_view.h"
#include "cl_ui.h"
#ifndef SIMULATOR
#include "voice_music.h"
#include "key_event_deal.h"
#include "os/os_api.h"
#include "tone_player.h"
#include "classic/tws_api.h"
#include "btstack/avctp_user.h"
#include "audio_dec_file.h"
#include "app_task.h"
#endif

LV_FONT_DECLARE(font_puhui_18_4)

LV_IMG_DECLARE(img_vinyl_red);       
LV_IMG_DECLARE(icon_music_one);          
LV_IMG_DECLARE(icon_priv_32);
LV_IMG_DECLARE(icon_pause_32);
LV_IMG_DECLARE(icon_play_32);
LV_IMG_DECLARE(icon_next_32);

LV_IMG_DECLARE(icon_loop_32);
LV_IMG_DECLARE(icon_loop1_32);
LV_IMG_DECLARE(icon_rand_32);
LV_IMG_DECLARE(icon_music_list_24);

LV_IMG_DECLARE(icon_tf_16);
LV_IMG_DECLARE(icon_bt_16);
LV_IMG_DECLARE(icon_bright_16);
LV_IMG_DECLARE(icon_vol_16);

LV_IMG_DECLARE(icon_music_cy);
LV_IMG_DECLARE(icon_music_rand);
LV_IMG_DECLARE(icon_music_bt);
LV_IMG_DECLARE(icon_music_sd);

#define TAG "music_view"

static music_view_t vw = {0};

extern int  file_scan_init(char* logo, int* file_number);
extern unsigned char* file_scan_get_name(int num);
static lv_obj_t*music_menu_creat(lv_obj_t* root, const char* name, int index);

static int _map(int in, int in_min, int in_max, int out_min, int out_max) {
    return (in_max == in_min) ? 0 : (in - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

static void _on_btn_cb(lv_event_t *e){
    
    if (!vw.is_act)
    {
        return;
    }

    cl_button_t *btn = (cl_button_t*)lv_event_get_param(e);
    if (!btn) return;
    switch (btn->id)
    {
    case USER_BUTTON_UP:
        
        break;
    case USER_BUTTON_CENTER:
        if (btn->event == BTN_CLICK)
        {
            page_change("home");
        }
        break;
    case USER_BUTTON_DOWN:
        
        break;
    
    default:
        break;
    }
    
}
#ifndef SIMULATOR
static void _on_mode_change_cb(lv_event_t *e){
    u8 mode = (u8)lv_event_get_param(e);
    lv_obj_t *btn_img = lv_obj_get_child(vw.play_btn, 0);
    u8 is_play = 0;
    switch (mode)
    {
    case APP_BT_TASK:
        lv_obj_add_flag(vw.loop_btn, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(vw.list_btn, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(vw.progress_bar, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_state(lv_obj_get_child(vw.menu.seg_btn, 0), LV_STATE_CHECKED);
        lv_obj_clear_state(lv_obj_get_child(vw.menu.seg_btn, 1), LV_STATE_CHECKED);
        is_play = bt_a2dp_status_check();

        break;
    case APP_MUSIC_TASK:
        lv_obj_clear_flag(vw.loop_btn, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(vw.list_btn, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(vw.progress_bar, LV_OBJ_FLAG_CLICKABLE);
        lv_obj_add_state(lv_obj_get_child(vw.menu.seg_btn, 1), LV_STATE_CHECKED);
        lv_obj_clear_state(lv_obj_get_child(vw.menu.seg_btn, 0), LV_STATE_CHECKED);
        is_play = music_player_get_play_status()==1;    
        break;
    default:
        break;
    }

    if (is_play)
    {
        lv_img_set_src(btn_img, &icon_pause_32);
    }else{
        lv_img_set_src(btn_img, &icon_play_32);
    }
}
#endif

char* get_time_str(int time) {
    char time_str[16] = { 0 };
    uint8_t  min, sec;
    min = time / 1000 / 60;
    sec = time / 1000 - (min * 60);

    sprintf(time_str, "%02d:%02d", min, sec);
    printf(">>>>>>>>>>>>>>>>>>>  %d   %d   %d  %s",time,min,sec,time_str);
    return time_str;
}
#ifndef SIMULATOR
extern void music_lyrics_loop(void);
int last_total_time = 0;
int last_play_time = 0;
void _get_playing_info(lv_timer_t* timer) {

    if (!vw.is_act)
    {
        return;
    }
        
    if (query_mode() == USB_MODE || query_mode() == TF_CARD_MODE) {
        music_lyrics_loop();
    } else {

    }
    if (vw.progress_lock == true) {
        return;
    }
    uint8_t  min, sec;
    uint8_t percent = 0;
    if (vw.total_time == 0)
    {
        percent = 0;
    }else{
        percent = vw.curr_time * 100 / vw.total_time;
    }
    
    if (last_play_time != vw.curr_time)
    {
        min = vw.curr_time / 1000 / 60;
        sec = vw.curr_time / 1000 - (min * 60);
        lv_label_set_text_fmt(vw.time_curr, "%02d:%02d", min, sec);
        last_play_time = vw.curr_time;
    }

    if (last_total_time != vw.total_time)
    {
        min = vw.total_time / 1000 / 60;
        sec = vw.total_time / 1000 - (min * 60);
        lv_label_set_text_fmt(vw.time_total, "%02d:%02d", min, sec);
        last_total_time = vw.total_time;
    }
    
    if (lv_bar_get_value(vw.progress_bar) != percent)
    {
        lv_bar_set_value(vw.progress_bar, percent, LV_ANIM_ON);
    }
}
#endif

static void _on_music_title_cb(lv_event_t *e){
    
    if (!vw.is_act)
    {
        return;
    }
    const char* title = (char*)lv_event_get_param(e);
    if (vw.title_label==NULL)
    {
        return;
    }
    lv_label_set_text(vw.title_label, title);

}


void _on_music_lyrc_cb(lv_event_t *e){
    if (vw.lyrc_label==NULL)
    {
        return;
    }
    const char* lyrc = (char*)lv_event_get_param(e);
    char* p_lyrc = lv_label_get_text(vw.lyrc_label);
    if (strcmp(p_lyrc, lyrc) == 0)
    {
        return; // 如果歌词没有变化，则不更新
    }
    lv_label_set_text(vw.lyrc_label, lyrc);
}

void _on_music_time_cb(lv_event_t *e) {
    if (vw.progress_bar==NULL)
    {
        return;
    }
    u32 time = (u32)lv_event_get_param(e);
    u32 now = timer_get_ms();
    if (now - vw.updata_time > 100)
    {
        vw.total_time = time;
    } else {
        vw.curr_time = time;
    }
    vw.updata_time = now;
}

void _on_music_play_status_cb(lv_event_t *e) {
    if (vw.play_btn==NULL)
    {
        return;
    }
    u32 play = (u32)lv_event_get_param(e);
    lv_obj_t *img = lv_obj_get_child(vw.play_btn, 0);
    if (play) {
        lv_img_set_src(img, &icon_pause_32);
    } else {
        lv_img_set_src(img, &icon_play_32);
    }
}

static void _on_bright_set(lv_event_t *e){
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t *slider = lv_event_get_target(e);
        int value = lv_slider_get_value(slider);
        xz_hal_set_bl(value);
    }
}

static void _on_vol_set(lv_event_t *e){
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t *slider = lv_event_get_target(e);
        int value = lv_slider_get_value(slider);
        if (value >=90) {
            value = 10;
        } else if (value < 10) {
            value = 1;
        } else {
           value = value / 10;
        }
        xz_hal_set_vol(value);
    }
}
/*
#define    FCYCLE_ALL			1
#define    FCYCLE_ONE			2
#define    FCYCLE_RANDOM		4
*/
static void _on_loop_btn(lv_event_t *e){
    if (!vw.is_act)
    {
        return;
    }
    lv_obj_t *btn_img = (lv_obj_t*)lv_event_get_user_data(e);
    int mode = xz_hal_get_loop_mode();
    if (mode == 1) {
        mode = 2;
    } else if (mode == 2) {
        mode = 4;
    } else {
        mode = 1;
    }
    xz_hal_set_loop_mode(mode);
    switch (mode)
    {
        case 2:   lv_img_set_src(btn_img, &icon_loop1_32); break;
        case 4:   lv_img_set_src(btn_img, &icon_rand_32); break;
        default:   lv_img_set_src(btn_img, &icon_loop_32); break;
    }
}

static void _on_list_btn(lv_event_t *e){
    if (!vw.is_act)
    {
        return;
    }
    page_change("music_list");
}

static void _on_play_btn(lv_event_t *e){
    xz_hal_music_togle_play();
}

static void _on_prev_btn(lv_event_t *e){
    xz_hal_music_priv();
}

static void _on_next_btn(lv_event_t *e){
    xz_hal_music_next();    
}


void music_view_del(){
#ifndef SIMULATOR
    if (vw.get_info_timer)
    {
        lv_timer_del(vw.get_info_timer);
    }
#endif
}

static void slider_event_cb(lv_event_t *e)
{
    if (vw.progress_bar == NULL)
    {
        return;
    }
    lv_obj_t* slider = lv_event_get_target(e);
    lv_event_code_t code = lv_event_get_code(e);
    int val = lv_slider_get_value(vw.progress_bar);
    int32_t max = lv_slider_get_max_value(slider);
    int32_t current_time = (val * vw.total_time) / max;
    int32_t ff_time = 0;
    u8 ff_flage;
    if (current_time > vw.curr_time) {
        ff_time = current_time - vw.curr_time; //快进
        ff_flage = 1;
    } else {
        ff_flage = 0;
        ff_time = vw.curr_time - current_time; //快退
    }
    if (code == LV_EVENT_PRESSED) {
        vw.progress_lock = true;
    } else if (code == LV_EVENT_RELEASED) {
        if (ff_flage) {
            xz_hal_music_ff(ff_time);
        } else {
            xz_hal_music_fr(ff_time);
        }
        vw.progress_lock = false;
    } else {
        lv_label_set_text(vw.time_curr, get_time_str(current_time));
    }
}



static void _on_bt_btn_cb(lv_event_t *e){
    lv_obj_t *cont = (lv_obj_t *)lv_event_get_user_data(e);
    if (!cont) return;
    lv_obj_t *btn = lv_event_get_target(e);
    if (!btn) return;
    if (!lv_obj_has_state(btn, LV_STATE_CHECKED))
    {
        lv_obj_add_state(btn, LV_STATE_CHECKED);
        lv_obj_t *btn_tf = lv_obj_get_child(cont, 1);
        if (lv_obj_has_state(btn_tf, LV_STATE_CHECKED))
        {
            lv_obj_clear_state(btn_tf, LV_STATE_CHECKED);
        }
    }
    if (xz_hal_music_get_mode() != BLUETOOTH_MODE) {
        xz_hal_music_set_mode(BLUETOOTH_MODE);
        lv_label_set_text(vw.title_label,"暂无歌曲");
        lv_label_set_text(vw.lyrc_label, "");
        lv_obj_add_flag(vw.progress_bar, LV_OBJ_FLAG_CLICKABLE);
    }
}

static void _on_tf_btn_cb(lv_event_t *e){
    lv_obj_t *cont = (lv_obj_t *)lv_event_get_user_data(e);
    if (!cont) return;
    lv_obj_t *btn = lv_event_get_target(e);
    if (!btn) return;
    if (!lv_obj_has_state(btn, LV_STATE_CHECKED))
    {
        lv_obj_add_state(btn, LV_STATE_CHECKED);
        lv_obj_t *btn_bt = lv_obj_get_child(cont, 0);
        if (lv_obj_has_state(btn_bt, LV_STATE_CHECKED))
        {
            lv_obj_clear_state(btn_bt, LV_STATE_CHECKED);
        }
    }
    if (xz_hal_music_get_mode() != TF_CARD_MODE) {
        xz_hal_music_set_mode(TF_CARD_MODE);
        lv_label_set_text(vw.title_label,"暂无歌曲");
        lv_label_set_text(vw.lyrc_label, "");
        lv_obj_add_flag(vw.progress_bar, LV_OBJ_FLAG_CLICKABLE);
    }
}

static lv_obj_t *lv_seg_btn_create(lv_obj_t *parent){
    lv_obj_t *cont = lv_obj_create(parent);
    lv_obj_set_size(cont, 140, 60);
    lv_obj_set_style_bg_color(cont, lv_color_hex(0x313133), 0);
    lv_obj_set_style_border_width(cont, 0, 0);
    lv_obj_set_style_bg_opa(cont, LV_OPA_COVER, 0);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_radius(cont, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_pad_all(cont, 3, 0);

    lv_obj_t *btn_mode_bt = lv_btn_create(cont);
    lv_obj_set_style_shadow_width(btn_mode_bt, 0, 0);
    lv_obj_set_style_pad_all(btn_mode_bt, 0, 0);
    lv_obj_set_style_radius(btn_mode_bt, LV_RADIUS_CIRCLE, 0);
    lv_obj_align(btn_mode_bt, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_set_size(btn_mode_bt, 65, 55);
    lv_obj_set_style_bg_color(btn_mode_bt, lv_color_hex(0x313133), 0);
    lv_obj_set_style_bg_color(btn_mode_bt, lv_color_hex(0x5c5c5c), LV_PART_MAIN|LV_STATE_CHECKED);
    lv_obj_t *img_bt = lv_img_create(btn_mode_bt);
    lv_obj_center(img_bt);
    lv_img_set_src(img_bt, &icon_bt_16);
    
    lv_obj_t *btn_mode_tf = lv_btn_create(cont);
    lv_obj_set_style_shadow_width(btn_mode_tf, 0, 0);
    lv_obj_set_style_pad_all(btn_mode_tf, 0, 0);
    lv_obj_set_style_radius(btn_mode_tf, LV_RADIUS_CIRCLE, 0);
    lv_obj_align(btn_mode_tf, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_set_size(btn_mode_tf, 65, 55);
    lv_obj_set_style_bg_color(btn_mode_tf, lv_color_hex(0x313133), 0);
    lv_obj_set_style_bg_color(btn_mode_tf, lv_color_hex(0x5c5c5c), LV_PART_MAIN|LV_STATE_CHECKED);
    lv_obj_t *img_tf = lv_img_create(btn_mode_tf);
    lv_obj_center(img_tf);
    lv_img_set_src(img_tf, &icon_tf_16);

    if (xz_hal_music_get_mode() == BLUETOOTH_MODE)
    {
        lv_obj_add_state(btn_mode_bt, LV_STATE_CHECKED);
        lv_obj_clear_state(btn_mode_tf, LV_STATE_CHECKED);
    }else{
        lv_obj_add_state(btn_mode_tf, LV_STATE_CHECKED);
        lv_obj_clear_state(btn_mode_bt, LV_STATE_CHECKED);
    }
    lv_obj_add_event_cb(btn_mode_bt, _on_bt_btn_cb, LV_EVENT_CLICKED, cont);
    lv_obj_add_event_cb(btn_mode_tf, _on_tf_btn_cb, LV_EVENT_CLICKED, cont);

    return cont;
}

static void _menu_anim_close_cb(lv_anim_t *a){
    lv_obj_set_width(vw.menu.trig_bar, 35);
    lv_obj_add_flag(vw.menu.cont_bg, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_style_bg_opa(vw.menu.cont, LV_OPA_0, 0);
}

/* 统一的 y 动画 */
static void menu_anim_to_y(lv_obj_t *obj, lv_coord_t to_y, uint32_t time_ms, lv_anim_completed_cb_t cb)
{
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, obj);
    lv_anim_set_values(&a, lv_obj_get_y(obj), to_y);
    lv_anim_set_time(&a, time_ms);
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)lv_obj_set_y);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
    if (cb)
    {
        lv_anim_set_completed_cb(&a, cb);
    }
    lv_anim_start(&a);
}

/* 展开/收起两个便捷函数 */
static void menu_open(lv_obj_t *obj, menu_drag_ctx_t *ctx)  { menu_anim_to_y(obj, ctx->open_y,   100, NULL); }
static void menu_close(lv_obj_t *obj, menu_drag_ctx_t *ctx) { menu_anim_to_y(obj, ctx->closed_y, 100, _menu_anim_close_cb); }

/* 事件回调：实现只允许竖直拖拽 + 吸附 */
static void menu_drag_event_cb(lv_event_t *e)
{
    menu_drag_ctx_t *ctx = (menu_drag_ctx_t *)lv_event_get_user_data(e);
    lv_obj_t *obj = ctx->cont;
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_PRESSED) {
        lv_obj_set_width(ctx->trig_bar, 48);   // 按下时，扩大拖拽手柄的宽度，方便拖拽
        lv_obj_clear_flag(ctx->cont_bg, LV_OBJ_FLAG_HIDDEN);
        lv_obj_set_style_bg_opa(ctx->cont, LV_OPA_COVER, 0);
        lv_point_t p; lv_indev_t *indev = lv_indev_get_act();
        lv_indev_get_point(indev, &p);
        ctx->press_y     = p.y;
        ctx->obj_start_y = lv_obj_get_y(obj);
    }
    else if(code == LV_EVENT_PRESSING) {
        lv_point_t p; lv_indev_t *indev = lv_indev_get_act();
        lv_indev_get_point(indev, &p);

        lv_coord_t dy    = p.y - ctx->press_y;          // 只看 Y，忽略 X -> 仅上下拖
        lv_coord_t new_y = ctx->obj_start_y + dy;
        
        printf("dy=%d, new_y=%d    ", dy, new_y);
        /* 夹在 open_y 与 closed_y 之间，避免越界 */
        if(new_y < ctx->open_y)   new_y = ctx->open_y;
        if(new_y > ctx->closed_y) new_y = ctx->closed_y;

        printf("dy=%d, new_y=%d\r\n", dy, new_y);
        lv_obj_set_y(obj, new_y);
    }
    else if(code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST) {
        lv_point_t p; lv_indev_t *indev = lv_indev_get_act();
        lv_indev_get_point(indev, &p);
        lv_coord_t diff = p.y - ctx->press_y;;

        if (ctx->obj_start_y == 180 )
        {
            if (diff >= 10)
            {
                menu_close(obj, ctx);             // 向下拖超过 10px -> 吸附收起
                return;
            }
            else
            {
                menu_open(obj, ctx);              // 否则 -> 吸附展开
                return;
            }
            
        }else{
            if (diff <= -20)
            {
                menu_open(obj, ctx);              // 向上拖超过 10px -> 吸附展开
                return;
            }
            else
            {
                menu_close(obj, ctx);             // 向下拖超过 10px -> 吸附收起
                return;
            }
            
        }
    }
}

static void _on_bg_mask_click(lv_event_t *e){
    if (vw.menu.cont == NULL)
    {
        return;
    }
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_RELEASED)
    {
        menu_close(vw.menu.cont, &vw.menu);
    }
    
}

static lv_obj_t* _create_menu_cont(lv_obj_t *parent){
    lv_obj_t *bg_mask = lv_obj_create(parent);
    lv_obj_set_size(bg_mask, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_bg_color(bg_mask, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(bg_mask, LV_OPA_50, 0);
    lv_obj_set_style_border_width(bg_mask, 0, 0);
    lv_obj_clear_flag(bg_mask, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(bg_mask, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_event_cb(bg_mask, _on_bg_mask_click, LV_EVENT_ALL, NULL);

    lv_obj_t *cont = lv_obj_create(parent);
    lv_obj_set_size(cont, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_bg_color(cont, lv_color_hex(0x222222), 0);
    lv_obj_set_style_radius(cont, 40, 0);
    lv_obj_set_style_border_width(cont, 0, 0);
    lv_obj_set_style_bg_opa(cont, LV_OPA_0, 0);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_y(cont, 280);
    lv_obj_set_x(cont, 0);
    
    lv_obj_t *bar_brightness = lv_slider_create(cont);
    lv_obj_set_style_bg_opa(bar_brightness, 0, LV_PART_KNOB);
    lv_obj_set_style_radius(bar_brightness, 0, LV_PART_INDICATOR);
    lv_obj_set_size(bar_brightness, 200, 40);
    lv_obj_align(bar_brightness, LV_ALIGN_TOP_MID, 0, 30);
    lv_obj_t *img_bright = lv_img_create(bar_brightness);
    lv_img_set_src(img_bright, &icon_bright_16);
    lv_obj_align(img_bright, LV_ALIGN_LEFT_MID, 16, 0);
    lv_bar_set_value(bar_brightness, 50, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(bar_brightness, lv_color_hex(0x4A505A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(bar_brightness, lv_color_hex(0xFFFFFF), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(bar_brightness, 100, LV_PART_MAIN | LV_STATE_DEFAULT);
    u8 bl = xz_hal_get_bl();
    lv_slider_set_value(bar_brightness, bl, LV_ANIM_OFF);
    lv_obj_add_event_cb(bar_brightness, _on_bright_set, LV_EVENT_VALUE_CHANGED, NULL);

    lv_obj_t *bar_volume = lv_slider_create(cont);
    lv_obj_set_style_bg_opa(bar_volume, 0, LV_PART_KNOB);
    lv_obj_set_style_radius(bar_volume, 0, LV_PART_INDICATOR);
    lv_obj_add_flag(bar_volume, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_size(bar_volume, 200, 40);
    lv_obj_align_to(bar_volume, bar_brightness,LV_ALIGN_OUT_BOTTOM_MID, 0, 20);
    lv_obj_t *img_volume = lv_img_create(bar_volume);
    lv_img_set_src(img_volume, &icon_vol_16);
    lv_obj_align(img_volume, LV_ALIGN_LEFT_MID, 16, 0);
    lv_obj_set_style_bg_color(bar_volume, lv_color_hex(0x4A505A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(bar_volume, lv_color_hex(0xFFFFFF), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(bar_volume, 100, LV_PART_MAIN | LV_STATE_DEFAULT);
    int volume = xz_hal_get_vol();
    lv_slider_set_value(bar_volume, volume, LV_ANIM_OFF);
    lv_obj_add_event_cb(bar_volume, _on_vol_set, LV_EVENT_VALUE_CHANGED, NULL);
    
    lv_obj_t *handle = lv_obj_create(cont);
    lv_obj_set_size(handle, 48, 6);
    lv_obj_set_style_radius(handle, 3, 0);
    lv_obj_set_style_border_width(handle, 0, 0);
    lv_obj_set_style_bg_color(handle, lv_color_hex(0x5A5A5A), 0);
    lv_obj_set_style_bg_opa(handle, LV_OPA_60, 0);
    lv_obj_align(handle, LV_ALIGN_TOP_MID, 0, -8);
    lv_obj_add_flag(handle, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(handle, LV_OBJ_FLAG_SCROLLABLE);


    lv_obj_t *seg_btn = lv_seg_btn_create(cont);
    lv_obj_align(seg_btn, LV_ALIGN_BOTTOM_MID, 0, -30);

    vw.menu.cont_bg       = bg_mask;
    vw.menu.cont       = cont;
    vw.menu.seg_btn       = seg_btn;
    vw.menu.open_y   = 0;
    vw.menu.closed_y = 280;    
    vw.menu.trig_bar = handle;
    // 只在手柄上响应拖拽：把同一回调也挂到手柄即可
    lv_obj_add_flag(cont, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(cont, menu_drag_event_cb, LV_EVENT_ALL, &vw.menu);
    lv_obj_add_event_cb(handle, menu_drag_event_cb, LV_EVENT_ALL, &vw.menu);

    return cont;
}

music_view_t* music_view_create(lv_obj_t* root) {

    lv_obj_remove_style_all(root);
    lv_obj_set_size(root, LV_HOR_RES, LV_VER_RES);     // 背景白色
    lv_obj_set_style_bg_color(root, lv_color_hex(0x000000), 0);
    lv_obj_set_style_border_width(root, 0, 0);
    lv_obj_set_style_bg_opa(root, LV_OPA_COVER, 0);
    lv_obj_clear_flag(root, LV_OBJ_FLAG_SCROLLABLE);
   
    lv_obj_t *title = lv_label_create(root);
    vw.title_label = title;
    lv_obj_set_width(title, 120);
    lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_long_mode(title, LV_LABEL_LONG_DOT);
    lv_obj_set_style_text_font(title, &font_puhui_18_4, 0);
    lv_label_set_text(title, "暂无歌曲");
    lv_obj_set_style_text_color(title, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 30);

    lv_obj_t *img_vinyl = lv_img_create(root);
    vw.bg_img = img_vinyl;
    lv_obj_set_style_border_width(img_vinyl, 0, 0);
    lv_img_set_src(img_vinyl, ASSERT_PREXI"/icon/music/img_vinyl_red.png");
    lv_obj_align(img_vinyl, LV_ALIGN_CENTER, 0, -10);

    lv_obj_t *lyrc = lv_label_create(root);
    vw.lyrc_label = lyrc;
    lv_obj_set_width(lyrc, 200);
    lv_obj_set_style_text_align(lyrc, LV_TEXT_ALIGN_CENTER, 0);
    lv_label_set_long_mode(lyrc, LV_LABEL_LONG_SCROLL_ONCE);
    lv_obj_set_style_text_color(lyrc, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_text_font(lyrc, &font_puhui_18_4, 0);
    lv_label_set_text(lyrc, " ");
    lv_obj_align_to(lyrc, img_vinyl, LV_ALIGN_OUT_BOTTOM_MID, 0, 2);
    lv_obj_clear_flag(lyrc, LV_OBJ_FLAG_PRESS_LOCK);
    
    
    lv_obj_t* progress_bar = lv_slider_create(root);
    vw.progress_bar = progress_bar;
    lv_obj_set_size(progress_bar, 200,32); 
    lv_obj_align_to(progress_bar, lyrc, LV_ALIGN_OUT_BOTTOM_MID, 0, -6);
    lv_slider_set_range(progress_bar, 0, 100);
    lv_obj_set_style_bg_color(progress_bar, lv_color_hex(0xCCCCCC), LV_PART_MAIN);         // 背景色
    lv_obj_set_style_bg_color(progress_bar, lv_color_hex(0xFFFFFF), LV_PART_INDICATOR);     // 已滑动区域
    lv_obj_set_style_border_width(progress_bar, 14, LV_PART_MAIN);
    lv_obj_set_style_border_side(progress_bar, LV_BORDER_SIDE_BOTTOM | LV_BORDER_SIDE_TOP, 0);
    lv_obj_set_style_pad_top(progress_bar, 19, LV_PART_MAIN);
    lv_obj_set_style_pad_bottom(progress_bar, 19, LV_PART_MAIN);
    lv_obj_set_style_opa(progress_bar, LV_OPA_0, LV_PART_KNOB);
    lv_obj_clear_flag(progress_bar, LV_OBJ_FLAG_GESTURE_BUBBLE); 
    lv_obj_move_background(progress_bar);
    int percent = 0;
    if (vw.total_time != 0)
    {
        percent = vw.curr_time*100/vw.total_time;
    }
    lv_slider_set_value(progress_bar, percent, LV_ANIM_OFF);
    

    lv_obj_t *time_curr = lv_label_create(root);
    vw.time_curr = time_curr;
    lv_obj_set_style_text_color(time_curr, lv_color_hex(0xFFFFFF), 0);
    lv_label_set_text_fmt(time_curr, "%02d:%02d", vw.curr_time/1000/60, (vw.curr_time/1000) % 60);
    lv_obj_align_to(time_curr, progress_bar, LV_ALIGN_OUT_BOTTOM_LEFT, 6, -8);
    
    lv_obj_t *time_total = lv_label_create(root);
    vw.time_total = time_total;
    lv_obj_set_style_text_color(time_total, lv_color_hex(0xFFFFFF), 0);
    lv_label_set_text_fmt(time_total, "%02d:%02d", vw.total_time/1000/60, (vw.total_time/1000) % 60);
    lv_obj_align_to(time_total, progress_bar, LV_ALIGN_OUT_BOTTOM_RIGHT, -6, -8);


    lv_obj_t *btn_play = lv_btn_create(root);
    vw.play_btn = btn_play;
    lv_obj_set_size(btn_play, 50, 50);
    lv_obj_set_style_radius(btn_play, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_opa(btn_play, LV_OPA_TRANSP, 0);
    lv_obj_set_style_shadow_width(btn_play, 0, 0);
    lv_obj_align_to(btn_play, img_vinyl, LV_ALIGN_CENTER, 0, 0);
    lv_obj_t *btn_img_play = lv_img_create(btn_play);
    lv_img_set_src(btn_img_play, &icon_play_32);
    lv_obj_center(btn_img_play);


    lv_obj_t *btn_prev = lv_btn_create(root);
    vw.priv_btn = btn_prev;
    lv_obj_set_size(btn_prev, 50, 50);
    lv_obj_set_style_radius(btn_prev, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_opa(btn_prev, LV_OPA_TRANSP, 0);
    lv_obj_set_style_shadow_width(btn_prev, 0, 0);
    lv_obj_align_to(btn_prev, btn_play, LV_ALIGN_OUT_LEFT_MID, -15, 0);
    lv_obj_t *btn_img_prev = lv_img_create(btn_prev);
    lv_img_set_src(btn_img_prev, &icon_priv_32);
    lv_obj_center(btn_img_prev);

    lv_obj_t *btn_next = lv_btn_create(root);
    vw.next_btn = btn_next;
    lv_obj_set_size(btn_next, 50, 50);
    lv_obj_set_style_radius(btn_next, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_opa(btn_next, LV_OPA_TRANSP, 0);
    lv_obj_set_style_shadow_width(btn_next, 0, 0);
    lv_obj_align_to(btn_next, btn_play, LV_ALIGN_OUT_RIGHT_MID, 15, 0);
    lv_obj_t *btn_img_next = lv_img_create(btn_next);
    lv_img_set_src(btn_img_next, &icon_next_32);
    lv_obj_center(btn_img_next);

    lv_obj_t *btn_loop = lv_btn_create(root);
    vw.loop_btn = btn_loop;
    lv_obj_set_size(btn_loop, 50, 50);
    lv_obj_set_style_radius(btn_loop, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_opa(btn_loop, LV_OPA_TRANSP, 0);
    lv_obj_set_style_shadow_width(btn_loop, 0, 0);
    lv_obj_align_to(btn_loop, btn_play, LV_ALIGN_OUT_TOP_MID, 0, -15);
    lv_obj_t *btn_img_loop = lv_img_create(btn_loop);
    lv_img_set_src(btn_img_loop, &icon_loop_32);
    lv_obj_center(btn_img_loop);

    lv_obj_t *btn_list = lv_btn_create(root);
    vw.list_btn = btn_list;
    lv_obj_set_size(btn_list, 50, 50);
    lv_obj_set_style_radius(btn_list, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_opa(btn_list, LV_OPA_TRANSP, 0);
    lv_obj_set_style_shadow_width(btn_list, 0, 0);
    lv_obj_align_to(btn_list, btn_play, LV_ALIGN_OUT_BOTTOM_MID, 0, 15);
    lv_obj_t *btn_img_list = lv_img_create(btn_list);
    lv_img_set_src(btn_img_list, &icon_music_list_24);
    lv_obj_center(btn_img_next);

    _create_menu_cont(root);
    if (xz_hal_music_get_mode() == BLUETOOTH_MODE)
    {
        lv_obj_add_flag(btn_list, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_flag(btn_loop, LV_OBJ_FLAG_HIDDEN);
        lv_obj_clear_flag(vw.progress_bar, LV_OBJ_FLAG_CLICKABLE);
    }else{
        switch (xz_hal_get_loop_mode()){
            case 2:   lv_img_set_src(btn_img_loop, &icon_loop1_32); break;
            case 4:   lv_img_set_src(btn_img_loop, &icon_rand_32); break;
        }
        lv_obj_add_flag(vw.progress_bar, LV_OBJ_FLAG_CLICKABLE);
        lv_label_set_text(vw.title_label, xz_hal_get_music_name()); 
        lv_slider_set_value(vw.progress_bar, 0, LV_ANIM_OFF);
    }


    if (xz_hal_music_status())
    {
        lv_img_set_src(btn_img_play, &icon_pause_32);
    }else{
        lv_img_set_src(btn_img_play, &icon_play_32);
    }

    lv_obj_add_event_cb(btn_list, _on_list_btn, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(btn_loop, _on_loop_btn, LV_EVENT_CLICKED, btn_img_loop);

    lv_obj_add_event_cb(progress_bar, slider_event_cb, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(progress_bar, slider_event_cb, LV_EVENT_PRESSED, NULL);
    lv_obj_add_event_cb(progress_bar, slider_event_cb, LV_EVENT_RELEASED, NULL);

    lv_obj_add_event_cb(btn_play, _on_play_btn, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(btn_prev, _on_prev_btn, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(btn_next, _on_next_btn, LV_EVENT_CLICKED, NULL);
    // lv_obj_add_event_cb(btn_mode, _on_mode_btn, LV_EVENT_CLICKED, NULL);

#ifndef SIMULATOR
    vw.get_info_timer = lv_timer_create(_get_playing_info, 200, NULL);
    lv_obj_add_event_cb(root, _on_mode_change_cb, CL_UI_EVENT_MODE_CHANGE, NULL);
#endif
    lv_obj_add_event_cb(root, _on_btn_cb, CL_UI_EVENT_BTN, NULL);
    lv_obj_add_event_cb(root, _on_music_title_cb, CL_UI_EVENT_MUSIC_TITLE, NULL);
    lv_obj_add_event_cb(root, _on_music_lyrc_cb, CL_UI_EVENT_MUSIC_LYRC, NULL);
    lv_obj_add_event_cb(root, _on_music_time_cb, CL_UI_EVENT_MUSIC_TIME, NULL);
    lv_obj_add_event_cb(root, _on_music_play_status_cb, CL_UI_EVENT_MUSIC_PLAY_STATUS, NULL);
    // lv_obj_add_event_cb(root, _on_gesture_cb, LV_EVENT_GESTURE, NULL);
    
    return &vw;
}


