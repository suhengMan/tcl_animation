#include "cl_ui.h"
#include "vw_music_fft.h"
#include "lv_bar_spectrum/lv_bar_spectrum.h"
#include "lv_round_fft/lv_round_fft.h"
#include <string.h>
#ifndef SIMULATOR
#include "esp_lvgl_port.h"
#include "vb_adapter.h"
#endif

#define TAG "sys_music_fft"
LV_IMG_DECLARE(icon_play_40)
LV_IMG_DECLARE(icon_pause_40)

static music_fft_view_t *vw = NULL;
static void request_page(uint32_t play_idx);

// 频谱刷新回调
static void fft_timer_cb(lv_timer_t *timer)
{
    int16_t val[32];
#ifdef SIMULATOR
    for(int i = 0; i < 32; ++i) {
        val[i] = (rand() % 80); // 5~44 随机值
    }
#else
    cl_ui_get_fft_data(val, sizeof(val));
#endif
    lv_bar_spectrum_set_val(vw->spectrum_bar.vw, val);
    lv_round_fft_set_val(vw->spectrum_round.vw, val);
    if (vw->last_play_tick!=0 && lv_tick_get()-vw->last_play_tick>=30*1000)
    {
        page_change("home");
    }
}

//播放状态改变
static void _on_play_status(lv_event_t *e){
    lv_obj_t *btn_img = lv_obj_get_child(vw->ctrl.btn_play, 0);
    if (!btn_img)
    {
        return;
    }
    uint32_t *status = lv_event_get_param(e);
    if (status[0] == 0)
    {
        vw->last_play_tick = lv_tick_get();
        lv_img_set_src(btn_img, &icon_play_40);
    }else{
        vw->last_play_tick = 0;
        lv_img_set_src(btn_img, &icon_pause_40);
    }
}

//歌词回调
static void _on_lyrc(lv_event_t *e)
{
    lv_obj_t *obj = lv_event_get_user_data(e);
    char *str = (char *)lv_event_get_param(e);
    lv_label_set_text(obj, str);
}

//歌词回调
static void _on_title(lv_event_t *e)
{
    char *str = (char *)lv_event_get_param(e);
    char *dash = strrchr(str, '-');
    if (dash) {
        // 左部分: 歌曲名; 右部分: 歌手
        char *left_start = str;
        char *left_end = dash - 1;
        char *right_start = dash + 1;
        char *right_end = str + strlen(str) - 1;
        // 剔除左端空格
        while (left_end > left_start && *left_end == ' ') left_end--;
        // 剔除右端空格
        while (*right_start == ' ' && right_start < right_end) right_start++;
        int left_len = left_end - left_start + 1;
        int right_len = right_end - right_start + 1;
        char left_buf[128] = {0};
        char right_buf[128] = {0};
        if (left_len > 0 && left_len < (int)sizeof(left_buf)) {
            memcpy(left_buf, left_start, left_len);
            left_buf[left_len] = '\0';
        }
        if (right_len > 0 && right_len < (int)sizeof(right_buf)) {
            memcpy(right_buf, right_start, right_len);
            right_buf[right_len] = '\0';
        }
        lv_label_set_text(vw->ctrl.title, left_buf);
        lv_label_set_text(vw->ctrl.singer, right_buf);
    } else {
        lv_label_set_text(vw->ctrl.title, str);
        lv_label_set_text(vw->ctrl.singer, "");
    }
}

//音量回调
static void _on_volume(lv_event_t *e){
    uint8_t *pvol = lv_event_get_param(e);
    uint8_t vol = (uint8_t)pvol[0];
    lv_bar_set_value(vw->cont_volume.bar_volume, vol, LV_ANIM_OFF);
}

//播放进度回调
static void _on_time(lv_event_t *e){
    
    uint8_t *time8 = lv_event_get_param(e);
    uint32_t time32[2] = {0};
    memcpy(time32, time8, sizeof(time32));
    uint8_t percent = 100;
    if(time32[1] != 0){
        percent = (time32[0]*100/time32[1]);
    }
    lv_arc_set_value(vw->ctrl.progress_bar, percent);
}

