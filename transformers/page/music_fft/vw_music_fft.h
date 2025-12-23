#ifndef MUSIC_FFT_VIEW_H
#define MUSIC_FFT_VIEW_H
#include "lvgl.h"

typedef struct
{
    bool is_act;
    lv_obj_t *time_arc;
    lv_obj_t *spectrum_bar;
    lv_obj_t *spectrum_round;
    lv_obj_t *title;
    lv_obj_t *lyrc_bar;
    lv_obj_t *lyrc_round;

    lv_obj_t *btn_mode_tf;
    lv_obj_t *btn_mode_bt;

    lv_obj_t *btn_play;
    lv_obj_t *btn_prev;
    lv_obj_t *btn_next;
    lv_obj_t *btn_more;

    lv_obj_t *vol_bar;

    lv_timer_t *timer;
}music_fft_view_t;

music_fft_view_t* music_fft_view_create(lv_obj_t* root);
void music_fft_view_delete(void);
void music_fft_resume(void);
void music_fft_pause(void);
void music_fft_set_data(const int16_t * data);  // 设置32个频点的数据

#endif

