/*
 * SPDX-FileCopyrightText: 2022-2024 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Apache-2.0
 */

/*********************
 *      INCLUDES
 *********************/

#include "../../draw/lv_image_decoder_private.h"
#include "../../draw/lv_draw_buf.h"
 
 #include "lvgl.h"

 #define QOI_IMPLEMENTATION
 #include "qoi.h"

#define LOGD(format, ...) printf("\033[0;36m" "["TAG"]" format "\033[0m\n", ##__VA_ARGS__)
#define LOGI(format, ...) printf("\033[0;32m" "["TAG"]" format "\033[0m\n", ##__VA_ARGS__)
#define LOGW(format, ...) printf("\033[0;33m" "["TAG"]" format "\033[0m\n", ##__VA_ARGS__)
#define LOGE(format, ...) printf("\033[0;31m" "["TAG"]" format "\033[0m\n", ##__VA_ARGS__)

 #define TAG "qoi_dec"
 /*********************
  *      DEFINES
  *********************/
 
 /**********************
  *      TYPEDEFS
  **********************/

// QOI解码器状态结构
typedef struct {
    image_decoder_t decoder;        // 行解码器
    lv_draw_buf_t *partial_buf;      // 部分解码缓冲区
    bool use_get_area;               // 是否使用get_area模式
    int batch_size;                  // 批处理大小（行数）
} qoi_decoder_state_t;
 
 /**********************
  *  STATIC PROTOTYPES
  **********************/
 static lv_result_t decoder_info(lv_image_decoder_t * decoder, lv_image_decoder_dsc_t * dsc, lv_image_header_t * header);
 static lv_result_t decoder_open(lv_image_decoder_t *decoder, lv_image_decoder_dsc_t *dsc);
 static lv_result_t decoder_read_line(lv_image_decoder_t *decoder, lv_image_decoder_dsc_t *dsc, lv_coord_t x, lv_coord_t y,
                                   lv_coord_t len, uint8_t *buf);
 static lv_result_t decoder_get_area(lv_image_decoder_t *decoder, lv_image_decoder_dsc_t *dsc,
                                   const lv_area_t *full_area, lv_area_t *decoded_area);
static void decoder_close(lv_image_decoder_t *dec, lv_image_decoder_dsc_t *dsc);
 static void convert_color_depth(uint8_t *img, uint32_t px_cnt);
 static int is_qoi(const uint8_t *raw_data, size_t len);
 static void decoder_cleanup(image_decoder_t *img_dec);
 static void decoder_free(image_decoder_t *img_dec);
 
 static lv_result_t qoi_decode32(uint8_t **out, uint32_t *w, uint32_t *h, const uint8_t *in, size_t insize);
 
 /**********************
  *  STATIC VARIABLES
  **********************/
 
 /**********************
  *      MACROS
  **********************/
 
 /**********************
  *   GLOBAL FUNCTIONS
  **********************/
 
 /**
  * Register the PNG decoder functions in LVGL
  */
 void lv_qoi_dec_init()
 {
    lv_image_decoder_t *dec = lv_image_decoder_create();
    lv_image_decoder_set_info_cb(dec, decoder_info);
    lv_image_decoder_set_open_cb(dec, decoder_open);
    lv_image_decoder_set_close_cb(dec, decoder_close);
    lv_image_decoder_set_get_area_cb(dec, decoder_get_area);
    dec->name = "qoi_dec";
 }
 
void lv_qoi_dec_deinit(void)
{
    lv_image_decoder_t * dec = NULL;
    while((dec = lv_image_decoder_get_next(dec)) != NULL) {
        if(dec->info_cb == decoder_info) {
            lv_image_decoder_delete(dec);
            break;
        }
    } 
}

/**********************
 *   STATIC FUNCTIONS
 **********************/
static lv_result_t qoi_decode32(uint8_t **out, uint32_t *w, uint32_t *h, const uint8_t *in, size_t insize)
{
    if (!in || !out || !w || !h) {
        return LV_RES_INV;
    }

    qoi_desc image;
    memset(&image, 0, sizeof(image));

    unsigned char *pixels = qoi_decode(in, insize, &image, 0);

    *w = image.width;
    *h = image.height;
    *out = pixels;
    if (*out == NULL) {
        return LV_RES_INV;
    }
    return LV_RES_OK;
}

