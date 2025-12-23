#include "lv_toast.h"



// LV_FONT_DECLARE(font_puhui_18_4)

/* ---------- 可按需修改的参数 ---------- */
#define TOAST_FADE_IN_TIME   250   // 渐显时间 ms
#define TOAST_FADE_OUT_TIME  250   // 渐隐时间 ms
#define TOAST_DEFAULT_STAY   1500  // 默认停留时间 ms（不包含淡入淡出）
#define TOAST_MAX_STAY       3500  // 默认停留时间 ms（不包含淡入淡出）
#define TOAST_BOTTOM_OFFSET  60    // 距离底部的像素偏移
/* ------------------------------------ */

static lv_font_t *s_toast_font = NULL;
void lv_toast_set_font(lv_font_t *font){
    s_toast_font = font;
}

/* 动画执行回调：修改对象的整体透明度 */
static void toast_opa_exec_cb(void *var, int32_t value)
{
    lv_obj_t *obj = (lv_obj_t *)var;
    lv_obj_set_style_opa(obj, (lv_opa_t)value, 0);
}

/* 渐隐动画结束后删除对象 */
static void toast_anim_ready_cb(lv_anim_t *a)
{
    lv_obj_t *obj = (lv_obj_t *)a->var;
    if (obj && !lv_obj_is_valid(obj)) {
        return;
    }
    lv_obj_del(obj);
}

/**
 * @brief 创建一个 toast 对象并播放动画
 *
 * @param text      文本
 * @param stay_time 停留时间，毫秒
 */
lv_obj_t * lv_toast_show(const char *text, uint32_t stay_time)
{
    if (text == NULL) {
        text = "";
    }
    if (stay_time == 0) {
        stay_time = TOAST_DEFAULT_STAY;
    }
    if (stay_time > TOAST_MAX_STAY)
    {
        stay_time = TOAST_MAX_STAY;
    }
    

    lv_obj_t *parent = lv_screen_active();   // LVGL9 的当前 screen
    if (!parent) {
        return NULL;
    }

    /* 容器：黑底圆角，居中靠底 */
    lv_obj_t *toast = lv_obj_create(parent);
    lv_obj_remove_style_all(toast);  // 去掉默认样式，完全自定义

    lv_obj_set_style_bg_color(toast, lv_color_hex(0x999999), 0);
    lv_obj_set_style_bg_opa(toast, LV_OPA_80, 0);
    lv_obj_set_style_radius(toast, 12, 0);
    lv_obj_set_style_pad_hor(toast, 16, 0);
    lv_obj_set_style_pad_ver(toast, 10, 0);
    lv_obj_set_style_border_width(toast, 0, 0);
    lv_obj_set_style_opa(toast, LV_OPA_TRANSP, 0);   // 初始透明

    lv_obj_clear_flag(toast, LV_OBJ_FLAG_SCROLLABLE);

    /* 放到底部中间 */
    lv_obj_set_width(toast, LV_SIZE_CONTENT);
    lv_obj_set_height(toast, LV_SIZE_CONTENT);
    lv_obj_align(toast, LV_ALIGN_BOTTOM_MID, 0, -TOAST_BOTTOM_OFFSET);

    /* 文本 label */
    lv_obj_t *label = lv_label_create(toast);
    lv_label_set_text(label, text);
    if (s_toast_font) {
        lv_obj_set_style_text_font(label, s_toast_font, 0);
    }
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_obj_center(label);

    /* ---------- 动画：淡入 ---------- */
    lv_anim_t a_in;
    lv_anim_init(&a_in);
    lv_anim_set_var(&a_in, toast);
    lv_anim_set_values(&a_in, LV_OPA_TRANSP, LV_OPA_COVER);
    lv_anim_set_time(&a_in, TOAST_FADE_IN_TIME);
    lv_anim_set_exec_cb(&a_in, toast_opa_exec_cb);
    lv_anim_set_path_cb(&a_in, lv_anim_path_ease_out);
    lv_anim_start(&a_in);

    /* ---------- 动画：停留 + 淡出 + 删除 ---------- */
    lv_anim_t a_out;
    lv_anim_init(&a_out);
    lv_anim_set_var(&a_out, toast);
    lv_anim_set_values(&a_out, LV_OPA_COVER, LV_OPA_TRANSP);
    lv_anim_set_time(&a_out, TOAST_FADE_OUT_TIME);
    lv_anim_set_exec_cb(&a_out, toast_opa_exec_cb);
    lv_anim_set_path_cb(&a_out, lv_anim_path_ease_in);

    /* 在淡入 + 停留时间之后再开始淡出 */
    lv_anim_set_delay(&a_out, TOAST_FADE_IN_TIME + stay_time);

    /* 动画结束时自动删除对象 */
    lv_anim_set_ready_cb(&a_out, toast_anim_ready_cb);

    lv_anim_start(&a_out);

    return toast;
}
