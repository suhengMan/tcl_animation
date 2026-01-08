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
LV_IMG_DECLARE(icon_list_32)
LV_IMAGE_DECLARE(img_lv_demo_music_cover_1);

LV_IMG_DECLARE(icon_tf_28);
LV_IMG_DECLARE(icon_bt_28);

#define TAG "vw_music"
/* ---------------- 频谱参数配置 ---------------- */
#define SPEC_BAND_NUM          32          /* 频点数量 */
#define SPEC_TIMER_PERIOD_MS   20          /* 定时器周期，模拟输入刷新频率 */
#define SPEC_BAR_MAX_HEIGHT    180         /* 柱状条最大高度 */
#define SPEC_BAR_MIN_HEIGHT    5           /* 柱状条最小可见高度 */
#define SPEC_PEAK_DECAY_STEP   2           /* 小横条每次下落步长（比柱子变化慢） */
#define SPEC_REFLECT_SCALE     60          /* 倒影高度比例(百分比) */
#define SPEC_REFLECT_OPA       LV_OPA_40   /* 倒影透明度 */
#define ROUND_SPEC_BASE_OFFSET 300

static music_fft_view_t vw = {0};


// 圆形频谱
static lv_obj_t *_craete_round_spectrum(lv_obj_t *parent){
    
    vw.spectrum_round.vw = lv_round_fft_create(parent);
    lv_round_fft_set_count(vw.spectrum_round.vw, 32);
    
    lv_obj_set_size(vw.spectrum_round.vw, 200, 200);
    lv_round_fft_set_val_range(vw.spectrum_round.vw, ROUND_SPEC_BASE_OFFSET, 80);
    lv_round_fft_set_rotation(vw.spectrum_round.vw, 90);
    lv_round_fft_set_colors(vw.spectrum_round.vw, lv_color_hex(0xe9dbfc), lv_color_hex(0x6f8af6));
    lv_obj_center(vw.spectrum_round.vw);
    
    lv_obj_t *cover = lv_img_create(vw.spectrum_round.vw);
    lv_img_set_src(cover, &img_lv_demo_music_cover_1);
    lv_obj_center(cover);

    lv_obj_t *label_lyrc = lv_label_create(parent);
    lv_obj_set_style_max_width(label_lyrc, 200, 0);
    lv_label_set_long_mode(label_lyrc, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_style_text_font(label_lyrc, cl_ui_get_font(), 0);
    lv_obj_set_style_text_color(label_lyrc, lv_color_hex(0xffffff), 0);
    lv_obj_align(label_lyrc, LV_ALIGN_CENTER, 0, 120);

    vw.spectrum_round.img = cover;
    vw.spectrum_round.lyrc = label_lyrc;
    return vw.spectrum_round.vw;
}

static lv_obj_t *_create_bar_spectrum(lv_obj_t* parent){
    
    lv_obj_t *bar_spectrum = lv_bar_spectrum_create(parent);
    lv_obj_set_size(bar_spectrum, 300, 300);
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
    lv_obj_set_style_max_width(label_lyrc, 250, 0);
    lv_label_set_long_mode(label_lyrc, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_font(label_lyrc, cl_ui_get_font(), 0);
    lv_obj_set_style_text_color(label_lyrc, lv_color_hex(0xffffff), 0);
    // lv_label_set_text(label_lyrc, "lyrc");
    lv_obj_align(label_lyrc, LV_ALIGN_CENTER, 0, 80);
    
    vw.spectrum_bar.vw = bar_spectrum;
    vw.spectrum_bar.lyrc = label_lyrc;
    return bar_spectrum;
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
    lv_obj_set_style_bg_opa(btn_exit, LV_OPA_TRANSP, 0);
    lv_obj_set_style_radius(btn_exit, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_shadow_width(btn_exit, 0, 0);
    lv_obj_t *label_exit = lv_img_create(btn_exit);
    lv_img_set_src(label_exit, &icon_exit_32);
    lv_obj_center(label_exit);
    lv_obj_add_flag(cont, LV_OBJ_FLAG_HIDDEN);

    vw.cont_more.cont = cont;
    vw.cont_more.btn_bt = btn_mode_bt;
    vw.cont_more.btn_tf = btn_mode_tf;
    vw.cont_more.btn_exit = btn_exit;
    return cont;
}



// /* ---------------- 创建 item ---------------- */
static void song_click_event_cb(lv_event_t *e){
    #ifndef SIMULATOR
        uint32_t index = (uint32_t)(intptr_t)lv_event_get_user_data(e);
        vb_api_music_play_by_index(index);
    #endif
}

lv_obj_t *vw_music_fft_create_music_item(uint32_t index, const char *name, uint32_t play_idx)
{
    lv_obj_t *item = lv_obj_create(vw.list.cont_list);
    lv_obj_set_user_data(item, (void *)(uintptr_t)index);
    lv_obj_set_size(item, 300, 60);
    lv_obj_set_style_radius(item, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(item, lv_color_hex(0x2b2b2b), 0);
    lv_obj_set_style_bg_color(item, lv_color_hex(0xff5353), LV_PART_MAIN | LV_STATE_CHECKED);
    lv_obj_set_style_border_width(item, 0, 0);
    lv_obj_clear_flag(item, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(item, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(item, song_click_event_cb, LV_EVENT_CLICKED, (void*)(intptr_t)index);

    if (index == play_idx) {
        lv_obj_add_state(item, LV_STATE_CHECKED);
    }

    lv_obj_t* text = lv_label_create(item);
    lv_label_set_long_mode(text, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_width(text, 260); 
    lv_obj_align(text, LV_ALIGN_LEFT_MID, 20, 0);
    lv_obj_set_style_text_font(text, cl_ui_get_font(), 0);
    lv_obj_set_scrollbar_mode(text, LV_SCROLLBAR_MODE_AUTO);
    lv_label_set_text(text, name);
    lv_obj_set_style_text_color(text, lv_color_white(), 0);

    return item;
}

static lv_obj_t *_create_list_cont(lv_obj_t *parent){

    lv_obj_t *cont = lv_obj_create(parent);
    lv_obj_set_style_border_width(cont, 0, 0);
    lv_obj_set_style_bg_color(cont, lv_color_hex(0x000000), 0);
    lv_obj_set_style_pad_all(cont, 0, 0);
    lv_obj_set_size(cont, LV_HOR_RES, LV_VER_RES);
    lv_obj_clear_flag(cont, LV_OBJ_FLAG_SCROLLABLE);

    lv_obj_t* status_bar_label = lv_label_create(cont);
    lv_label_set_text(status_bar_label, LT_MUSIC_LIST);
    lv_obj_set_style_text_font(status_bar_label, cl_ui_get_font(), 0);
    lv_obj_set_style_text_color(status_bar_label, lv_color_hex(0xFFFFFF), 0);
    lv_obj_align(status_bar_label, LV_ALIGN_TOP_MID, 0, 30);

    /* 列表容器（替换原 LVGL8 lv_list_create） */
    lv_obj_t *list_cont = lv_obj_create(cont);
    lv_obj_set_size(list_cont, LV_PCT(100), LV_PCT(70));
    lv_obj_set_style_bg_opa(list_cont, LV_OPA_TRANSP, 0);
    lv_obj_align(list_cont, LV_ALIGN_TOP_MID, 10, 70);
    lv_obj_set_style_border_width(list_cont, 0, 0);
    lv_obj_set_style_radius(list_cont, 0, 0);
    lv_obj_set_style_pad_row(list_cont, 0, 20);
    lv_obj_set_style_pad_all(list_cont, 0, 0);
    lv_obj_set_style_bg_color(list_cont, lv_color_hex(0x000000), 0);
    lv_obj_set_scrollbar_mode(list_cont, LV_SCROLLBAR_MODE_OFF);

    lv_obj_set_scroll_dir(list_cont, LV_DIR_VER);
    lv_obj_set_flex_flow(list_cont, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_flex_align(list_cont, LV_FLEX_ALIGN_START, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);

    lv_obj_add_flag(cont, LV_OBJ_FLAG_HIDDEN);
    // lv_obj_add_event_cb(list_cont, scroll_cb, LV_EVENT_SCROLL, NULL);

    vw.list.loaded_start = 0;
    vw.list.loaded_end = 0;
    vw.list.cont = cont;
    vw.list.cont_list = list_cont;

    return cont;
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

    // 歌曲名称
    lv_obj_t *title = lv_label_create(content);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 50);
    lv_obj_set_style_text_font(title, cl_ui_get_font(), 0);
    lv_obj_set_width(title, 140);
    lv_obj_set_style_text_color(title, lv_color_hex(0xffffff), 0);
    lv_label_set_long_mode(title, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_style_transform_zoom(title, 400, 0); // 256=2x LVGL zoom
    lv_obj_set_style_transform_pivot_x(title, 70, 0);
    lv_obj_set_style_transform_pivot_y(title, 10, 0);
    lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_CENTER, 0);

    // 歌手
    lv_obj_t *singer = lv_label_create(content);
    lv_obj_align(singer, LV_ALIGN_TOP_MID, 0, 80);
    lv_obj_set_style_text_font(singer, cl_ui_get_font(), 0);
    lv_obj_set_width(singer, 140);
    lv_obj_set_style_text_color(singer, lv_color_hex(0x999999), 0);
    lv_label_set_long_mode(singer, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_style_text_align(singer, LV_TEXT_ALIGN_CENTER, 0);

    // Play Button
    lv_obj_t *btn_play = lv_btn_create(content);
    lv_obj_set_size(btn_play, 100, 100);
    lv_obj_align(btn_play, LV_ALIGN_CENTER, 0, 0);
    lv_obj_set_style_shadow_width(btn_play, 0, 0);
    lv_obj_set_style_radius(btn_play, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_bg_color(btn_play, lv_color_hex(0x333333), 0);
    lv_obj_set_style_bg_opa(btn_play, LV_OPA_40, 0);
    lv_obj_t *icon_play = lv_img_create(btn_play);
    lv_obj_center(icon_play);
    lv_img_set_src(icon_play, &icon_play_40);

    // 进度条
    lv_obj_t *arc = lv_arc_create(content);
    lv_obj_set_size(arc, 113, 113); // 比按钮大约8px
    lv_obj_align_to(arc, btn_play, LV_ALIGN_CENTER, 0, 0);
    lv_arc_set_bg_angles(arc, 0, 360);   // 整圈
    lv_arc_set_value(arc, 50); // 设置初始进度为0
    lv_arc_set_rotation(arc, -90);
    lv_obj_set_style_arc_width(arc, 3, LV_PART_MAIN); // 底条宽度
    lv_obj_set_style_arc_color(arc, lv_color_hex(0x333333), LV_PART_MAIN); // 底色
    lv_obj_set_style_arc_width(arc, 3, LV_PART_INDICATOR); // 前景宽度
    lv_obj_set_style_arc_color(arc, lv_color_hex(0xff5353), LV_PART_INDICATOR); // 前景色
    lv_obj_remove_style(arc, NULL, LV_PART_KNOB);
    lv_obj_remove_flag(arc, LV_OBJ_FLAG_CLICKABLE);

    // 保证arc在按钮下层显示
    lv_obj_move_background(arc);

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

    // 创建一个横向flex布局容器并两端对齐
    lv_obj_t *ctrl_bottom_row = lv_obj_create(content);
    lv_obj_set_size(ctrl_bottom_row, 240, 48); // 根据需要调整宽度
    lv_obj_align(ctrl_bottom_row, LV_ALIGN_CENTER, 0, 100);
    lv_obj_set_flex_flow(ctrl_bottom_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(ctrl_bottom_row, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_pad_column(ctrl_bottom_row, 40, 0); // 设置子控件间的间距为16px，可根据需要调整
    lv_obj_set_style_bg_opa(ctrl_bottom_row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(ctrl_bottom_row, 0, 0);
    lv_obj_set_style_shadow_width(ctrl_bottom_row, 0, 0);
    lv_obj_clear_flag(ctrl_bottom_row, LV_OBJ_FLAG_SCROLLABLE);

    // Volume Button
    lv_obj_t *btn_volume = lv_btn_create(ctrl_bottom_row);
    lv_obj_set_size(btn_volume, 48, 48);
    lv_obj_set_style_bg_opa(btn_volume, LV_OPA_0, 0);
    lv_obj_set_style_shadow_width(btn_volume, 0, 0);
    lv_obj_t *icon_volume = lv_img_create(btn_volume);
    lv_obj_center(icon_volume);
    lv_img_set_src(icon_volume, &icon_volume_32); // 你需要有icon_volume_48图片资源

    // List Button (before More Button)
    lv_obj_t *btn_list = lv_btn_create(ctrl_bottom_row);
    lv_obj_set_size(btn_list, 48, 48);
    lv_obj_set_style_bg_opa(btn_list, LV_OPA_0, 0);
    lv_obj_set_style_shadow_width(btn_list, 0, 0);
    lv_obj_t *icon_list = lv_img_create(btn_list);
    lv_obj_center(icon_list);
    lv_img_set_src(icon_list, &icon_list_32);

    // More Button
    lv_obj_t *btn_more = lv_btn_create(ctrl_bottom_row);
    lv_obj_set_size(btn_more, 48, 48);
    lv_obj_set_style_bg_opa(btn_more, LV_OPA_0, 0);
    lv_obj_set_style_shadow_width(btn_more, 0, 0);
    lv_obj_t *icon_more = lv_img_create(btn_more);
    lv_obj_center(icon_more);
    lv_img_set_src(icon_more, &icon_more_32);

    _create_list_cont(content);
    
    vw.ctrl.btn_play = btn_play;
    vw.ctrl.btn_prev = btn_priv;
    vw.ctrl.btn_next = btn_next;
    vw.ctrl.btn_list = btn_list;
    vw.ctrl.btn_volume = btn_volume;
    vw.ctrl.btn_more = btn_more;
    vw.ctrl.progress_bar = arc;
    vw.ctrl.title = title;
    vw.ctrl.singer = singer;
    vw.ctrl.vw = content;
    return content;
}



music_fft_view_t* music_fft_view_create(lv_obj_t* root)
{
    lv_obj_remove_style_all(root);
    lv_obj_set_size(root, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_bg_color(root, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(root, LV_OPA_COVER, 0);
    lv_obj_clear_flag(root, LV_OBJ_FLAG_SCROLLABLE);

    vw.tileview.vw = lv_tileview_create(root);
    lv_obj_set_size(vw.tileview.vw, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_scrollbar_mode(vw.tileview.vw, LV_SCROLLBAR_MODE_OFF);
    lv_obj_set_style_bg_color(vw.tileview.vw, lv_color_black(), 0);

    // 创建四个tile
    vw.tileview.tile[0] = lv_tileview_add_tile(vw.tileview.vw, 0, 0, LV_DIR_RIGHT);
    vw.tileview.tile[1] = lv_tileview_add_tile(vw.tileview.vw, 0, 1, LV_DIR_RIGHT);
    vw.tileview.tile_fft[0] = lv_tileview_add_tile(vw.tileview.vw, 1, 0, LV_DIR_LEFT | LV_DIR_BOTTOM);
    vw.tileview.tile_fft[1] = lv_tileview_add_tile(vw.tileview.vw, 1, 1, LV_DIR_LEFT | LV_DIR_TOP);
    
    // 创建初始
    vw.tileview.active_row = 0;
    _create_ctrl_content(vw.tileview.tile[0]);
    
    _create_more_cont(root);

    _create_bar_spectrum(vw.tileview.tile_fft[0]);
    _craete_round_spectrum(vw.tileview.tile_fft[1]);

    sys_music_fft_event_init(&vw, root);
    sys_music_fft_state_init(&vw, root);


    return &vw;
}



void music_fft_view_delete(void)
{
    sys_music_fft_uninit();
}