static lv_draw_buf_t *_qoi_decode(const uint8_t *in, size_t insize)
{
    if (!in) {
        return NULL;
    }

    qoi_desc image;
    memset(&image, 0, sizeof(image));
    unsigned char *pixels = qoi_decode(in, insize, &image, 0);
    if (pixels == NULL)
    {
        return NULL;
    }
    
    lv_draw_buf_t *draw_buf = malloc(sizeof(lv_draw_buf_t));
    draw_buf->header.cf = image.channels==4?LV_COLOR_FORMAT_ARGB8888:LV_COLOR_FORMAT_RGB888;
    draw_buf->data = pixels;
    draw_buf->header.h = image.height;
    draw_buf->header.w = image.width;
    draw_buf->header.stride = image.width * image.channels;
    return draw_buf;
}

static lv_draw_buf_t *_qoi_decode_565(const uint8_t *in, size_t insize)
{
    if (!in) {
        return NULL;
    }

    qoi_desc image;
    memset(&image, 0, sizeof(image));
    unsigned char *pixels = qoi_decode_565(in, insize, &image, 0);
    if (pixels == NULL)
    {
        return NULL;
    }
    lv_draw_buf_t *draw_buf = malloc(sizeof(lv_draw_buf_t));
    draw_buf->header.cf = image.channels==4?LV_COLOR_FORMAT_RGB565A8:LV_COLOR_FORMAT_RGB565;
    draw_buf->data = pixels;
    draw_buf->header.h = image.height;
    draw_buf->header.w = image.width;
    draw_buf->header.stride = image.width*2;
    return draw_buf;
}

static lv_fs_res_t load_image_file(const char *filename, uint8_t **buffer, size_t *size, bool read_head)
{
    uint32_t len;
    lv_fs_file_t f;
    lv_fs_res_t res = lv_fs_open(&f, filename, LV_FS_MODE_RD);
    if (res != LV_FS_RES_OK) {
        LOGE("Failed to open file %s", filename);
        return res;
    }

    lv_fs_seek(&f, 0, LV_FS_SEEK_END);
    lv_fs_tell(&f, &len);
    lv_fs_seek(&f, 0, LV_FS_SEEK_SET);

    if (read_head && len > 1024) {
        len = 1024;
    } else if (len <= 0) {
        lv_fs_close(&f);
        return LV_FS_RES_FS_ERR;
    }

    *buffer = malloc(len);
    if (!*buffer) {
        LOGE("Failed to allocate memory for file %s", filename);
        lv_fs_close(&f);
        return LV_FS_RES_OUT_OF_MEM;
    }

    uint32_t rn = 0;
    res = lv_fs_read(&f, *buffer, len, &rn);
    lv_fs_close(&f);

    if (res != LV_FS_RES_OK || rn != len) {
        free(*buffer);
        *buffer = NULL;
        LOGE("Failed to read file %s", filename);
        return LV_FS_RES_UNKNOWN;
    }
    *size = len;

    return LV_FS_RES_OK;
}

/**
 * Get info about a PNG image
 * @param src can be file name or pointer to a C array
 * @param header store the info here
 * @return LV_RES_OK: no error; LV_RES_INV: can't get the info
 */
