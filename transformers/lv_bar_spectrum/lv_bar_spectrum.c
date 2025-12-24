#include "lvgl.h"
#include "lv_bar_spectrum.h"

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

/**********************
 *      DEFINES
 **********************/

#define LV_BAR_SPECTRUM_DEFAULT_COUNT         32
#define LV_BAR_SPECTRUM_DEFAULT_MAX_HEIGHT    120
#define LV_BAR_SPECTRUM_DEFAULT_MIN_HEIGHT    5
#define LV_BAR_SPECTRUM_DEFAULT_PEAK_HEIGHT   2
#define LV_BAR_SPECTRUM_DEFAULT_PEAK_DECAY    2
#define LV_BAR_SPECTRUM_DEFAULT_REFLECT_SCALE 60
#define LV_BAR_SPECTRUM_DEFAULT_REFLECT_OPA   LV_OPA_40
#define LV_BAR_SPECTRUM_DEFAULT_BAR_WIDTH     8
#define LV_BAR_SPECTRUM_DEFAULT_BAR_SPACING   2

#define MY_CLASS (&lv_bar_spectrum_class)

/**********************
 *      TYPEDEFS
 **********************/

typedef struct {
    lv_obj_t  *bar;          /* 主柱 */
    lv_obj_t  *bar_reflect;  /* 倒影柱 */
    lv_obj_t  *peak;         /* 顶部小横条 */
    lv_obj_t  *peak_reflect; /* 倒影上的小横条 */
    lv_coord_t peak_value;   /* 当前峰值（像素高度） */
} band_t;

struct lv_bar_spectrum_t {
    lv_obj_t obj; /* 必须把 lv_obj_t 作为首成员以匹配 LVGL 的实例布局 */
    
    uint8_t    count;              /* 频点数量 */
    band_t    *bands;              /* 频点数组 */
    int16_t   *vals;               /* 当前值数组 */
    
    lv_coord_t baseline_y;         /* 主柱基线 y（频谱居中时的中线） */
    lv_coord_t center_x;           /* 水平中心 */
    lv_coord_t bar_width;          /* 每个柱子的宽度 */
    lv_coord_t bar_spacing;        /* 柱子之间的间距 */
    
    lv_coord_t max_height;         /* 柱状条最大高度 */
    lv_coord_t min_height;         /* 柱状条最小可见高度 */
    lv_coord_t peak_height;        /* 小横条自身厚度（高度） */
    lv_coord_t peak_decay_step;    /* 小横条每次下落步长 */
    
    uint8_t    reflect_scale;      /* 倒影高度比例(百分比) */
    lv_opa_t   reflect_opa;        /* 倒影透明度 */
    
    lv_color_t color_start;        /* 左侧颜色 */
    lv_color_t color_end;          /* 右侧颜色 */
    
    lv_timer_t *timer;             /* 自动更新定时器 */
    bool        auto_update_enable; /* 是否启用自动更新 */
};

/**********************
 *  STATIC PROTOTYPES
 **********************/

static void lv_bar_spectrum_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj);
static void lv_bar_spectrum_destructor(const lv_obj_class_t * class_p, lv_obj_t * obj);
static void lv_bar_spectrum_event(const lv_obj_class_t * class_p, lv_event_t * e);

static void _spec_clamp_height(lv_bar_spectrum_t * spec, lv_coord_t * h);
static void _spec_update_band(lv_bar_spectrum_t * spec, uint8_t idx);
static void _spec_create_ui(lv_bar_spectrum_t * spec);
static lv_obj_t * _spec_create_rect(lv_obj_t * parent,
                                    lv_coord_t w,
                                    lv_coord_t h,
                                    lv_coord_t x,
                                    lv_coord_t y,
                                    lv_color_t color,
                                    lv_opa_t opa);
static void _spec_timer_cb(lv_timer_t * timer);

/**********************
 *  GLOBAL VARIABLES
 **********************/

const lv_obj_class_t lv_bar_spectrum_class = {
    .base_class     = &lv_obj_class,
    .constructor_cb = lv_bar_spectrum_constructor,
    .destructor_cb  = lv_bar_spectrum_destructor,
    .event_cb       = lv_bar_spectrum_event,
    .width_def      = LV_SIZE_CONTENT,
    .height_def     = LV_SIZE_CONTENT,
    .instance_size  = sizeof(lv_bar_spectrum_t),
    .theme_inheritable = LV_OBJ_CLASS_THEME_INHERITABLE_FALSE,
    .name           = "lv_bar_spectrum",
};

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

