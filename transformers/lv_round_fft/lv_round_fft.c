#include "lv_round_fft.h"
#include "lvgl.h"

#ifdef SIMULATOR
#include "lvgl/src/core/lv_obj_class_private.h"
#include "lvgl/src/core/lv_obj_private.h"
#include "lvgl/src/stdlib/lv_mem.h"
#include "lvgl/src/stdlib/lv_string.h"
#else
#include "src/core/lv_obj_class_private.h"
#include "src/core/lv_obj_private.h"
#include "src/stdlib/lv_mem.h"
#include "src/stdlib/lv_string.h"
#endif

#include <stdlib.h>
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

/**********************
 *      DEFINES
 **********************/

#define LV_ROUND_FFT_DEFAULT_COUNT     32
#define LV_ROUND_FFT_DEFAULT_R_IN      0   /* 默认从中心点开始发散，不保留中间黑圈 */
#define LV_ROUND_FFT_DEFAULT_MIN_H     0   /* 默认最小高度为 0，不再形成固定内圈 */
#define LV_ROUND_FFT_DEFAULT_MAX_IN    100
#define LV_ROUND_FFT_DEFAULT_SCALE     100  /* 100% 缩放，不改变条纹长度 */
#define LV_ROUND_FFT_DEFAULT_ROTATION  90
#define LV_ROUND_FFT_DEG_GAP           1

#define LV_ROUND_FFT_COLOR_START       lv_color_hex(0xFFFF00)
#define LV_ROUND_FFT_COLOR_END         lv_color_hex(0x00FF00)

#define MY_CLASS (&lv_round_fft_class)

/**********************
 *      TYPEDEFS
 **********************/

struct lv_round_fft_t {
    lv_obj_t obj; /* 必须把 lv_obj_t 作为首成员以匹配 LVGL 的实例布局 */
    uint8_t    count;
    int16_t   *vals;
    lv_coord_t r_in;
    lv_coord_t min_h;
    int16_t    base_offset;
    int16_t    max_in;
    int16_t    rotation; /* degrees, 0 at right, CCW positive */
    int16_t    scale;    /* 条纹长度缩放百分比，100 表示 1.0 */
    lv_color_t c_start;
    lv_color_t c_end;
};

/**********************
 *  STATIC PROTOTYPES
 **********************/

static void lv_round_fft_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj);
static void lv_round_fft_destructor(const lv_obj_class_t * class_p, lv_obj_t * obj);
static void lv_round_fft_event(const lv_obj_class_t * class_p, lv_event_t * e);
static void draw_fft(lv_obj_t * obj, lv_event_t * e);

static LV_ATTRIBUTE_FAST_MEM int32_t get_cos(float deg, int32_t a);
static LV_ATTRIBUTE_FAST_MEM int32_t get_sin(float deg, int32_t a);

/**********************
 *  GLOBAL VARIABLES
 **********************/

const lv_obj_class_t lv_round_fft_class = {
    .base_class     = &lv_obj_class,
    .constructor_cb = lv_round_fft_constructor,
    .destructor_cb  = lv_round_fft_destructor,
    .event_cb       = lv_round_fft_event,
    .width_def      = 200,
    .height_def     = 200,
    .instance_size  = sizeof(lv_round_fft_t),
    .theme_inheritable = LV_OBJ_CLASS_THEME_INHERITABLE_FALSE,
    .name           = "lv_round_fft",
};

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

lv_obj_t * lv_round_fft_create(lv_obj_t * parent)
{
    lv_obj_t * obj = lv_obj_class_create_obj(MY_CLASS, parent);
    lv_obj_class_init_obj(obj);
    return obj;
}

void lv_round_fft_set_count(lv_obj_t * obj, uint8_t count)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_round_fft_t * fft = (lv_round_fft_t *)obj;

    if(count == 0) count = 1;
    if(count == fft->count) return;

    int16_t *new_buf = (int16_t *)lv_realloc(fft->vals, sizeof(int16_t) * count);
    if(!new_buf) return;
    if(count > fft->count) {
        lv_memset(new_buf + fft->count, 0, sizeof(int16_t) * (count - fft->count));
    }
    fft->vals = new_buf;
    fft->count = count;
    lv_obj_invalidate(obj);
}

