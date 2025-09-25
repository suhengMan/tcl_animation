// #include "ctrl_center.h"

/* 保留你工程里的这些头，以保持行为一致。
 * 如需完全解耦，可把这些改成回调钩子。 */
#include "cl_ui.h"
#include "utils/btn/xz_btn.h"

#define TAG "CRTL_BAR"

LV_FONT_DECLARE(font_puhui_20_4)
LV_FONT_DECLARE(myFont)

LV_IMG_DECLARE(icon_bright_24)
LV_IMG_DECLARE(icon_vol_24)


/* 布局参数，可按需调整 */
#define OPEN_Y    0
#define CLOSED_Y 280

typedef struct {
    lv_obj_t *bg_mask;      /* 背景遮罩 */
    lv_obj_t *cont;         /* 控制中心容器 */
    lv_coord_t press_y;     /* 按下时触点Y */
    lv_coord_t start_y;     /* 按下时容器Y */
    ctrl_center_cb_t ctrl_evt_cb;
} ctrl_center_t;

static void _drag_event_cb(lv_event_t *e);
/* ---------- 工具/内部函数 ---------- */

static int _map(int in, int in_min, int in_max, int out_min, int out_max) {
    return (in_max == in_min) ? 0 : (in - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}


static void _menu_anim_close_cb(lv_anim_t *a){
    lv_obj_t *obj = (lv_obj_t *)a->var;
    /* 收起后把背景透明（保留原来做法） */
    lv_obj_set_style_bg_opa(obj, LV_OPA_0, 0);
    ctrl_center_t *ctrl = lv_anim_get_user_data(a);
    lv_obj_add_flag(ctrl->bg_mask, LV_OBJ_FLAG_HIDDEN);
    if (ctrl->ctrl_evt_cb)
    {
        ctrl->ctrl_evt_cb(CTRL_CENTER_CLOSED, NULL);
    }
     
    lv_obj_add_flag(ctrl->bg_mask, LV_OBJ_FLAG_HIDDEN);
}

static void _menu_anim_open_cb(lv_anim_t *a){
    lv_obj_t *obj = (lv_obj_t *)a->var;
    /* 收起后把背景透明（保留原来做法） */
    // lv_obj_set_style_bg_opa(obj, LV_OPA_0, 0);
    ctrl_center_t *ctrl = lv_anim_get_user_data(a);
    if (ctrl->ctrl_evt_cb)
    {
        ctrl->ctrl_evt_cb(CTRL_CENTER_OPEN, NULL);
    }
     
    lv_obj_clear_flag(ctrl->bg_mask, LV_OBJ_FLAG_HIDDEN);
}

static void _anim_cb(void *arg, int32_t val){
    lv_obj_t *obj = (lv_obj_t*)arg;
    lv_obj_set_y(obj, val);
    uint32_t i, cnt = lv_obj_get_event_count(obj);
    for (i = 0; i < cnt; i++) {
        lv_event_dsc_t *d = lv_obj_get_event_dsc(obj, i);
        if (!d) continue;
        if (lv_event_dsc_get_cb(d) == _drag_event_cb) {
            ctrl_center_t *self = (ctrl_center_t *)lv_event_dsc_get_user_data(d);
            if (self->ctrl_evt_cb) self->ctrl_evt_cb(CTRL_CENTER_Y_CHANGE, val);
        }
    }
}

/* 统一的 y 动画 */
static void menu_anim_to_y(ctrl_center_t *ctrl, lv_coord_t to_y, uint32_t time_ms, lv_anim_completed_cb_t cb)
{
    lv_obj_t *obj = ctrl->cont;
    lv_anim_t a;
    lv_anim_init(&a);
    lv_anim_set_var(&a, obj);
    lv_anim_set_values(&a, lv_obj_get_y(obj), to_y);
    lv_anim_set_time(&a, time_ms);
    lv_anim_set_exec_cb(&a, (lv_anim_exec_xcb_t)_anim_cb);
    lv_anim_set_path_cb(&a, lv_anim_path_ease_out);
    lv_anim_set_user_data(&a, ctrl);
    if (cb) lv_anim_set_completed_cb(&a, cb);
    lv_anim_start(&a);
}

/* 对外的开合接口 */
void ctrl_center_open(ctrl_center_t *ctrl)  { menu_anim_to_y(ctrl, OPEN_Y,   100, _menu_anim_open_cb); }
void ctrl_center_close(ctrl_center_t *ctrl) { menu_anim_to_y(ctrl, CLOSED_Y, 100, _menu_anim_close_cb); }

/* ---------- 业务事件（按钮/滑块） ---------- */
static void _on_bright_set(lv_event_t *e){
    if (lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t *slider = lv_event_get_target(e);
        int value = lv_slider_get_value(slider);
        // lcd_bl_set_backlight(value);
    }
}

static void _on_vol_set(lv_event_t *e){
    if (lv_event_get_code(e) == LV_EVENT_VALUE_CHANGED) {
        lv_obj_t *slider = lv_event_get_target(e);
        int value = lv_slider_get_value(slider);
        if (value >=90)       value = 10;
        else if (value < 10)  value = 1;
        else                  value = value / 10;
        // app_tone_set_volume(value);
    }
}

/* ---------- UI 构建 ---------- */
static lv_obj_t *_create_menu_cont(lv_obj_t *cont_main) {

    /* 亮度条 */
    lv_obj_t *bar_brightness = lv_slider_create(cont_main);
    lv_obj_set_style_bg_opa(bar_brightness, 0, LV_PART_KNOB);
    lv_obj_set_style_radius(bar_brightness, 0, LV_PART_INDICATOR);
    lv_obj_set_size(bar_brightness, 200, 40);
    lv_obj_align(bar_brightness, LV_ALIGN_TOP_MID, 0, 20);
    lv_obj_t *img_bright_n = lv_img_create(bar_brightness);
    lv_img_set_src(img_bright_n, &icon_bright_24);
    lv_obj_align(img_bright_n, LV_ALIGN_LEFT_MID, 16, 0);
    lv_bar_set_value(bar_brightness, 50, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(bar_brightness, lv_color_hex(0x4A505A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(bar_brightness, lv_color_hex(0xFFFFFF), LV_PART_INDICATOR | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(bar_brightness, 100, LV_PART_MAIN | LV_STATE_DEFAULT);

    /* 音量条 */
    lv_obj_t *bar_volume = lv_slider_create(cont_main);
    lv_obj_set_style_bg_opa(bar_volume, 0, LV_PART_KNOB);
    lv_obj_set_style_radius(bar_volume, 0, LV_PART_INDICATOR);
    lv_obj_add_flag(bar_volume, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_set_size(bar_volume, 200, 40);
    lv_obj_align_to(bar_volume, bar_brightness, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 20);
    lv_obj_t *img_volume_n = lv_img_create(bar_volume);
    lv_img_set_src(img_volume_n, &icon_vol_24);
    lv_obj_align(img_volume_n, LV_ALIGN_LEFT_MID, 16, 0);
    lv_bar_set_value(bar_volume, 50, LV_ANIM_OFF);
    lv_obj_set_style_bg_color(bar_volume, lv_color_hex(0x4A505A), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(bar_volume, 100, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(bar_volume, lv_color_hex(0xFFFFFF), LV_PART_INDICATOR | LV_STATE_DEFAULT);

    /* 初始化亮度/音量 */
    // u8 bl = xiaozhi_board_get_bl();
    u8 bl = 100;
    lv_slider_set_value(bar_brightness, bl, LV_ANIM_OFF);

    // int volume = app_audio_get_volume(APP_AUDIO_STATE_MUSIC);
    int volume = 28;
    volume = _map(volume, 0, 30, 0, 100);
    lv_slider_set_value(bar_volume, volume, LV_ANIM_OFF);

    lv_obj_add_event_cb(bar_brightness, _on_bright_set, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(bar_volume,     _on_vol_set,     LV_EVENT_VALUE_CHANGED, NULL);

    return cont_main;
}

/* ---------- 拖拽回调（带外部回调） ---------- */

static void _drag_event_cb(lv_event_t *e)
{
    ctrl_center_t *self = (ctrl_center_t *)lv_event_get_user_data(e);
    lv_event_code_t code = lv_event_get_code(e);

    if(code == LV_EVENT_PRESSED) {
        lv_obj_set_style_bg_opa(self->cont, LV_OPA_COVER, 0);    
        // lv_obj_clear_flag(self->bg_mask, LV_OBJ_FLAG_HIDDEN);

        lv_point_t p; lv_indev_t *indev = lv_indev_get_act();
        lv_indev_get_point(indev, &p);
        self->press_y = p.y;
        self->start_y = lv_obj_get_y(self->cont);
        if (self->ctrl_evt_cb)
        {
            self->ctrl_evt_cb(CTRL_CENTER_PRESS, NULL);
        }
    }
    else if(code == LV_EVENT_PRESSING) {
        lv_point_t p; lv_indev_t *indev = lv_indev_get_act();
        lv_indev_get_point(indev, &p);

        lv_coord_t dy    = p.y - self->press_y;   /* 仅 Y 方向 */
        lv_coord_t new_y = self->start_y + dy;
        if(new_y < OPEN_Y)   new_y = OPEN_Y;
        if(new_y > CLOSED_Y) new_y = CLOSED_Y;
        if (self->ctrl_evt_cb)
        {
            self->ctrl_evt_cb(CTRL_CENTER_Y_CHANGE, (void*)new_y);
        }
        
        lv_obj_set_y(self->cont, new_y);
    }
    else if(code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST) {
        lv_point_t p; lv_indev_t *indev = lv_indev_get_act();
        lv_indev_get_point(indev, &p);
        lv_coord_t diff = p.y - self->press_y;

        if (self->start_y == OPEN_Y) {
            if (diff >= 20) { ctrl_center_close(self); }
            else             { ctrl_center_open(self);  }
        } else {
            if (diff <= -10)  { ctrl_center_open(self);  }
            else             { ctrl_center_close(self); }
        }
    }else if (code == LV_EVENT_DELETE)
    {
        // LOGE("DEL SELF");
        free(self);
    }
    
}

static void _on_bg_mask_click(lv_event_t *e){
    lv_event_code_t code = lv_event_get_code(e);
    ctrl_center_t *ctrl = lv_event_get_user_data(e);
    if (code == LV_EVENT_RELEASED)
    {
        ctrl_center_close(ctrl);
    }
}

/* ---------- 对外创建接口 ---------- */

void ctrl_center_set_cb(lv_obj_t *ctrl, ctrl_center_cb_t ctrl_evt_cb) {
    /* 通过对象的第一个事件描述符拿回 user_data（LVGL v8 里安全做法是保存在回调里）
       这里简化：我们把 user_data 直接存到对象的 event 回调上，因此可复用。 */
    /* 如果你工程启用了 OBJ user data，也可以用 lv_obj_set_user_data/ lv_obj_get_user_data 来保存 self。 */
    uint32_t i, cnt = lv_obj_get_event_count(ctrl);
    for (i = 0; i < cnt; i++) {
        lv_event_dsc_t *d = lv_obj_get_event_dsc(ctrl, i);
        if (!d) continue;
        if (lv_event_dsc_get_cb(d) == _drag_event_cb) {
            ctrl_center_t *self = (ctrl_center_t *)lv_event_dsc_get_user_data(d);
            if (self) self->ctrl_evt_cb = ctrl_evt_cb;
            break;
        }
    }
}

lv_obj_t *ctrl_center_create(lv_obj_t *parent, ctrl_center_cb_t ctrl_evt_cb)
{
    /* 分配实例 */
    ctrl_center_t *self = (ctrl_center_t *)malloc(sizeof(ctrl_center_t));
    memset(self, 0, sizeof(*self));
    self->ctrl_evt_cb = ctrl_evt_cb;

    /* 背景遮罩（按你原来的做法） */
    self->bg_mask = lv_obj_create(parent);
    lv_obj_set_size(self->bg_mask, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_bg_color(self->bg_mask, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(self->bg_mask, LV_OPA_30, 0);
    lv_obj_set_style_border_width(self->bg_mask, 0, 0);
    lv_obj_clear_flag(self->bg_mask, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_flag(self->bg_mask, LV_OBJ_FLAG_HIDDEN);
    lv_obj_add_event_cb(self->bg_mask, _on_bg_mask_click, LV_EVENT_ALL, self);


    /* 主容器 */
    self->cont = lv_obj_create(parent);
    
    lv_obj_clear_flag(self->cont, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(self->cont, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_radius(self->cont, 40, 0);
    lv_obj_set_style_bg_color(self->cont, lv_color_hex(0x000000), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_border_width(self->cont, 0, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(self->cont, LV_OPA_0, 0);
    lv_obj_set_y(self->cont, CLOSED_Y);
    lv_obj_set_x(self->cont, 0);

    _create_menu_cont(self->cont);

    /* 拖拽事件（仅竖直拖拽 + 吸附） */
    lv_obj_add_flag(self->cont, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_add_event_cb(self->cont, _drag_event_cb, LV_EVENT_ALL, self);

    return self->cont;
}