lv_obj_t * lv_bar_spectrum_create(lv_obj_t * parent)
{
    lv_obj_t * obj = lv_obj_class_create_obj(MY_CLASS, parent);
    lv_obj_class_init_obj(obj);
    return obj;
}

void lv_bar_spectrum_set_count(lv_obj_t * obj, uint8_t count)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    if(count == 0) return;
    
    lv_bar_spectrum_t * spec = (lv_bar_spectrum_t *)obj;
    
    /* 如果数量改变，需要重新创建 UI */
    if(spec->count != count) {
        /* 清理旧的 */
        if(spec->bands) {
            for(uint8_t i = 0; i < spec->count; i++) {
                if(spec->bands[i].bar) lv_obj_del(spec->bands[i].bar);
                if(spec->bands[i].bar_reflect) lv_obj_del(spec->bands[i].bar_reflect);
                if(spec->bands[i].peak) lv_obj_del(spec->bands[i].peak);
                if(spec->bands[i].peak_reflect) lv_obj_del(spec->bands[i].peak_reflect);
            }
            lv_free(spec->bands);
            spec->bands = NULL;
        }
        if(spec->vals) {
            lv_free(spec->vals);
            spec->vals = NULL;
        }
        
        spec->count = count;
        spec->bands = (band_t *)lv_malloc(sizeof(band_t) * count);
        spec->vals = (int16_t *)lv_malloc(sizeof(int16_t) * count);
        
        if(spec->bands) {
            lv_memzero(spec->bands, sizeof(band_t) * count);
        }
        if(spec->vals) {
            lv_memzero(spec->vals, sizeof(int16_t) * count);
        }
        
        /* 重新创建 UI */
        _spec_create_ui(spec);
    }
}

void lv_bar_spectrum_set_val(lv_obj_t * obj, const int16_t * val)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    if(!val) return;
    
    lv_bar_spectrum_t * spec = (lv_bar_spectrum_t *)obj;
    if(!spec->vals || spec->count == 0) return;
    
    /* 复制数据 */
    for(uint8_t i = 0; i < spec->count; i++) {
        spec->vals[i] = val[i];
        _spec_update_band(spec, i);
    }
}

void lv_bar_spectrum_set_max_height(lv_obj_t * obj, lv_coord_t max_h)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_bar_spectrum_t * spec = (lv_bar_spectrum_t *)obj;
    spec->max_height = max_h;
}

void lv_bar_spectrum_set_min_height(lv_obj_t * obj, lv_coord_t min_h)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_bar_spectrum_t * spec = (lv_bar_spectrum_t *)obj;
    spec->min_height = min_h;
}

void lv_bar_spectrum_set_reflect_scale(lv_obj_t * obj, uint8_t scale)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_bar_spectrum_t * spec = (lv_bar_spectrum_t *)obj;
    if(scale > 100) scale = 100;
    spec->reflect_scale = scale;
}

void lv_bar_spectrum_set_reflect_opa(lv_obj_t * obj, lv_opa_t opa)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_bar_spectrum_t * spec = (lv_bar_spectrum_t *)obj;
    spec->reflect_opa = opa;
    
    /* 更新现有倒影的透明度 */
    if(spec->bands) {
        for(uint8_t i = 0; i < spec->count; i++) {
            if(spec->bands[i].bar_reflect) {
                lv_obj_set_style_bg_opa(spec->bands[i].bar_reflect, opa, 0);
            }
            if(spec->bands[i].peak_reflect) {
                lv_obj_set_style_bg_opa(spec->bands[i].peak_reflect, opa, 0);
            }
        }
    }
}

void lv_bar_spectrum_set_peak_decay_step(lv_obj_t * obj, lv_coord_t step)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_bar_spectrum_t * spec = (lv_bar_spectrum_t *)obj;
    spec->peak_decay_step = step;
}

void lv_bar_spectrum_set_colors(lv_obj_t * obj, lv_color_t c_start, lv_color_t c_end)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_bar_spectrum_t * spec = (lv_bar_spectrum_t *)obj;
    spec->color_start = c_start;
    spec->color_end = c_end;
    
    /* 更新现有柱子的颜色 */
    if(spec->bands) {
        for(uint8_t i = 0; i < spec->count; i++) {
            uint8_t mix = (uint8_t)((i * 255) / ((spec->count > 1) ? (spec->count - 1) : 1));
            lv_color_t bar_color = lv_color_mix(spec->color_end, spec->color_start, mix);
            
            if(spec->bands[i].bar) {
                lv_obj_set_style_bg_color(spec->bands[i].bar, bar_color, 0);
            }
            if(spec->bands[i].bar_reflect) {
                lv_obj_set_style_bg_color(spec->bands[i].bar_reflect, bar_color, 0);
            }
            if(spec->bands[i].peak) {
                lv_obj_set_style_bg_color(spec->bands[i].peak, bar_color, 0);
            }
            if(spec->bands[i].peak_reflect) {
                lv_obj_set_style_bg_color(spec->bands[i].peak_reflect, bar_color, 0);
            }
        }
    }
}