static lv_result_t decoder_info(lv_image_decoder_t * decoder, lv_image_decoder_dsc_t * dsc, lv_image_header_t * header)
{
    LV_UNUSED(decoder); /*Unused*/
    const void * src = dsc->src;
    lv_image_src_t src_type = dsc->src_type;       /*Get the source type*/
    if (src_type == LV_IMAGE_SRC_VARIABLE) {
        const lv_image_dsc_t *img_dsc = src;
        uint8_t *img_dsc_data = (uint8_t *)img_dsc->data;
        const uint32_t img_dsc_size = img_dsc->data_size;

        if (is_qoi(img_dsc_data, img_dsc_size) == true) {
            const uint8_t *size = ((uint8_t *)img_dsc->data) + 4;
            header->cf = LV_COLOR_FORMAT_RGB565;
            header->w = (uint16_t)((size[0] << 24) + (size[1] << 16) + (size[2] << 8) + (size[3] << 0));
            header->h = (uint16_t)((size[4] << 24) + (size[5] << 16) + (size[6] << 8) + (size[7] << 0));
            return LV_RESULT_OK;
        } else {
            return LV_RES_INV;
        }
    } else if (src_type == LV_IMAGE_SRC_FILE)
    {
        const char *fn = src;
        uint8_t *load_img_data = NULL;  /*Pointer to the loaded data. Same as the original file just loaded into the RAM*/
        size_t load_img_size;           /*Size of `load_img_data` in bytes*/
        if (load_image_file(fn, &load_img_data, &load_img_size, true) != LV_FS_RES_OK) {
            if (load_img_data) {
                free(load_img_data);
            }
            return LV_RES_INV;
        }
        if (is_qoi(load_img_data, load_img_size) == true) {
            const uint8_t *size = ((uint8_t *)load_img_data) + 4;
            header->cf = LV_COLOR_FORMAT_RGB565;
            header->w = (uint16_t)((size[0] << 24) + (size[1] << 16) + (size[2] << 8) + (size[3] << 0));
            header->h = (uint16_t)((size[4] << 24) + (size[5] << 16) + (size[6] << 8) + (size[7] << 0));
            free(load_img_data);
            return LV_RESULT_OK;
        } else {        
            free(load_img_data);
            return LV_RES_INV;
        }
    }

    return LV_RES_INV;         /*If didn't succeeded earlier then it's an error*/
}

/**
 * Open a PNG image and return the decided image
 * @param src can be file name or pointer to a C array
 * @param style style of the image object (unused now but certain formats might use it)
 * @return pointer to the decoded image or `LV_IMG_DECODER_OPEN_FAIL` if failed
 */
static lv_result_t decoder_open(lv_image_decoder_t *decoder, lv_image_decoder_dsc_t *dsc)
{

    LV_UNUSED(decoder);
    lv_result_t lv_ret = LV_RES_OK;        /*For the return values of PNG decoder functions*/

    uint8_t *img_data = NULL;
    uint32_t png_width;
    uint32_t png_height;

    if (dsc->src_type == LV_IMAGE_SRC_VARIABLE) {

        const lv_img_dsc_t *img_dsc = dsc->src;

        uint8_t *data;
        qoi_decoder_state_t *img_dec = (qoi_decoder_state_t *) dsc->user_data;
        if (img_dec == NULL) {
            img_dec =  malloc(sizeof(qoi_decoder_state_t));
            if (!img_dec) {
                return LV_RES_INV;
            }
            memset(img_dec, 0, sizeof(qoi_decoder_state_t));
            dsc->user_data = img_dec;
            img_dec->decoder.dsc_data = (uint8_t *)((lv_img_dsc_t *)(dsc->src))->data;
            img_dec->decoder.dsc_size = ((lv_img_dsc_t *)(dsc->src))->data_size;
        }

        if (qoi_565_line_decoder_init(&img_dec->decoder, 0)==1)
        {
            // 计算RGB565格式的缓冲区大小
            size_t buffer_size = img_dec->decoder.line_width * img_dec->decoder.total_lines * 2; // RGB565每像素2字节
            // 如果缓冲区大于30KB，使用get_area模式
            if (buffer_size > 30 * 1024) {
                img_dec->decoder.current_line = 0;
                img_dec->use_get_area = true;
                img_dec->batch_size = 20; // 一次解码10行
                img_dec->partial_buf = NULL;
                
                // 设置头部信息
                dsc->header.cf = LV_COLOR_FORMAT_RGB565;
                dsc->header.w = img_dec->decoder.line_width;
                dsc->header.h = img_dec->decoder.total_lines;
                dsc->header.stride = dsc->header.w * 2;
                return LV_RESULT_OK;
            } else {
                // 使用直接解码模式
                dsc->decoded = _qoi_decode_565(img_dsc->data, img_dsc->data_size);
                if (dsc->decoded == NULL) {
                    return LV_RESULT_INVALID;
                } else {
                    return LV_RESULT_OK;
                }
            }
        } else {
            return LV_RES_INV;
        }
    } else if (dsc->src_type == LV_IMAGE_SRC_FILE)
    {
        const char *fn = dsc->src;
        uint8_t *load_img_data = NULL;  /*Pointer to the loaded data. Same as the original file just loaded into the RAM*/
        size_t load_img_size;           /*Size of `load_img_data` in bytes*/

        if (load_image_file(fn, &load_img_data, &load_img_size, false) != LV_FS_RES_OK) {
            if (load_img_data) {
                free(load_img_data);
                // decoder_cleanup(img_dec);
            }
            return LV_RES_INV;
        }

        qoi_decoder_state_t *img_dec = (qoi_decoder_state_t *) dsc->user_data;
        if (img_dec == NULL) {
            img_dec =  malloc(sizeof(qoi_decoder_state_t));
            if (!img_dec) {
                return LV_RES_INV;
            }
            memset(img_dec, 0, sizeof(qoi_decoder_state_t));
            dsc->user_data = img_dec;
            img_dec->decoder.dsc_data = (uint8_t *)load_img_data;
            img_dec->decoder.dsc_size = load_img_size;
        }

        if (qoi_565_line_decoder_init(&img_dec->decoder, 0)==1) {
            // 计算RGB565格式的缓冲区大小
            size_t buffer_size = img_dec->decoder.line_width * img_dec->decoder.total_lines * 2; // RGB565每像素2字节
            
            // 如果缓冲区大于30KB，使用get_area模式
            if (buffer_size > 30 * 1024) {
                img_dec->decoder.current_line = 0;
                img_dec->use_get_area = true;
                img_dec->batch_size = 20; // 一次解码10行
                img_dec->partial_buf = NULL;
                // 设置头部信息
                dsc->header.cf = LV_COLOR_FORMAT_RGB565;
                dsc->header.w = img_dec->decoder.line_width;
                dsc->header.h = img_dec->decoder.total_lines;
                dsc->header.stride = dsc->header.w * 2;
                return LV_RESULT_OK;
            } else {
                // 使用直接解码模式
                dsc->decoded = _qoi_decode_565(load_img_data, load_img_size);
                if (dsc->decoded == NULL) {
                    return LV_RESULT_INVALID;
                } else {
                    return LV_RESULT_OK;
                }
            }
        }else{
            return LV_RES_INV;
        }
    }
    
    return LV_RES_INV;    /*If not returned earlier then it failed*/
}