static void _on_mode_change(lv_event_t *e){
    uint32_t *status = lv_event_get_param(e);
#ifndef SIMULATOR
    if ((vb_music_mode_t)status[0] == VB_MUSIC_MODE_BT)
    {
        lv_obj_add_flag(vw->ctrl.btn_list, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_state(vw->cont_more.btn_bt, LV_STATE_CHECKED);
        lv_obj_remove_state(vw->cont_more.btn_tf, LV_STATE_CHECKED);
        if (!lv_obj_has_flag(vw->list.cont, LV_OBJ_FLAG_HIDDEN))
        {
            lv_obj_add_flag(vw->list.cont, LV_OBJ_FLAG_HIDDEN);
        }
        
    }else{
        lv_obj_remove_flag(vw->ctrl.btn_list, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_state(vw->cont_more.btn_tf, LV_STATE_CHECKED);
        lv_obj_remove_state(vw->cont_more.btn_bt, LV_STATE_CHECKED);
    }
#endif
}


// 关闭容器的通用回调
static void _cont_evt_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_GESTURE)
    {
        lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_active());
        if (dir == LV_DIR_RIGHT)
        {
            lv_indev_wait_release(lv_indev_active());
            lv_obj_t *cont = lv_event_get_target(e);
            lv_obj_add_flag(cont, LV_OBJ_FLAG_HIDDEN);
        }
    }else if (code == LV_EVENT_SHORT_CLICKED)
    {
        lv_obj_t *cont = lv_event_get_user_data(e);
        lv_obj_add_flag(cont, LV_OBJ_FLAG_HIDDEN);
    }
}

//模式按钮
static void _on_mode_btn_cb(lv_event_t *e){
    lv_obj_t *btn = lv_event_get_target(e);
#ifdef SIMULATOR
    lv_obj_add_state(btn, LV_STATE_CHECKED);
    lv_obj_remove_state((btn == vw->cont_more.btn_tf) ? vw->cont_more.btn_bt : vw->cont_more.btn_tf, LV_STATE_CHECKED);
#else
    vb_api_set_music_mode((btn == vw->cont_more.btn_tf) ?1:0);
#endif
}

//音量按钮
static void _on_volume_btn_cb(lv_event_t *e){
    lv_obj_t *btn = lv_event_get_target(e);
#ifndef SIMULATOR
    int cur_vol = vb_audio_get_volume();
    cur_vol += (btn == vw->cont_volume.btn_vol_up)?10:-10;
    if (cur_vol > 100) cur_vol = 100;
    else if (cur_vol < 0) cur_vol = 0;
    vb_audio_set_volume((uint8_t)cur_vol);
#else
    int cur_vol = lv_bar_get_value(vw->cont_volume.bar_volume);
    cur_vol += (btn == vw->cont_volume.btn_vol_up)?10:-10;
    if (cur_vol > 100) cur_vol = 100;
    else if (cur_vol < 0) cur_vol = 0;
    lv_bar_set_value(vw->cont_volume.bar_volume, cur_vol, LV_ANIM_OFF);
#endif

}

