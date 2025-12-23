/**
 * @file lv_round_fft.h
 *
 * 圆形 FFT 频谱控件（基于 LVGL9）
 */

#ifndef LV_ROUND_FFT_H
#define LV_ROUND_FFT_H

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

typedef struct lv_round_fft_t lv_round_fft_t;

LV_ATTRIBUTE_EXTERN_DATA extern const lv_obj_class_t lv_round_fft_class;

/**********************
 *  GLOBAL PROTOTYPES
 **********************/

/**
 * 创建圆形 FFT 控件
 * @param parent 父对象
 * @return 新建的控件对象
 */
lv_obj_t * lv_round_fft_create(lv_obj_t * parent);

/**
 * 设置频点个数
 * @param obj   lv_round_fft 对象
 * @param count 频点个数（>0）
 */
void lv_round_fft_set_count(lv_obj_t * obj, uint8_t count);

/**
 * 设置频点数据
 * @param obj lv_round_fft 对象
 * @param val 长度至少为 count 的数组；值范围任意，内部结合 max_gain 映射高度
 */
void lv_round_fft_set_val(lv_obj_t * obj, const int16_t * val);

/**
 * 设置内圆半径
 * @param obj  lv_round_fft 对象
 * @param r_in 内圆半径（像素）
 */
void lv_round_fft_set_inner_radius(lv_obj_t * obj, lv_coord_t r_in);

/**
 * 设置最小柱高
 * @param obj   lv_round_fft 对象
 * @param h_min 最小高度（像素）
 */
void lv_round_fft_set_min_height(lv_obj_t * obj, lv_coord_t h_min);

/**
 * 设置输入数据的最大值范围（如 0~100 或 0~255）
 * @param obj    lv_round_fft 对象
 * @param max_in 最大输入值（>0），用于线性映射到可视高度
 */
void lv_round_fft_set_val_range(lv_obj_t * obj, int16_t max_in);

/**
 * 设置起始角度
 * @param obj       lv_round_fft 对象
 * @param deg_start 起始角度（度），0 在右侧，正向逆时针
 */
void lv_round_fft_set_rotation(lv_obj_t * obj, int16_t deg_start);

/**
 * 设置整体尺寸缩放（条纹长度缩放）
 * @param obj   lv_round_fft 对象
 * @param scale 缩放百分比，100 表示 1.0，不改变；50 表示一半；200 表示两倍
 */
void lv_round_fft_set_scale(lv_obj_t * obj, int16_t scale);

/**
 * 设置颜色渐变的起止颜色
 * @param obj     lv_round_fft 对象
 * @param c_start 起始颜色（较低能量）
 * @param c_end   终止颜色（较高能量）
 */
void lv_round_fft_set_colors(lv_obj_t * obj, lv_color_t c_start, lv_color_t c_end);

/**********************
 *      MACROS
 **********************/

#ifdef __cplusplus
} /* extern "C" */
#endif

#endif /* LV_ROUND_FFT_H */