static lv_result_t decoder_get_area(lv_image_decoder_t *decoder, lv_image_decoder_dsc_t *dsc,
                                   const lv_area_t *full_area, lv_area_t *decoded_area)
{
    LV_UNUSED(decoder);
    
    qoi_decoder_state_t *t_decoder = (qoi_decoder_state_t *)dsc->user_data;
    if (!t_decoder || !t_decoder->use_get_area) {
        return LV_RESULT_INVALID;
    }
    image_decoder_t *state = &t_decoder->decoder;
    
    // 如果是第一次调用，重置解码状态
    if (decoded_area->y1 == LV_COORD_MIN) {
        state->current_line = 0;
        decoded_area->x1 = 0;
        decoded_area->x2 = state->line_width-1;
        decoded_area->y1 = full_area->y1;
        decoded_area->y2 = full_area->y1 + t_decoder->batch_size - 1;
        // 确保不超出图像边界
        if (decoded_area->y2 >= state->total_lines) {
            decoded_area->y2 = state->total_lines - 1;
        }
        // 创建部分解码缓冲区
        int batch_height = decoded_area->y2 - decoded_area->y1 + 1;
        if (t_decoder->partial_buf == NULL) {
            t_decoder->partial_buf = lv_draw_buf_create(state->line_width, batch_height, 
                                                   dsc->header.cf, LV_STRIDE_AUTO);
            if (t_decoder->partial_buf == NULL) {
                return LV_RESULT_INVALID;
            }
        } else {
            // 重新调整缓冲区大小
            lv_draw_buf_t *new_buf = lv_draw_buf_reshape(t_decoder->partial_buf, 
                                                         dsc->header.cf,
                                                         state->line_width, 
                                                         batch_height, 
                                                         LV_STRIDE_AUTO);
            if (new_buf == NULL) {
                lv_draw_buf_destroy(t_decoder->partial_buf);
                t_decoder->partial_buf = lv_draw_buf_create(state->line_width, batch_height, 
                                                       dsc->header.cf, LV_STRIDE_AUTO);
                if (t_decoder->partial_buf == NULL) {
                    return LV_RESULT_INVALID;
                }
            }else{
                t_decoder->partial_buf = new_buf;
            }
        }
        
        // 设置解码缓冲区
        dsc->decoded = t_decoder->partial_buf;
        
        // 解码当前批次的行
        unsigned char *buf_ptr = (unsigned char *)t_decoder->partial_buf->data;
        for (int i = 0; i < batch_height; i++) {
            int line_num = decoded_area->y1 + i;
            if (line_num >= state->total_lines) {
                break;
            }
            
            int bytes_decoded = qoi_565_decode_line(state, 
                                                   buf_ptr + i * state->line_width * 2, 
                                                   line_num);
            if (bytes_decoded <= 0) {
                return LV_RESULT_INVALID;
            }
        }
        
        state->current_line = decoded_area->y2 + 1;
        return LV_RESULT_OK;
    }
    
    // 继续解码下一批
    decoded_area->y1 += t_decoder->batch_size;
    decoded_area->y2 += t_decoder->batch_size;
    
    // 检查是否还有更多行需要解码
    if (decoded_area->y1 > full_area->y2) {
        return LV_RESULT_INVALID; // 解码完成
    }
    
    // 确保不超出图像边界
    if (decoded_area->y2 >= state->total_lines) {
        decoded_area->y2 = state->total_lines - 1;
    }
    
    // 重新调整缓冲区大小
    int batch_height = decoded_area->y2 - decoded_area->y1 + 1;
    lv_draw_buf_t *new_buf = lv_draw_buf_reshape(t_decoder->partial_buf, 
                                                 dsc->header.cf,
                                                 state->line_width, 
                                                 batch_height, 
                                                 LV_STRIDE_AUTO);
    if (new_buf == NULL) {
        lv_draw_buf_destroy(t_decoder->partial_buf);
        t_decoder->partial_buf = lv_draw_buf_create(state->line_width, batch_height, 
                                               dsc->header.cf, LV_STRIDE_AUTO);
        if (t_decoder->partial_buf == NULL) {
            return LV_RESULT_INVALID;
        }
    }else {
        t_decoder->partial_buf = new_buf;
    }
    
    // 设置解码缓冲区
    dsc->decoded = t_decoder->partial_buf;
    
    // 解码当前批次的行
    unsigned char *buf_ptr = (unsigned char *)t_decoder->partial_buf->data;
    for (int i = 0; i < batch_height; i++) {
        int line_num = decoded_area->y1 + i;
        if (line_num >= state->total_lines) {
            break;
        }
        
        int bytes_decoded = qoi_565_decode_line(state, 
                                               buf_ptr + i * state->line_width * 2, 
                                               line_num);
        if (bytes_decoded <= 0) {
            return LV_RESULT_INVALID;
        }
    }
    
    state->current_line = decoded_area->y2 + 1;
    return LV_RESULT_OK;
}

