/**
 * @file lv_bar_spectrum.h
 *
 * 柱状频谱控件（基于 LVGL9）
 * 支持倒影效果和峰值显示
 */

#ifndef LV_BAR_SPECTRUM_H
#define LV_BAR_SPECTRUM_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#include "lvgl.h"

/**********************
 *      TYPEDEFS
 **********************/

typedef struct lv_bar_spectrum_t lv_bar_spectrum_t;

LV_ATTRIBUTE_EXTERN_DATA extern const lv_obj_class_t lv_bar_spectrum_class;

/**********************
 *  GLOBAL PROTOTYPES
 **********************/

/**
 * 创建柱状频谱控件
 * @param parent 父对象
 * @return 新建的控件对象
 */
lv_obj_t * lv_bar_spectrum_create(lv_obj_t * parent);

/**
 * 设置频点个数
 * @param obj   lv_bar_spectrum 对象
 * @param count 频点个数（>0，建议不超过 64）
 */
void lv_bar_spectrum_set_count(lv_obj_t * obj, uint8_t count);

/**
 * 设置频点数据
 * @param obj lv_bar_spectrum 对象
 * @param val 长度至少为 count 的数组；值范围建议 0~max_height
 */
void lv_bar_spectrum_set_val(lv_obj_t * obj, const int16_t * val);

/**
 * 设置柱状条最大高度
 * @param obj      lv_bar_spectrum 对象
 * @param max_h    最大高度（像素）
 */
void lv_bar_spectrum_set_max_height(lv_obj_t * obj, lv_coord_t max_h);

/**
 * 设置柱状条最小高度
 * @param obj      lv_bar_spectrum 对象
 * @param min_h    最小高度（像素）
 */
void lv_bar_spectrum_set_min_height(lv_obj_t * obj, lv_coord_t min_h);

/**
 * 设置倒影高度比例
 * @param obj   lv_bar_spectrum 对象
 * @param scale 倒影高度比例（百分比，0-100），0 表示不显示倒影
 */
void lv_bar_spectrum_set_reflect_scale(lv_obj_t * obj, uint8_t scale);

/**
 * 设置倒影透明度
 * @param obj lv_bar_spectrum 对象
 * @param opa 透明度（LV_OPA_*）
 */
void lv_bar_spectrum_set_reflect_opa(lv_obj_t * obj, lv_opa_t opa);

/**
 * 设置峰值衰减步长
 * @param obj lv_bar_spectrum 对象
 * @param step 每次衰减的像素数
 */
void lv_bar_spectrum_set_peak_decay_step(lv_obj_t * obj, lv_coord_t step);

/**
 * 设置颜色渐变的起止颜色
 * @param obj     lv_bar_spectrum 对象
 * @param c_start 起始颜色（左侧）
 * @param c_end   终止颜色（右侧）
 */
void lv_bar_spectrum_set_colors(lv_obj_t * obj, lv_color_t c_start, lv_color_t c_end);

/**
 * 设置柱状条宽度
 * @param obj lv_bar_spectrum 对象
 * @param width 每个柱子的宽度（像素）
 */
void lv_bar_spectrum_set_bar_width(lv_obj_t * obj, lv_coord_t width);

/**
 * 设置柱状条间距
 * @param obj lv_bar_spectrum 对象
 * @param spacing 柱子之间的间距（像素）
 */
void lv_bar_spectrum_set_bar_spacing(lv_obj_t * obj, lv_coord_t spacing);

/**
 * 设置基线位置（频谱中心线）
 * @param obj lv_bar_spectrum 对象
 * @param y 基线 y 坐标
 */
void lv_bar_spectrum_set_baseline_y(lv_obj_t * obj, lv_coord_t y);

/**
 * 启用/禁用自动定时器更新（用于模拟数据）
 * @param obj     lv_bar_spectrum 对象
 * @param enable  是否启用
 * @param period_ms 定时器周期（毫秒），仅在启用时有效
 */
void lv_bar_spectrum_set_auto_update(lv_obj_t * obj, bool enable, uint32_t period_ms);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LV_BAR_SPECTRUM_H */

