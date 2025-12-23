// #include "lv_round_fft.h"
// #include "lvgl.h"

// #ifdef SIMULATOR
// #include "lvgl/src/core/lv_obj_class_private.h"
// #include "lvgl/src/core/lv_obj_private.h"
// #include "lvgl/src/stdlib/lv_mem.h"
// #include "lvgl/src/stdlib/lv_string.h"
// #else
// #include "src/core/lv_obj_class_private.h"
// #include "src/core/lv_obj_private.h"
// #include "src/stdlib/lv_mem.h"
// #include "src/stdlib/lv_string.h"
// #endif

// #include <stdlib.h>

// /**********************
//  *      DEFINES
//  **********************/

// #define LV_ROUND_FFT_DEFAULT_COUNT     32
// #define LV_ROUND_FFT_DEFAULT_R_IN      60
// #define LV_ROUND_FFT_DEFAULT_MIN_H     6
// #define LV_ROUND_FFT_DEFAULT_MAX_GAIN  2
// #define LV_ROUND_FFT_DEFAULT_ROTATION  90
// #define LV_ROUND_FFT_DEG_GAP           2

// #define LV_ROUND_FFT_COLOR_START       lv_color_hex(0xFFFF00)
// #define LV_ROUND_FFT_COLOR_END         lv_color_hex(0x00FF00)

// #define MY_CLASS (&lv_round_fft_class)

// /**********************
//  *      TYPEDEFS
//  **********************/

// struct lv_round_fft_t {
//     lv_obj_t obj; /* 必须把 lv_obj_t 作为首成员以匹配 LVGL 的实例布局 */
//     uint8_t    count;
//     int16_t   *vals;
//     lv_coord_t r_in;
//     lv_coord_t min_h;
//     int16_t    max_gain;
//     int16_t    rotation; /* degrees, 0 at right, CCW positive */
//     lv_color_t c_start;
//     lv_color_t c_end;
// };

// /**********************
//  *  STATIC PROTOTYPES
//  **********************/

// static void lv_round_fft_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj);
// static void lv_round_fft_destructor(const lv_obj_class_t * class_p, lv_obj_t * obj);
// static void lv_round_fft_event(const lv_obj_class_t * class_p, lv_event_t * e);
// static void draw_fft(lv_obj_t * obj, lv_event_t * e);

// static LV_ATTRIBUTE_FAST_MEM int32_t get_cos(int32_t deg, int32_t a);
// static LV_ATTRIBUTE_FAST_MEM int32_t get_sin(int32_t deg, int32_t a);

// /**********************
//  *  GLOBAL VARIABLES
//  **********************/

// const lv_obj_class_t lv_round_fft_class = {
//     .base_class     = &lv_obj_class,
//     .constructor_cb = lv_round_fft_constructor,
//     .destructor_cb  = lv_round_fft_destructor,
//     .event_cb       = lv_round_fft_event,
//     .width_def      = 200,
//     .height_def     = 200,
//     .instance_size  = sizeof(lv_round_fft_t),
//     .theme_inheritable = LV_OBJ_CLASS_THEME_INHERITABLE_FALSE,
//     .name           = "lv_round_fft",
// };

// /**********************
//  *   GLOBAL FUNCTIONS
//  **********************/

// lv_obj_t * lv_round_fft_create(lv_obj_t * parent)
// {
//     lv_obj_t * obj = lv_obj_class_create_obj(MY_CLASS, parent);
//     lv_obj_class_init_obj(obj);
//     return obj;
// }

// void lv_round_fft_set_count(lv_obj_t * obj, uint8_t count)
// {
//     LV_ASSERT_OBJ(obj, MY_CLASS);
//     lv_round_fft_t * fft = (lv_round_fft_t *)obj;

//     if(count == 0) count = 1;
//     if(count == fft->count) return;

//     int16_t *new_buf = (int16_t *)lv_realloc(fft->vals, sizeof(int16_t) * count);
//     if(!new_buf) return;
//     if(count > fft->count) {
//         lv_memset(new_buf + fft->count, 0, sizeof(int16_t) * (count - fft->count));
//     }
//     fft->vals = new_buf;
//     fft->count = count;
//     lv_obj_invalidate(obj);
// }

// void lv_round_fft_set_val(lv_obj_t * obj, const int16_t * val)
// {
//     LV_ASSERT_OBJ(obj, MY_CLASS);
//     lv_round_fft_t * fft = (lv_round_fft_t *)obj;
//     if(!val || !fft->vals) return;