void lv_bar_spectrum_set_bar_width(lv_obj_t * obj, lv_coord_t width)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_bar_spectrum_t * spec = (lv_bar_spectrum_t *)obj;
    spec->bar_width = width;
    /* 需要重新创建 UI */
    if(spec->count > 0) {
        _spec_create_ui(spec);
    }
}

void lv_bar_spectrum_set_bar_spacing(lv_obj_t * obj, lv_coord_t spacing)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_bar_spectrum_t * spec = (lv_bar_spectrum_t *)obj;
    spec->bar_spacing = spacing;
    /* 需要重新创建 UI */
    if(spec->count > 0) {
        _spec_create_ui(spec);
    }
}

void lv_bar_spectrum_set_baseline_y(lv_obj_t * obj, lv_coord_t y)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_bar_spectrum_t * spec = (lv_bar_spectrum_t *)obj;
    spec->baseline_y = y;
    /* 更新所有柱子的位置 */
    if(spec->bands && spec->vals) {
        for(uint8_t i = 0; i < spec->count; i++) {
            _spec_update_band(spec, i);
        }
    }
}

void lv_bar_spectrum_set_auto_update(lv_obj_t * obj, bool enable, uint32_t period_ms)
{
    LV_ASSERT_OBJ(obj, MY_CLASS);
    lv_bar_spectrum_t * spec = (lv_bar_spectrum_t *)obj;
    
    if(enable && !spec->timer) {
        spec->timer = lv_timer_create(_spec_timer_cb, period_ms, spec);
        spec->auto_update_enable = true;
    } else if(!enable && spec->timer) {
        lv_timer_del(spec->timer);
        spec->timer = NULL;
        spec->auto_update_enable = false;
    } else if(enable && spec->timer) {
        lv_timer_set_period(spec->timer, period_ms);
    }
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static void lv_bar_spectrum_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj)
{
    LV_UNUSED(class_p);
    lv_bar_spectrum_t * spec = (lv_bar_spectrum_t *)obj;
    
    /* 初始化默认值 */
    spec->count = LV_BAR_SPECTRUM_DEFAULT_COUNT;
    spec->bands = NULL;
    spec->vals = NULL;
    
    spec->baseline_y = LV_VER_RES / 2;
    spec->center_x = LV_HOR_RES / 2;
    spec->bar_width = LV_BAR_SPECTRUM_DEFAULT_BAR_WIDTH;
    spec->bar_spacing = LV_BAR_SPECTRUM_DEFAULT_BAR_SPACING;
    
    spec->max_height = LV_BAR_SPECTRUM_DEFAULT_MAX_HEIGHT;
    spec->min_height = LV_BAR_SPECTRUM_DEFAULT_MIN_HEIGHT;
    spec->peak_height = LV_BAR_SPECTRUM_DEFAULT_PEAK_HEIGHT;
    spec->peak_decay_step = LV_BAR_SPECTRUM_DEFAULT_PEAK_DECAY;
    
    spec->reflect_scale = LV_BAR_SPECTRUM_DEFAULT_REFLECT_SCALE;
    spec->reflect_opa = LV_BAR_SPECTRUM_DEFAULT_REFLECT_OPA;
    
    spec->color_start = lv_color_hex(0x2E7DFF);
    spec->color_end = lv_color_hex(0xFF4081);
    
    spec->timer = NULL;
    spec->auto_update_enable = false;
    
    /* 设置控件样式 */
    lv_obj_remove_style_all(obj);
    lv_obj_set_style_bg_opa(obj, LV_OPA_TRANSP, 0);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_CLICKABLE | LV_OBJ_FLAG_SCROLLABLE);
    
    /* 分配内存并创建 UI */
    spec->bands = (band_t *)lv_malloc(sizeof(band_t) * spec->count);
    spec->vals = (int16_t *)lv_malloc(sizeof(int16_t) * spec->count);
    
    if(spec->bands) {
        lv_memzero(spec->bands, sizeof(band_t) * spec->count);
    }
    if(spec->vals) {
        lv_memzero(spec->vals, sizeof(int16_t) * spec->count);
    }
    
    _spec_create_ui(spec);
}