//控制按钮
static void on_ctrl_btn_click(lv_event_t *e){
    lv_obj_t *btn = lv_event_get_target(e);
    if (btn == vw->ctrl.btn_play)
    {
        lv_obj_t *img = lv_obj_get_child(btn, 0);
#ifndef SIMULATOR
        uint8_t playing = vb_api_get_play_status();
        vb_api_set_music_play(!playing);
#else
        uint8_t playing = (lv_img_get_src(img) == &icon_pause_40)?1:0;
#endif
        lv_img_set_src(img, playing ? &icon_play_40 : &icon_pause_40);
    }else if (btn == vw->ctrl.btn_prev)
    {
#ifndef SIMULATOR
        vb_api_set_music_next_prev(0);
    }else if (btn == vw->ctrl.btn_next)
    {
        vb_api_set_music_next_prev(1);
#endif
    }else if (btn == vw->ctrl.btn_more)
    {
        if (lv_obj_has_flag(vw->cont_more.cont, LV_OBJ_FLAG_HIDDEN))
        {
            lv_obj_remove_flag(vw->cont_more.cont, LV_OBJ_FLAG_HIDDEN);
        }else{
            lv_obj_add_flag(vw->cont_more.cont, LV_OBJ_FLAG_HIDDEN);
        }
    }else if (btn == vw->ctrl.btn_volume)
    {
        if (lv_obj_has_flag(vw->cont_volume.cont, LV_OBJ_FLAG_HIDDEN))
        {
            lv_obj_remove_flag(vw->cont_volume.cont, LV_OBJ_FLAG_HIDDEN);
        }else{
            lv_obj_add_flag(vw->cont_volume.cont, LV_OBJ_FLAG_HIDDEN);
        }
    }else if (btn == vw->ctrl.btn_list)
    {
        lv_obj_clear_flag(vw->list.cont, LV_OBJ_FLAG_HIDDEN);
        if (vw->list.loaded_start == 0 && vw->list.loaded_end == 0)
        {
#ifndef SIMULATOR
            vw->list.play_index = vb_api_get_play_index();
#endif
            request_page(vw->list.play_index);
        }
    }
    
}