//     for(uint8_t i = 0; i < fft->count; i++) {
//         fft->vals[i] = val[i];
//     }
//     lv_obj_invalidate(obj);
// }

// void lv_round_fft_set_inner_radius(lv_obj_t * obj, lv_coord_t r_in)
// {
//     LV_ASSERT_OBJ(obj, MY_CLASS);
//     lv_round_fft_t * fft = (lv_round_fft_t *)obj;
//     fft->r_in = r_in;
//     lv_obj_invalidate(obj);
// }

// void lv_round_fft_set_min_height(lv_obj_t * obj, lv_coord_t h_min)
// {
//     LV_ASSERT_OBJ(obj, MY_CLASS);
//     lv_round_fft_t * fft = (lv_round_fft_t *)obj;
//     fft->min_h = h_min;
//     lv_obj_invalidate(obj);
// }

// void lv_round_fft_set_max_gain(lv_obj_t * obj, int16_t gain)
// {
//     LV_ASSERT_OBJ(obj, MY_CLASS);
//     lv_round_fft_t * fft = (lv_round_fft_t *)obj;
//     if(gain <= 0) gain = 1;
//     fft->max_gain = gain;
//     lv_obj_invalidate(obj);
// }

// void lv_round_fft_set_rotation(lv_obj_t * obj, int16_t deg_start)
// {
//     LV_ASSERT_OBJ(obj, MY_CLASS);
//     lv_round_fft_t * fft = (lv_round_fft_t *)obj;
//     fft->rotation = deg_start;
//     lv_obj_invalidate(obj);
// }

// void lv_round_fft_set_colors(lv_obj_t * obj, lv_color_t c_start, lv_color_t c_end)
// {
//     LV_ASSERT_OBJ(obj, MY_CLASS);
//     lv_round_fft_t * fft = (lv_round_fft_t *)obj;
//     fft->c_start = c_start;
//     fft->c_end   = c_end;
//     lv_obj_invalidate(obj);
// }

// /**********************
//  *   STATIC FUNCTIONS
//  **********************/

// static void lv_round_fft_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj)
// {
//     LV_UNUSED(class_p);
//     lv_round_fft_t * fft = (lv_round_fft_t *)obj;

//     fft->count    = LV_ROUND_FFT_DEFAULT_COUNT;
//     fft->r_in     = LV_ROUND_FFT_DEFAULT_R_IN;
//     fft->min_h    = LV_ROUND_FFT_DEFAULT_MIN_H;
//     fft->max_gain = LV_ROUND_FFT_DEFAULT_MAX_GAIN;
//     fft->rotation = LV_ROUND_FFT_DEFAULT_ROTATION;
//     fft->c_start  = LV_ROUND_FFT_COLOR_START;
//     fft->c_end    = LV_ROUND_FFT_COLOR_END;

//     fft->vals = (int16_t *)lv_malloc(sizeof(int16_t) * fft->count);
//     if(fft->vals) {
//         lv_memset(fft->vals, 0, sizeof(int16_t) * fft->count);
//     }

//     lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
//     lv_obj_set_style_bg_opa(obj, LV_OPA_TRANSP, 0);
//     lv_obj_set_style_border_opa(obj, LV_OPA_TRANSP, 0);
// }

// static void lv_round_fft_destructor(const lv_obj_class_t * class_p, lv_obj_t * obj)
// {
//     LV_UNUSED(class_p);
//     lv_round_fft_t * fft = (lv_round_fft_t *)obj;
//     if(fft->vals) {
//         lv_free(fft->vals);
//         fft->vals = NULL;
//     }
// }

// static void lv_round_fft_event(const lv_obj_class_t * class_p, lv_event_t * e)
// {
//     lv_event_code_t code = lv_event_get_code(e);
//     if(code == LV_EVENT_DRAW_MAIN) {
//         draw_fft(lv_event_get_target(e), e);
//     }

//     lv_obj_event_base(class_p, e);
// }

// static void draw_fft(lv_obj_t * obj, lv_event_t * e)
// {
//     lv_layer_t * layer = lv_event_get_layer(e);
//     if(!layer) return;

//     lv_round_fft_t * fft = (lv_round_fft_t *)obj;
//     if(!fft->vals || fft->count == 0) return;

//     lv_area_t coords;
//     lv_obj_get_coords(obj, &coords);
//     lv_coord_t w = lv_area_get_width(&coords);
//     lv_coord_t h = lv_area_get_height(&coords);
//     lv_coord_t cx = coords.x1 + w / 2;
//     lv_coord_t cy = coords.y1 + h / 2;

