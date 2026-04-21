/**
 * @file lv_gif_private.h
 *
 */

#ifndef LV_VPG_PRIVATE_H
#define LV_VPG_PRIVATE_H

#ifdef __cplusplus
extern "C" {
#endif

/*********************
 *      INCLUDES
 *********************/
#ifdef SIMULATOR
#include "src/widgets/image/lv_image_private.h"
#else
#include "widgets/image/lv_image_private.h"
#endif
#include "lv_vpg.h"

#ifndef SIMULATOR
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "esp_jpeg_dec.h"
#endif

#if 1

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *      TYPEDEFS
 **********************/
typedef struct {
    lv_fs_res_t (*read)(void *ctx, void *buf, uint32_t size, uint32_t *out_read);
    lv_fs_res_t (*seek)(void *ctx, uint32_t pos, uint8_t whence);
    lv_fs_res_t (*tell)(void *ctx, uint32_t *pos);
    void        (*close)(void *ctx);
} vpg_io_t;

typedef struct {
    const uint8_t *base;
    uint8_t *owned_data;
    uint32_t size;
    uint32_t pos;
} vpg_mem_ctx_t;

struct lv_vpg_qoi_cache_t {
    lv_image_dsc_t dsc;
    uint8_t *data;
    uint32_t data_size;
};

#pragma pack(push, 1) // 设置结构体为1字节对齐
typedef struct {
    uint32_t offset; // 4 bytes offset
    uint32_t size;  // 2 bytes size
} vpg_item_header_t;

typedef struct {
    uint32_t magic; // 4 bytes offset
    uint32_t size;  // 2 bytes size
    uint32_t itemNum;  // 2 bytes size
    uint32_t fps:8;  // 2 bytes size
    uint32_t height:12;  // 2 bytes size
    uint32_t width:12;  // 2 bytes size
} vpg_file_header_t;

typedef struct
{
    vpg_file_header_t header;
    vpg_item_header_t item[0];
}vpg_file_t;


typedef struct
{
    // lv_fs_file_t f;
    uint8_t *frame;
    uint32_t frame_size;
    uint16_t delay_ms;
    uint16_t index;
    uint32_t width:16;
    uint32_t height:16;
    vpg_file_t *vpg;
    const vpg_io_t *io;
    void *io_ctx;
#ifndef SIMULATOR
    uint8_t *decoded_frames[4];           /* 滑动窗口解码缓冲 (16字节对齐 PSRAM) */
    uint8_t *predecoded_frames;           /* 整段动画预解码后的 RGB565 帧缓存 (PSRAM) */
    uint8_t *display_frame;               /* 当前显示帧，优先放内部 RAM */
    uint32_t decoded_size;
    uint8_t use_lvgl_decode;              /* 小图直接交给 LVGL 解码 JPEG */
    uint8_t use_predecoded_frames;        /* 是否启用整段预解码播放 */
    uint8_t use_internal_display_buf;     /* 当前显示帧是否使用内部 RAM 独立缓冲 */
    uint8_t decode_slot_count;            /* 后台可用于解码排队的槽位数 */
    volatile int display_idx;              /* LVGL 当前使用的帧槽 (0-2) */
    int ready_slots[4];                    /* 已解码待显示帧槽队列 */
    uint16_t ready_frame_idx[4];           /* 待显示帧对应的帧序号 */
    uint8_t ready_head;
    uint8_t ready_tail;
    uint8_t ready_count;
    uint16_t displayed_frame_idx;          /* 当前显示的帧序号 */
    volatile uint8_t stop_decode;
    TaskHandle_t decode_task;
    SemaphoreHandle_t decode_exit_sem;      /* 解码任务退出确认 */
    SemaphoreHandle_t source_lock;         /* 串行化 set src 和解码线程对源数据的访问 */
    SemaphoreHandle_t frame_lock;          /* 保护 display/ready 队列状态 */
    SemaphoreHandle_t pending_consumed_sem;/* 空闲解码槽计数，允许解码线程领先 1-2 帧 */
    jpeg_dec_handle_t jpeg_dec;
    jpeg_dec_io_t jpeg_io;
    jpeg_dec_header_info_t jpeg_header;
    uint16_t decode_index;
#endif
}vpg_t;
#pragma pack(pop) // 恢复对齐方式

struct lv_vpg_t {
    lv_image_t img;
    vpg_t *vpg;
    lv_timer_t * timer;
    lv_image_dsc_t imgdsc;
    uint32_t last_call;
};


/**********************
 * GLOBAL PROTOTYPES
 **********************/

/**********************
 *      MACROS
 **********************/

#endif /* LV_USE_GIF */

#ifdef __cplusplus
} /*extern "C"*/
#endif

#endif /*LV_GIF_PRIVATE_H*/