// 根据当前显示的页面调整 ctrl 的位置
static void _adjust_ctrl_position(void)
{
    // 获取当前活动的 tile
    lv_obj_t *active_tile = lv_tileview_get_tile_active(vw->tileview.vw);
    if (!active_tile) return;
    if (active_tile == vw->tileview.tile_fft[0])
    {
        lv_obj_set_parent(vw->ctrl.vw, vw->tileview.tile[0]);
    }else if (active_tile == vw->tileview.tile_fft[1])
    {
        lv_obj_set_parent(vw->ctrl.vw, vw->tileview.tile[1]);
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
}


typedef enum {
    REQ_NONE = 0,
    REQ_BOTTOM,
    REQ_TOP,
} req_dir_t;

#define MUSIC_PAGE_SIZE    10
#define MUSIC_WINDOW_MAX  20


// /* ---------------- 锚点回对齐 ---------------- */
static void scroll_to_anchor(void)
{
    if (vw->list.anchor == UINT32_MAX) return;

    uint32_t cnt = lv_obj_get_child_count(vw->list.cont_list);
    for (uint32_t i = 0; i < cnt; i++) {
        lv_obj_t *c = lv_obj_get_child(vw->list.cont_list, i);
        uint32_t idx = (uint32_t)(uintptr_t)lv_obj_get_user_data(c);
        if (idx == vw->list.anchor) {
            lv_obj_scroll_to_view(c, LV_ANIM_OFF);
            break;
        }
    }
    vw->list.anchor = UINT32_MAX;
}

// /* ---------------- 下拉后的追加逻辑 ---------------- */
static void feed_down(uint32_t start, uint32_t count, const char *names[])
{
    ESP_LOGI(TAG, "feed_down: start=%lu, count=%lu", start, count);
    if (start != vw->list.loaded_end) return;

    for (uint32_t i = 0; i < count; i++)
        vw_music_fft_create_music_item(start + i, names[i], vw->list.play_index);

    vw->list.loaded_end += count;

    /* 超过 10 条 → 删掉顶部 */
    while ((vw->list.loaded_end - vw->list.loaded_start) > MUSIC_WINDOW_MAX) {
        lv_obj_t *first = lv_obj_get_child(vw->list.cont_list, 0);
        lv_obj_del(first);
        vw->list.loaded_start++;
    }

    scroll_to_anchor();
}

// /* ---------------- 上拉后的补回逻辑 ---------------- */
static void feed_up(uint32_t start, uint32_t count, const char *names[])
{
    if (start + count != vw->list.loaded_start) return;

    /* 插入顶部 */
    for (uint32_t i = 0; i < count; i++) {
        lv_obj_t *item = vw_music_fft_create_music_item(start + i, names[i], vw->list.play_index);
        lv_obj_move_to_index(item, i);
    }

    vw->list.loaded_start = start;

    /* 超过 10 条 → 删掉底部 */
    while ((vw->list.loaded_end - vw->list.loaded_start) > MUSIC_WINDOW_MAX) {
        uint32_t c = lv_obj_get_child_count(vw->list.cont_list);
        lv_obj_t *last = lv_obj_get_child(vw->list.cont_list, c - 1);
        lv_obj_del(last);
        vw->list.loaded_end--;
    }
    scroll_to_anchor();
}

static void _list_feed(uint32_t start, uint32_t count, const char *names[], uint32_t total){
    if (!vw->list.in_process || start!=vw->list.req_start)
    {
        ESP_LOGE(TAG, "no req list");
        return;
    }
    if (total > 0) vw->list.total = total;
    if (vw->list.req_dir == REQ_BOTTOM)
    {
        feed_down(start, count, names);
    }else{
        feed_up(start, count, names);
    }
    
    vw->list.in_process = false;
    vw->list.req_dir = REQ_NONE;
}

#ifndef SIMULATOR
static void _get_music_list_cb(uint32_t total, uint32_t index, uint32_t count, const char *list[]){
    lvgl_port_lock(0);
    _list_feed(index, count, list, total);
    lvgl_port_unlock();

}

static void _list_req_post(uint32_t start, uint32_t count)
{
    ESP_LOGI(TAG, "请求数据: start=%lu, count=%lu\n", start, count);
    vb_api_music_list_req(start, count, _get_music_list_cb);
}
#else

static const char *demo_all_names[] = {
    "SONGS 1","SONGS 2","SONGS 3","SONGS 4","SONGS 5",
    "SONGS 6","SONGS 7","SONGS 8","SONGS 9","SONGS 10",
    "SONGS 11","SONGS 12","SONGS 13","SONGS 14","SONGS 15",
    "SONGS 16","SONGS 17","SONGS 18","SONGS 19","SONGS 20",
    "SONGS 21","SONGS 22","SONGS 23","SONGS 24","SONGS 25",
 };

static const uint32_t demo_total = sizeof(demo_all_names)/sizeof(demo_all_names[0]);

static uint32_t s_start = 0;
static uint32_t s_count = 0;
static void onTimer(lv_timer_t* timer)
{
    uint32_t start = s_start;

    uint32_t count = s_count;

    if (start + count > demo_total) {
        count = demo_total - start;
    }

    /* 这里可以是异步：先发 HTTP，等回来再调用 feed。
       为演示方便，直接同步调用 feed。 */
    const char *names[MUSIC_PAGE_SIZE];  // 最大一页5个
    for (uint32_t i = 0; i < count; i++) {
        names[i] = demo_all_names[start + i];
    }
    ESP_LOGI(TAG, "onTimer: start=%lu, count=%lu", start, count);
    _list_feed(start, count, names, demo_total);
}

static void _list_req_post(uint32_t start, uint32_t count)
{
    ESP_LOGI(TAG, "请求数据: start=%lu, count=%lu\n", start, count);

    if (start >= demo_total) {
        _list_feed(start, 0, NULL, demo_total);
        return;
    }

    s_start = start;
    s_count = count;

    lv_timer_t* timer = lv_timer_create(onTimer, 100, (void *)start);
    lv_timer_set_repeat_count(timer, 1);
}
#endif
// /* ---------------- 请求下一页 ---------------- */
static void request_down(void)
{
    uint32_t tick =  lv_tick_get();
    if (tick - vw->list.req_tick > 1000 && vw->list.in_process){
        vw->list.in_process = false;
    }
   
    if (vw->list.in_process) return;
    if (vw->list.total > 0 && vw->list.loaded_end >= vw->list.total) return;

    /* ✔ 锚点：倒数第二行 */
    if (vw->list.loaded_end > vw->list.loaded_start) {
        uint32_t win_cnt = vw->list.loaded_end - vw->list.loaded_start;
        uint32_t anchor_pos = (win_cnt >= 2) ? win_cnt - 2 : win_cnt - 1;
        vw->list.anchor = vw->list.loaded_start + anchor_pos;
    }

    uint32_t start = vw->list.loaded_end;
    uint32_t cnt = MUSIC_PAGE_SIZE;
    if (vw->list.total > 0 && start + cnt > vw->list.total)
        cnt = vw->list.total - start;

    vw->list.in_process = true;
    vw->list.req_dir = REQ_BOTTOM;
    vw->list.req_start = start;
    vw->list.req_cont = cnt;

    _list_req_post(start, cnt);
}
// /* ---------------- 请求上一页 ---------------- */
static void request_up(void)
{
    if (vw->list.in_process || vw->list.loaded_start == 0) return;

    /* ✔锚点：第二行（若有），避免看到标题 */
    if (vw->list.loaded_end > vw->list.loaded_start) {
        uint32_t win_cnt = vw->list.loaded_end - vw->list.loaded_start;
        uint32_t anchor_pos = (win_cnt >= 2) ? 1 : 0;
        vw->list.anchor = vw->list.loaded_start + anchor_pos;
    }

    uint32_t cnt = MUSIC_PAGE_SIZE;
    if (vw->list.loaded_start < cnt) cnt = vw->list.loaded_start;

    uint32_t start = vw->list.loaded_start - cnt;

    vw->list.in_process = true;
    vw->list.req_dir = REQ_TOP;
    vw->list.req_start = start;
    vw->list.req_cont = cnt;
    _list_req_post(start, cnt);
}
//请求播放页
static void request_page(uint32_t play_idx)
{
    uint32_t tick =  lv_tick_get();
    if (tick - vw->list.req_tick > 1000 && vw->list.in_process){
        vw->list.in_process = false;
    }
    if (vw->list.in_process) return;
    if (vw->list.total > 0 && vw->list.loaded_end >= vw->list.total) return;

    vw->list.anchor = play_idx;

    if (vw->list.total>0 && vw->list.loaded_start<= play_idx && vw->list.loaded_end >= play_idx)
    {
        scroll_to_anchor();
        return;
    }
        
    uint32_t start = play_idx;
    uint32_t cnt = MUSIC_PAGE_SIZE;
    if (vw->list.total > 0 && start + cnt > vw->list.total)
        cnt = vw->list.total - start;

    vw->list.loaded_start = start;
    vw->list.loaded_end = start;
    vw->list.in_process = true;
    vw->list.req_dir = REQ_BOTTOM;
    vw->list.req_start = start;
    vw->list.req_cont = cnt;
    ESP_LOGI(TAG, "request_page: start=%lu, cnt=%lu", start, cnt);
    _list_req_post(start, cnt);
}

// 音乐列表滑动回调
static void _list_scroll_cb(lv_event_t *e)
{
    lv_obj_t *list = lv_event_get_target(e);
    if (lv_obj_get_scroll_bottom(list) < 10){
        request_down();
    }else if (lv_obj_get_scroll_top(list) < 10)
    {
        request_up();
    }      
}

static void _on_music_idx(lv_event_t *e){
    uint8_t *inx_p = lv_event_get_param(e);
    uint32_t index = 0;
    memcpy(&index, inx_p, sizeof(uint32_t));
    vw->list.play_index = index;
    ESP_LOGI(TAG, "INDEX:%ld", index);
#ifndef SIMULATOR
    if (!lv_obj_has_flag(vw->list.cont, LV_OBJ_FLAG_HIDDEN))
    {
        if (index>=vw->list.loaded_start && index <= vw->list.loaded_end)
        {
            for (uint32_t i = 0; i < vw->list.loaded_end-vw->list.loaded_start; i++)
            {
                lv_obj_t *item = lv_obj_get_child(vw->list.cont_list, i);
                if (!item)
                {
                    break;
                }
                
                if (i+vw->list.loaded_start == index && !lv_obj_has_state(item, LV_STATE_CHECKED))
                {
                    lv_obj_add_state(item, LV_STATE_CHECKED);
                }else if (i+vw->list.loaded_start != index && lv_obj_has_state(item, LV_STATE_CHECKED))
                {
                    lv_obj_remove_state(item, LV_STATE_CHECKED);
                }
            }
            
        }
          
    }
#endif
}

static void _on_btn_cb(lv_event_t *e)
{
    // if (!vw->is_act) return;

    cl_button_t *btn = (cl_button_t*)lv_event_get_param(e);
    if (!btn) return;
    switch (btn->id)
    {
        case CL_UI_KEY_MODE:
            if (btn->event == CL_BTN_CLICK)
            {
#ifndef SIMULATOR
                vb_music_mode_t mode = vb_api_get_music_mode();
                vb_api_set_music_mode(mode==VB_MUSIC_MODE_BT?VB_MUSIC_MODE_TF:VB_MUSIC_MODE_BT);
#else
                // page_change("home");
#endif
            }
            break;
        default:
            break;
    }
}

void sys_music_fft_state_init(music_fft_view_t *view, lv_obj_t *root){
    lv_label_set_text(vw->ctrl.title, "");
    lv_label_set_text(vw->ctrl.singer, "");
#ifndef SIMULATOR
    int vol = (int)vb_audio_get_volume();
    lv_bar_set_value(vw->cont_volume.bar_volume, vol, LV_ANIM_OFF);

    vb_music_mode_t mode = vb_api_get_music_mode();
    if (mode == VB_MUSIC_MODE_TF)
    {
        lv_obj_add_state(vw->cont_more.btn_tf, LV_STATE_CHECKED);
        lv_obj_remove_state(vw->cont_more.btn_bt, LV_STATE_CHECKED);
    }else{
        lv_obj_add_flag(vw->ctrl.btn_list, LV_OBJ_FLAG_HIDDEN);
        lv_obj_add_state(vw->cont_more.btn_bt, LV_STATE_CHECKED);
        lv_obj_remove_state(vw->cont_more.btn_tf, LV_STATE_CHECKED);
    }

    lv_label_set_text(vw->spectrum_bar.lyrc, "");
    lv_label_set_text(vw->spectrum_round.lyrc, "");

    uint8_t play_status = vb_api_get_play_status();
    lv_obj_t *btn_img = lv_obj_get_child(vw->ctrl.btn_play, 0);
    lv_img_set_src(btn_img, play_status==1?&icon_pause_40:&icon_play_40);
    vw->last_play_tick = play_status==1?0:lv_tick_get();
#else
    lv_label_set_text(vw->ctrl.title, "男孩");
    lv_label_set_text(vw->ctrl.singer, "梁博");
#endif    
}

static void _on_root_ges(lv_event_t *e){
    lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_active());
    if (dir == LV_DIR_RIGHT)
    {
        lv_indev_wait_release(lv_indev_active());
        page_change("home");
    }

} 