void lv_round_fft_set_val(lv_obj_t * obj, const int16_t * val)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_round_fft_t * fft = (lv_round_fft_t *)obj;
    if(!val || !fft->vals) return;

    for(uint8_t i = 0; i < fft->count; i++) {
        fft->vals[i] = val[i] + fft->base_offset;
    }
    lv_obj_invalidate(obj);
}

void lv_round_fft_set_inner_radius(lv_obj_t * obj, lv_coord_t r_in)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_round_fft_t * fft = (lv_round_fft_t *)obj;
    fft->r_in = r_in;
    lv_obj_invalidate(obj);
}

void lv_round_fft_set_min_height(lv_obj_t * obj, lv_coord_t h_min)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_round_fft_t * fft = (lv_round_fft_t *)obj;
    fft->min_h = h_min;
    lv_obj_invalidate(obj);
}

void lv_round_fft_set_val_range(lv_obj_t * obj, int16_t base, int16_t max_in)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_round_fft_t * fft = (lv_round_fft_t *)obj;
    if(max_in <= 0) max_in = 1;
    fft->max_in = max_in + base;
    fft->base_offset = base;
    lv_obj_invalidate(obj);
}

void lv_round_fft_set_rotation(lv_obj_t * obj, int16_t deg_start)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_round_fft_t * fft = (lv_round_fft_t *)obj;
    fft->rotation = deg_start;
    lv_obj_invalidate(obj);
}

void lv_round_fft_set_colors(lv_obj_t * obj, lv_color_t c_start, lv_color_t c_end)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_round_fft_t * fft = (lv_round_fft_t *)obj;
    fft->c_start = c_start;
    fft->c_end   = c_end;
    lv_obj_invalidate(obj);
}