//     lv_coord_t avail_rad = LV_MIN(w, h) / 2;
//     lv_coord_t r_in = fft->r_in;
//     if(r_in < 0) r_in = 0;
//     if(r_in > avail_rad) r_in = avail_rad;

//     lv_coord_t min_h = fft->min_h;
//     lv_coord_t h_max = 0;
//     if(avail_rad > r_in + min_h) {
//         h_max = avail_rad - r_in - min_h;
//     }

//     int32_t step_deg = 360 / fft->count;
//     int32_t gap = LV_ROUND_FFT_DEG_GAP;
//     if(step_deg <= gap * 2) gap = 0;

//     for(uint8_t i = 0; i < fft->count; i++) {
//         int32_t deg_start = fft->rotation + i * step_deg + gap;
//         int32_t deg_end   = fft->rotation + (i + 1) * step_deg - gap;

//         int32_t val = fft->vals[i];
//         if(val < 0) val = -val;
//         int32_t scaled = val * fft->max_gain;
//         if(scaled < 0) scaled = 0;
//         if(scaled > h_max) scaled = h_max;

//         lv_coord_t r_out = r_in + min_h + scaled;

//         lv_color_t color;
//         if(h_max > 0) {
//             uint8_t mix = (uint8_t)((scaled * 255) / h_max);
//             color = lv_color_mix(fft->c_end, fft->c_start, mix);
//         } else {
//             color = fft->c_start;
//         }

//         lv_point_t poly[4];
//         int32_t x_out_l = get_cos(deg_start, r_out);
//         int32_t y_out_l = get_sin(deg_start, r_out);
//         int32_t x_in_l  = get_cos(deg_start, r_in);
//         int32_t y_in_l  = get_sin(deg_start, r_in);

//         int32_t x_out_r = get_cos(deg_end, r_out);
//         int32_t y_out_r = get_sin(deg_end, r_out);
//         int32_t x_in_r  = get_cos(deg_end, r_in);
//         int32_t y_in_r  = get_sin(deg_end, r_in);

//         poly[0].x = cx + x_out_l;
//         poly[0].y = cy + y_out_l;
//         poly[1].x = cx + x_in_l;
//         poly[1].y = cy + y_in_l;
//         poly[2].x = cx + x_in_r;
//         poly[2].y = cy + y_in_r;
//         poly[3].x = cx + x_out_r;
//         poly[3].y = cy + y_out_r;

//         lv_draw_triangle_dsc_t tri_dsc;
//         lv_draw_triangle_dsc_init(&tri_dsc);
//         tri_dsc.bg_opa = LV_OPA_COVER;
//         tri_dsc.bg_color = color;

//         /* triangle 0,1,2 */
//         tri_dsc.p[0].x = (lv_value_precise_t)poly[0].x;
//         tri_dsc.p[0].y = (lv_value_precise_t)poly[0].y;
//         tri_dsc.p[1].x = (lv_value_precise_t)poly[1].x;
//         tri_dsc.p[1].y = (lv_value_precise_t)poly[1].y;
//         tri_dsc.p[2].x = (lv_value_precise_t)poly[2].x;
//         tri_dsc.p[2].y = (lv_value_precise_t)poly[2].y;
//         lv_draw_triangle(layer, &tri_dsc);

//         /* triangle 0,2,3 */
//         tri_dsc.p[0].x = (lv_value_precise_t)poly[0].x;
//         tri_dsc.p[0].y = (lv_value_precise_t)poly[0].y;
//         tri_dsc.p[1].x = (lv_value_precise_t)poly[2].x;
//         tri_dsc.p[1].y = (lv_value_precise_t)poly[2].y;
//         tri_dsc.p[2].x = (lv_value_precise_t)poly[3].x;
//         tri_dsc.p[2].y = (lv_value_precise_t)poly[3].y;
//         lv_draw_triangle(layer, &tri_dsc);
//     }
// }

// static LV_ATTRIBUTE_FAST_MEM int32_t get_cos(int32_t deg, int32_t a)
// {
//     int32_t r = (lv_trigo_cos(deg) * a);
//     r += LV_TRIGO_SIN_MAX / 2;
//     return r >> LV_TRIGO_SHIFT;
// }

// static LV_ATTRIBUTE_FAST_MEM int32_t get_sin(int32_t deg, int32_t a)
// {
//     int32_t r = lv_trigo_sin(deg) * a;
//     r += LV_TRIGO_SIN_MAX / 2;
//     return r >> LV_TRIGO_SHIFT;
// }