void sys_music_fft_event_init(music_fft_view_t *view, lv_obj_t *root){
   
    vw = view;  
    lv_obj_add_event_cb(root, _on_lyrc, CL_UI_EVENT_MUSIC_LYRC, vw->spectrum_bar.lyrc);
    lv_obj_add_event_cb(root, _on_lyrc, CL_UI_EVENT_MUSIC_LYRC, vw->spectrum_round.lyrc);
    lv_obj_add_event_cb(root, _on_title, CL_UI_EVENT_MUSIC_TITLE, vw->ctrl.title);
    lv_obj_add_event_cb(root, _on_time, CL_UI_EVENT_MUSIC_TIME, NULL);
    lv_obj_add_event_cb(root, _on_play_status, CL_UI_EVENT_MUSIC_STATUS, NULL);
    lv_obj_add_event_cb(root, _on_volume, CL_UI_EVENT_MUSIC_VOL, NULL);
    lv_obj_add_event_cb(root, _on_mode_change, CL_UI_EVENT_MODE_CHANGE, NULL);
    lv_obj_add_event_cb(root, _on_music_idx, CL_UI_EVENT_MUSIC_IDX, NULL);

    // tileview 
    lv_obj_add_event_cb(vw->tileview.vw, _tileview_event_cb, LV_EVENT_SCROLL_END, NULL);
    lv_obj_add_event_cb(vw->tileview.vw, _tileview_event_cb, LV_EVENT_VALUE_CHANGED, NULL);

    // 注册按钮事件
    lv_obj_add_event_cb(vw->ctrl.btn_more, on_ctrl_btn_click, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(vw->ctrl.btn_volume, on_ctrl_btn_click, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(vw->ctrl.btn_play, on_ctrl_btn_click, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(vw->ctrl.btn_next, on_ctrl_btn_click, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(vw->ctrl.btn_prev, on_ctrl_btn_click, LV_EVENT_CLICKED, NULL);
    lv_obj_add_event_cb(vw->ctrl.btn_list, on_ctrl_btn_click, LV_EVENT_CLICKED, NULL);

    lv_obj_add_event_cb(vw->cont_more.btn_bt, _on_mode_btn_cb, LV_EVENT_SHORT_CLICKED, NULL);
    lv_obj_add_event_cb(vw->cont_more.btn_tf, _on_mode_btn_cb, LV_EVENT_SHORT_CLICKED, NULL);
    lv_obj_add_event_cb(vw->cont_more.btn_exit, _cont_evt_cb, LV_EVENT_SHORT_CLICKED, vw->cont_more.cont);
    lv_obj_add_event_cb(vw->cont_more.cont, _cont_evt_cb, LV_EVENT_GESTURE, vw->cont_more.cont);
    lv_obj_remove_flag(vw->cont_more.cont, LV_OBJ_FLAG_GESTURE_BUBBLE);
    
    
    lv_obj_add_event_cb(vw->cont_volume.btn_vol_up, _on_volume_btn_cb, LV_EVENT_SHORT_CLICKED, NULL);
    lv_obj_add_event_cb(vw->cont_volume.btn_vol_down, _on_volume_btn_cb, LV_EVENT_SHORT_CLICKED, NULL);
    lv_obj_add_event_cb(vw->cont_volume.btn_exit, _cont_evt_cb, LV_EVENT_SHORT_CLICKED, vw->cont_volume.cont);
    lv_obj_add_event_cb(vw->cont_volume.cont, _cont_evt_cb, LV_EVENT_GESTURE, vw->cont_volume.cont);
    lv_obj_remove_flag(vw->cont_volume.cont, LV_OBJ_FLAG_GESTURE_BUBBLE);

    lv_obj_add_event_cb(vw->ctrl.vw, _on_root_ges, LV_EVENT_GESTURE, NULL);
    lv_obj_clear_flag(vw->ctrl.vw, LV_OBJ_FLAG_GESTURE_BUBBLE);

    lv_obj_add_event_cb(vw->list.cont_list, _list_scroll_cb, LV_EVENT_SCROLL, NULL);
    lv_obj_add_event_cb(vw->list.cont, _cont_evt_cb, LV_EVENT_GESTURE, vw->list.cont);
    lv_obj_remove_flag(vw->list.cont, LV_OBJ_FLAG_GESTURE_BUBBLE);

    lv_obj_add_event_cb(root, _on_btn_cb, (lv_event_code_t)CL_UI_EVENT_BUTTON, NULL);
    vw->timer = lv_timer_create(fft_timer_cb, 80, NULL);
}

void sys_music_fft_uninit(){
    if (vw->timer)
    {
        lv_timer_delete(vw->timer);
        vw->timer = NULL;
    }
    
}