static void lv_bar_spectrum_destructor(const lv_obj_class_t * class_p, lv_obj_t * obj)
{
    LV_UNUSED(class_p);
    lv_bar_spectrum_t * spec = (lv_bar_spectrum_t *)obj;
    
    /* 删除定时器 */
    if(spec->timer) {
        lv_timer_del(spec->timer);
        spec->timer = NULL;
    }
    
    /* 内部的子对象在 lv 的删除流程中已被递归删除（obj_delete_core 在调用本析构函数前会先删除 children），
     * 这里不要再次调用 lv_obj_del，以避免重复删除导致的警告/崩溃。
     * 仅释放本结构分配的辅助内存。 */
    if(spec->bands) {
        lv_free(spec->bands);
        spec->bands = NULL;
    }
    
    if(spec->vals) {
        lv_free(spec->vals);
        spec->vals = NULL;
    }
}

static void lv_bar_spectrum_event(const lv_obj_class_t * class_p, lv_event_t * e)
{
    LV_UNUSED(class_p);
    lv_event_code_t code = lv_event_get_code(e);
    
    if(code == LV_EVENT_SIZE_CHANGED) {
        lv_obj_t * obj = lv_event_get_target(e);
        lv_bar_spectrum_t * spec = (lv_bar_spectrum_t *)obj;
        
        /* 更新中心位置和基线 */
    lv_area_t coords;
    lv_obj_get_coords(obj, &coords);
    /* 使用相对（本地）坐标——以控件左上角为原点计算中心和基线，
     * 避免将屏幕绝对坐标（coords.x1/y1）带入导致子对象位置偏移到控件外 */
    spec->center_x = lv_area_get_width(&coords) / 2;
    spec->baseline_y = lv_area_get_height(&coords) / 2;
        
        /* 重新创建 UI */
        if(spec->count > 0) {
            _spec_create_ui(spec);
        }
    }
    
    lv_obj_event_base(class_p, e);
}

static void _spec_clamp_height(lv_bar_spectrum_t * spec, lv_coord_t * h)
{
    if(*h < spec->min_height) *h = spec->min_height;
    if(*h > spec->max_height) *h = spec->max_height;
}

static void _spec_update_band(lv_bar_spectrum_t * spec, uint8_t idx)
{
    if(idx >= spec->count) return;
    if(!spec->bands || !spec->vals) return;
    
    band_t *band = &spec->bands[idx];
    if(!band->bar || !band->bar_reflect || !band->peak || !band->peak_reflect) return;
    
    lv_coord_t cur_h = spec->vals[idx];
    _spec_clamp_height(spec, &cur_h);
    
    /* 主柱：自 baseline 向上增长 */
    lv_obj_set_height(band->bar, cur_h);
    lv_obj_set_y(band->bar, spec->baseline_y - cur_h);
    
    /* 倒影柱：在 baseline 下方，按比例缩短，增加透明度 */
    lv_coord_t ref_h = 0;
    if(spec->reflect_scale > 0) {
        ref_h = (cur_h * spec->reflect_scale) / 100;
        if(ref_h < 1) ref_h = 1;
    }
    lv_obj_set_height(band->bar_reflect, ref_h);
    lv_obj_set_y(band->bar_reflect, spec->baseline_y + 2);
    
    /* 峰值小横条逻辑 */
    lv_coord_t peak_h = band->peak_value;
    if(cur_h > peak_h) {
        peak_h = cur_h;
    } else if(cur_h < peak_h) {
        if(peak_h > 0) {
            if(peak_h > spec->peak_decay_step) {
                peak_h -= spec->peak_decay_step;
            } else {
                peak_h = 0;
            }
        }
    }
    band->peak_value = peak_h;
    
    /* 顶部小横条位置 */
    lv_coord_t peak_y = spec->baseline_y - peak_h - spec->peak_height;
    lv_obj_set_y(band->peak, peak_y);
    
    /* 倒影上的小横条 */
    lv_coord_t peak_ref_h = 0;
    if(spec->reflect_scale > 0 && peak_h > 0) {
        peak_ref_h = (peak_h * spec->reflect_scale) / 100;
    }
    lv_coord_t pk_ref_y = spec->baseline_y + 2 + peak_ref_h;
    lv_obj_set_y(band->peak_reflect, pk_ref_y);
}