void lv_round_fft_set_scale(lv_obj_t * obj, int16_t scale)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_round_fft_t * fft = (lv_round_fft_t *)obj;
    if(scale <= 0) scale = 1;
    fft->scale = scale;
    lv_obj_invalidate(obj);
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void lv_round_fft_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj)
{
    LV_UNUSED(class_p);
    lv_round_fft_t * fft = (lv_round_fft_t *)obj;

    fft->count    = LV_ROUND_FFT_DEFAULT_COUNT;
    fft->r_in     = LV_ROUND_FFT_DEFAULT_R_IN;
    fft->min_h    = LV_ROUND_FFT_DEFAULT_MIN_H;
    fft->max_in   = LV_ROUND_FFT_DEFAULT_MAX_IN;
    fft->rotation = LV_ROUND_FFT_DEFAULT_ROTATION;
    fft->c_start  = LV_ROUND_FFT_COLOR_START;
    fft->c_end    = LV_ROUND_FFT_COLOR_END;
    fft->scale    = LV_ROUND_FFT_DEFAULT_SCALE;

    fft->vals = (int16_t *)lv_malloc(sizeof(int16_t) * fft->count);
    if(fft->vals) {
        lv_memset(fft->vals, 0, sizeof(int16_t) * fft->count);
    }

    lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_style_bg_opa(obj, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_opa(obj, LV_OPA_TRANSP, 0);
}

static void lv_round_fft_destructor(const lv_obj_class_t * class_p, lv_obj_t * obj)
{
    LV_UNUSED(class_p);
    lv_round_fft_t * fft = (lv_round_fft_t *)obj;
    if(fft->vals) {
        lv_free(fft->vals);
        fft->vals = NULL;
    }
}

static void lv_round_fft_event(const lv_obj_class_t * class_p, lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if(code == LV_EVENT_DRAW_MAIN) {
        draw_fft(lv_event_get_target(e), e);
    }

    lv_obj_event_base(class_p, e);
}

static void draw_fft(lv_obj_t * obj, lv_event_t * e)
{
    lv_layer_t * layer = lv_event_get_layer(e);
    if(!layer) return;

    lv_round_fft_t * fft = (lv_round_fft_t *)obj;
    if(!fft->vals || fft->count == 0) return;

    lv_area_t coords;
    lv_obj_get_coords(obj, &coords);
    lv_coord_t w = lv_area_get_width(&coords);
    lv_coord_t h = lv_area_get_height(&coords);
    lv_coord_t cx = coords.x1 + w / 2;
    lv_coord_t cy = coords.y1 + h / 2;

    lv_coord_t avail_rad = LV_MIN(w, h) / 2;
    lv_coord_t r_in = fft->r_in;
    if(r_in < 0) r_in = 0;
    if(r_in > avail_rad) r_in = avail_rad;

    lv_coord_t min_h = fft->min_h;
    /* 基础最大可用高度，由控件 size 决定，保证最外圈不超过控件边界 */
    lv_coord_t h_max_base = 0;
    if(avail_rad > r_in + min_h) {
        h_max_base = avail_rad - r_in - min_h;
    }

    /* 固定中心角 + 半宽的方式来计算每条的角度范围，
     * 每条用一个等腰三角形表示：顶点在内半径，底边在外半径（尖端朝向中心/内侧） */
    float step_deg = 360.0f / (float)fft->count;        /* 每条的中心角间距（度） */
    float gap      = (float)LV_ROUND_FFT_DEG_GAP;       /* 每条左右各留出的缝隙（度） */

    /* 半宽度 = step_deg/2 - gap，保证左右各留 gap；为负时退化为细线 */
    float half_width = step_deg * 0.5f - gap;
    if(half_width < 0.0f) half_width = 0.0f;

    for(uint8_t i = 0; i < fft->count; i++) {
        /* 第 i 条的中心角与两侧边界角（度） */
        float center_deg = (float)fft->rotation + (float)i * step_deg;
        float deg_left   = center_deg - half_width;
        float deg_right  = center_deg + half_width;

        int32_t val = fft->vals[i];
        if(val < 0) val = -val;
        int32_t max_in = fft->max_in;
        if(max_in <= 0) max_in = 1;
        if(val > max_in) val = max_in;

        int32_t scale = fft->scale;
        if(scale <= 0) scale = 1;

        /* 先根据 size 算出基础最大高度，再按 scale 压缩，但不允许超过 size 限制 */
        int32_t h_max = h_max_base;
        if(h_max < 0) h_max = 0;

        int32_t scaled = (int32_t)((int64_t)val * h_max * scale / (max_in * 100));
        if(scaled > h_max) scaled = h_max;

        lv_coord_t r_out = r_in + min_h + scaled;

        lv_color_t color;
        if(h_max > 0) {
            uint8_t mix = (uint8_t)((scaled * 255) / h_max);
            color = lv_color_mix(fft->c_end, fft->c_start, mix);
        } else {
            color = fft->c_start;
        }

        /* 计算一个等腰三角形的三个顶点：
         *  - 顶点：几何中心点 (cx, cy)，所有尖端统一汇聚到同一点
         *  - 左底点：deg_left,  r_out
         *  - 右底点：deg_right, r_out
         */
        int32_t x_apex  = 0;
        int32_t y_apex  = 0;
        int32_t x_left  = get_cos(deg_left,  r_out);
        int32_t y_left  = get_sin(deg_left,  r_out);
        int32_t x_right = get_cos(deg_right, r_out);
        int32_t y_right = get_sin(deg_right, r_out);

        lv_draw_triangle_dsc_t tri_dsc;
        lv_draw_triangle_dsc_init(&tri_dsc);
#ifdef SIMULATOR
        tri_dsc.bg_opa = LV_OPA_COVER;
        tri_dsc.bg_color = color;
#else
        tri_dsc.opa = LV_OPA_COVER;
        tri_dsc.color = color;
#endif
        tri_dsc.p[0].x = (lv_value_precise_t)(cx + x_apex);
        tri_dsc.p[0].y = (lv_value_precise_t)(cy + y_apex);
        tri_dsc.p[1].x = (lv_value_precise_t)(cx + x_left);
        tri_dsc.p[1].y = (lv_value_precise_t)(cy + y_left);
        tri_dsc.p[2].x = (lv_value_precise_t)(cx + x_right);
        tri_dsc.p[2].y = (lv_value_precise_t)(cy + y_right);
        lv_draw_triangle(layer, &tri_dsc);
    }
}

static LV_ATTRIBUTE_FAST_MEM int32_t get_cos(float deg, int32_t a)
{
    float rad = deg * (float)M_PI / 180.0f;
    float r   = cosf(rad) * (float)a;
    return (int32_t)lrintf(r);
}

static LV_ATTRIBUTE_FAST_MEM int32_t get_sin(float deg, int32_t a)
{
    float rad = deg * (float)M_PI / 180.0f;
    float r   = sinf(rad) * (float)a;
    return (int32_t)lrintf(r);
}