static void decoder_close(lv_image_decoder_t *decoder, lv_image_decoder_dsc_t *dsc)
{
    LV_UNUSED(decoder);
    /*Free all allocated data*/
    qoi_decoder_state_t *img_dec = (qoi_decoder_state_t *) dsc->user_data;

    if (!img_dec) {
        return;
    }

    // 检查是否是get_area模式，需要清理额外资源
    if (img_dec->use_get_area) {
        if (img_dec->partial_buf) {
            lv_draw_buf_destroy(img_dec->partial_buf);
        }
        free(img_dec);
        dsc->user_data = NULL;
    } else {
        // 直接解码模式，清理decoded缓冲区
        if (dsc->decoded) {
            lv_draw_buf_t *buf = dsc->decoded;
            if (buf->data) {
                free(buf->data);
            }
            free(buf);
            dsc->decoded = NULL;
        }
        free(img_dec);
        dsc->user_data = NULL;
    }    
}

static int is_qoi(const uint8_t *raw_data, size_t len)
{
    const uint8_t magic[] = {0x71, 0x6F, 0x69, 0x66};
    if (len < sizeof(magic)) {
        return false;
    }
    return memcmp(magic, raw_data, sizeof(magic)) == 0;
}

static void decoder_free(image_decoder_t *img_dec)
{
    // if (img_dec->frame_cache) {
    //     free(img_dec->frame_cache);
    // }
    // if (img_dec->frame_base_array) {
    //     free(img_dec->frame_base_array);
    // }
    // if (img_dec->frame_base_offset) {
    //     free(img_dec->frame_base_offset);
    // }
}

static void decoder_cleanup(image_decoder_t *img_dec)
{
    if (! img_dec) {
        return;
    }

    decoder_free(img_dec);
    free(img_dec);
}