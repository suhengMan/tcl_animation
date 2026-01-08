#ifndef MUSIC_FFT_VIEW_H
#define MUSIC_FFT_VIEW_H
#include "lvgl.h"

typedef struct
{
    lv_obj_t *vw;
    lv_obj_t *tile[2];
    lv_obj_t *tile_fft[2];
    lv_obj_t *cont_ctrl;
    uint8_t active_row;
}vw_music_fft_tile_t;

typedef struct
{
    lv_obj_t *vw;
    lv_obj_t *img;
    lv_obj_t *lyrc;
}vw_music_fft_spectrum_t;

typedef struct
{
    lv_obj_t *vw;
    lv_obj_t *title;//标题
    lv_obj_t *singer;//标题
    lv_obj_t *lyrc;//歌词
    lv_obj_t *progress_bar; //进度条
    lv_obj_t *btn_play; //播放按钮
    lv_obj_t *btn_prev; //上一首按钮
    lv_obj_t *btn_next; //下一首按钮
    lv_obj_t *btn_more; //更多按钮
    lv_obj_t *btn_list; //列表按钮
    lv_obj_t *btn_volume; //音量按钮

}vw_music_fft_ctrl_t;

typedef struct
{
    lv_obj_t *cont;
    lv_obj_t *btn_tf;
    lv_obj_t *btn_bt;
    lv_obj_t *btn_exit;
}vw_music_fft_more_cont_t;

typedef struct 
{
    lv_obj_t *cont;
    lv_obj_t *cont_list;
    uint32_t req_tick;
    uint32_t play_index;
    int total;
    int loaded_start;
    int loaded_end;
    int req_start;
    int req_cont;
    int anchor;
    int req_dir;
    bool in_process;
}vw_music_fft_list_t;


typedef struct
{
    uint8_t is_act;
    vw_music_fft_tile_t tileview;               //tileview
    vw_music_fft_spectrum_t spectrum_bar;       //频谱条形
    vw_music_fft_spectrum_t spectrum_round;     //频谱圆形
    vw_music_fft_ctrl_t ctrl;                   //控制cont
    vw_music_fft_more_cont_t cont_more;         //模式cont
    vw_music_fft_list_t list;
    uint32_t last_play_tick;
    lv_timer_t *timer;
    lv_timer_t *timer_check_state;
}music_fft_view_t;

music_fft_view_t* music_fft_view_create(lv_obj_t* root);
lv_obj_t *vw_music_fft_create_music_item(uint32_t index, const char *name, uint32_t play_idx);

void music_fft_view_delete(void);
void sys_music_fft_event_init(music_fft_view_t *view, lv_obj_t *root);
void sys_music_fft_state_init(music_fft_view_t *view, lv_obj_t *root);
void sys_music_fft_uninit();

#endif