static lv_obj_t * _spec_create_rect(lv_obj_t * parent,
                                    lv_coord_t w,
                                    lv_coord_t h,
                                    lv_coord_t x,
                                    lv_coord_t y,
                                    lv_color_t color,
                                    lv_opa_t opa)
{
    lv_obj_t *obj = lv_obj_create(parent);
    lv_obj_remove_style_all(obj);
    lv_obj_set_size(obj, w, h);
    lv_obj_set_style_bg_color(obj, color, 0);
    lv_obj_set_style_bg_opa(obj, opa, 0);
    lv_obj_set_style_radius(obj, 0, 0);
    lv_obj_set_style_border_width(obj, 0, 0);
    lv_obj_set_style_pad_all(obj, 0, 0);
    lv_obj_clear_flag(obj, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_pos(obj, x, y);
    return obj;
}

static void _spec_create_ui(lv_bar_spectrum_t * spec)
{
    if(!spec->bands || spec->count == 0) return;
    
    lv_obj_t *root = (lv_obj_t *)spec;
    
    /* 清理旧的 UI */
    for(uint8_t i = 0; i < spec->count; i++) {
        if(spec->bands[i].bar) lv_obj_del(spec->bands[i].bar);
        if(spec->bands[i].bar_reflect) lv_obj_del(spec->bands[i].bar_reflect);
        if(spec->bands[i].peak) lv_obj_del(spec->bands[i].peak);
        if(spec->bands[i].peak_reflect) lv_obj_del(spec->bands[i].peak_reflect);
        spec->bands[i].bar = NULL;
        spec->bands[i].bar_reflect = NULL;
        spec->bands[i].peak = NULL;
        spec->bands[i].peak_reflect = NULL;
        spec->bands[i].peak_value = spec->min_height;
    }
    
    /* 计算左右对称排布 */
    lv_coord_t total_width = spec->count * spec->bar_width +
                             (spec->count - 1) * spec->bar_spacing;
    lv_coord_t start_x = spec->center_x - total_width / 2;
    
    for(uint8_t i = 0; i < spec->count; i++) {
        lv_coord_t x = start_x + i * (spec->bar_width + spec->bar_spacing);
        
        /* 针对当前频点计算渐变色 */
        uint8_t mix = (spec->count > 1) ? (uint8_t)((i * 255) / (spec->count - 1)) : 0;
        lv_color_t bar_color = lv_color_mix(spec->color_end, spec->color_start, mix);
        
        /* 主柱 */
        lv_obj_t *bar = _spec_create_rect(root,
                                          spec->bar_width,
                                          spec->min_height,
                                          x,
                                          spec->baseline_y - spec->min_height,
                                          bar_color,
                                          LV_OPA_COVER);
        
        /* 倒影柱 */
        lv_coord_t ref_h = 0;
        if(spec->reflect_scale > 0) {
            ref_h = (spec->min_height * spec->reflect_scale) / 100;
            if(ref_h < 1) ref_h = 1;
        }
        lv_obj_t *ref = _spec_create_rect(root,
                                          spec->bar_width,
                                          ref_h,
                                          x,
                                          spec->baseline_y + 2,
                                          bar_color,
                                          spec->reflect_opa);
        
        /* 顶部小横条 */
        lv_obj_t *pk = _spec_create_rect(root,
                                         spec->bar_width,
                                         spec->peak_height,
                                         x - 1,
                                         spec->baseline_y - spec->min_height - spec->peak_height,
                                         bar_color,
                                         LV_OPA_COVER);
        
        /* 倒影小横条 */
        lv_obj_t *pk_ref = _spec_create_rect(root,
                                             spec->bar_width,
                                             spec->peak_height,
                                             x - 1,
                                             spec->baseline_y + 2,
                                             bar_color,
                                             spec->reflect_opa);
        
        spec->bands[i].bar = bar;
        spec->bands[i].bar_reflect = ref;
        spec->bands[i].peak = pk;
        spec->bands[i].peak_reflect = pk_ref;
        spec->bands[i].peak_value = spec->min_height;
    }
    
    /* 更新所有频点 */
    if(spec->vals) {
        for(uint8_t i = 0; i < spec->count; i++) {
            _spec_update_band(spec, i);
        }
    }
}

static void _spec_timer_cb(lv_timer_t * timer)
{
    lv_bar_spectrum_t * spec = (lv_bar_spectrum_t *)lv_timer_get_user_data(timer);
    if(!spec || !spec->auto_update_enable) return;
    
#ifdef SIMULATOR
    /* 模拟数据 */
    if(spec->vals) {
        for(uint8_t i = 0; i < spec->count; i++) {
            spec->vals[i] = rand() % (spec->max_height + 1);
            _spec_update_band(spec, i);
        }
    }
#else
    /* 实际使用时，应该通过事件或其他方式获取 FFT 数据 */
    /* 这里暂时不做处理，由外部调用 lv_bar_spectrum_set_val 来更新 */
#endif
}

