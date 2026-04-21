#ifndef SIMULATOR
#include "misc/lv_timer_private.h"
#include "core/lv_obj_class_private.h"
#include "widgets/image/lv_image_private.h"
#include "esp_heap_caps.h"
#include "esp_log.h"
#else
#include "src/misc/lv_timer_private.h"
#include "src/core/lv_obj_class_private.h"
#include "src/widgets/image/lv_image_private.h"
#endif
#include "lv_vpg_private.h"
#include <string.h>
#include <stdio.h>
/*********************
 *      DEFINES
 *********************/
#define MY_CLASS (&lv_vpg_class)

#ifndef SIMULATOR
#define VPG_COLOR_BPP 2
#define VPG_LVGL_DECODE_HEIGHT_THRESHOLD 100
static const char *TAG = "lv_vpg";

static void vpg_free_decoded_frames(vpg_t *vpg)
{
    if(!vpg) return;

    if (vpg->decoded_frames[0]) { heap_caps_free(vpg->decoded_frames[0]); vpg->decoded_frames[0] = NULL; }
    if (vpg->decoded_frames[1]) { heap_caps_free(vpg->decoded_frames[1]); vpg->decoded_frames[1] = NULL; }
    if (vpg->decoded_frames[2]) { heap_caps_free(vpg->decoded_frames[2]); vpg->decoded_frames[2] = NULL; }
}
#endif

/**********************
 *      TYPEDEFS
 **********************/

 /**********************
 *  STATIC PROTOTYPES
 **********************/
static lv_fs_res_t vpg_file_read(void *ctx, void *buf, uint32_t size, uint32_t *out_read);
static lv_fs_res_t vpg_file_seek(void *ctx, uint32_t pos, uint8_t whence);
static lv_fs_res_t vpg_file_tell(void *ctx, uint32_t *pos);
static void vpg_file_close(void *ctx);
static lv_fs_res_t vpg_mem_read(void *ctx, void *buf, uint32_t size, uint32_t *out_read);
static lv_fs_res_t vpg_mem_seek(void *ctx, uint32_t pos, uint8_t whence);
static lv_fs_res_t vpg_mem_tell(void *ctx, uint32_t *pos);
static void vpg_mem_close(void *ctx);
static bool lv_vpg_qoi_parse_header(const uint8_t *data, uint32_t data_size, uint32_t *w, uint32_t *h);

static const vpg_io_t VPG_MEM_IO = {
    .read = vpg_mem_read,
    .seek = vpg_mem_seek,
    .tell = vpg_mem_tell,
    .close = vpg_mem_close,
};

static const vpg_io_t VPG_FILE_IO = {
    .read = vpg_file_read,
    .seek = vpg_file_seek,
    .tell = vpg_file_tell,
    .close = vpg_file_close,
};

static void lv_vpg_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj);
static void lv_vpg_destructor(const lv_obj_class_t * class_p, lv_obj_t * obj);
static void next_frame_task_cb(lv_timer_t * t);
static bool vpg_obj_can_consume(lv_obj_t *obj);
static void vpg_close(vpg_t *vpg);
static bool vpg_reset_source(vpg_t *vpg, const void *src);
#ifndef SIMULATOR
static bool vpg_stop_decode_task(vpg_t *vpg);
static void vpg_decode_task(void *arg);
static bool vpg_decode_frame_to_rgb565(vpg_t *vpg, uint16_t frame_idx, uint8_t *out, uint32_t out_size);
static void vpg_drain_pending_sem(vpg_t *vpg);
#endif
static void vpg_load_frame(vpg_t *vpg, uint8_t *frame);
static int vpg_get_frame(vpg_t *vpg);
static vpg_t *vpg_open(const void *src);
static void vpg_release_source_data(vpg_t *vpg);
static void vpg_reset_runtime_state(vpg_t *vpg);


/**********************
 *  STATIC VARIABLES
 **********************/

const lv_obj_class_t lv_vpg_class = {
    .constructor_cb = lv_vpg_constructor,
    .destructor_cb = lv_vpg_destructor,
    .instance_size = sizeof(lv_vpg_t),
    .base_class = &lv_image_class,
    // .name = "vpg",
};

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

lv_obj_t * lv_vpg_create(lv_obj_t * parent)
{

    LV_LOG_INFO("begin");
    lv_obj_t * obj = lv_obj_class_create_obj(MY_CLASS, parent);
    lv_obj_class_init_obj(obj);
    return obj;
}

void lv_vpg_set_img(lv_obj_t * obj, const void * img)
{
    lv_vpg_t * vpgobj = (lv_vpg_t *) obj;
    vpg_t *vpg = vpgobj->vpg;
    lv_timer_pause(vpgobj->timer);

    if(vpg != NULL) {
        lv_image_cache_drop(lv_image_get_src(obj));
        if(!vpg_reset_source(vpg, NULL)) {
            lv_obj_send_event(obj, LV_EVENT_CANCEL, NULL);
            LV_LOG_WARN("Couldn't reset the VPG source");
            return;
        }
        vpgobj->imgdsc.data = NULL;
    }

    lv_image_set_src(obj, img);

}

void lv_vpg_set_src(lv_obj_t * obj, const void * src)
{
    lv_vpg_t * vpgobj = (lv_vpg_t *) obj;
    vpg_t *vpg = vpgobj->vpg;
    lv_timer_pause(vpgobj->timer);

    if(vpg != NULL) {
        lv_image_cache_drop(lv_image_get_src(obj));
        if(!vpg_reset_source(vpg, src)) {
            vpgobj->imgdsc.data = NULL;
            lv_image_set_src(obj, NULL);
            lv_obj_send_event(obj, LV_EVENT_CANCEL, NULL);
            LV_LOG_WARN("Couldn't load the source");
            return;
        }
    }
    else if(src != NULL) {
        vpg = vpg_open(src);
        if(vpg == NULL) {
            lv_obj_send_event(obj, LV_EVENT_CANCEL, NULL);
            LV_LOG_WARN("Couldn't load the source");
            return;
        }
        vpgobj->vpg = vpg;
    }

    if (src == NULL || vpgobj->vpg == NULL || vpgobj->vpg->vpg == NULL) {
        vpgobj->imgdsc.data = NULL;
        lv_image_set_src(obj, NULL);
        return;
    }

    vpg = vpgobj->vpg;
#ifndef SIMULATOR
    if (vpg->use_lvgl_decode) {
        vpgobj->imgdsc.data = vpg->frame;
        vpgobj->imgdsc.header.magic = LV_IMAGE_HEADER_MAGIC;
        vpgobj->imgdsc.header.flags = LV_IMAGE_FLAGS_MODIFIABLE;
        vpgobj->imgdsc.header.cf = LV_COLOR_FORMAT_RAW;
        vpgobj->imgdsc.header.h = vpg->height;
        vpgobj->imgdsc.header.w = vpg->width;
        vpgobj->imgdsc.data_size = vpg->frame_size;
    }
    else {
        vpgobj->imgdsc.data = vpg->decoded_frames[vpg->display_idx >= 0 ? vpg->display_idx : 0];
    vpgobj->imgdsc.header.magic = LV_IMAGE_HEADER_MAGIC;
    vpgobj->imgdsc.header.flags = LV_IMAGE_FLAGS_MODIFIABLE;
    vpgobj->imgdsc.header.cf = LV_COLOR_FORMAT_RGB565;
    vpgobj->imgdsc.header.h = vpg->height;
    vpgobj->imgdsc.header.w = vpg->width;
    vpgobj->imgdsc.header.stride = vpg->width * VPG_COLOR_BPP;
    vpgobj->imgdsc.data_size = vpg->decoded_size;
    }
#else
    vpgobj->imgdsc.data = vpg->frame;
    vpgobj->imgdsc.header.magic = LV_IMAGE_HEADER_MAGIC;
    vpgobj->imgdsc.header.flags = LV_IMAGE_FLAGS_MODIFIABLE;
    vpgobj->imgdsc.header.cf = LV_COLOR_FORMAT_RAW;
    vpgobj->imgdsc.header.h = vpg->height;
    vpgobj->imgdsc.header.w = vpg->width;
    vpgobj->imgdsc.data_size = vpg->frame_size;
#endif

    vpgobj->last_call = lv_tick_get();

    lv_image_set_src(obj, &vpgobj->imgdsc);

    lv_timer_resume(vpgobj->timer);
    lv_timer_reset(vpgobj->timer);

    next_frame_task_cb(vpgobj->timer);

}

lv_vpg_qoi_cache_t * lv_vpg_qoi_cache_create(const char * src)
{
    if(src == NULL) {
        return NULL;
    }

    lv_fs_file_t file;
    if(lv_fs_open(&file, src, LV_FS_MODE_RD) != LV_FS_RES_OK) {
        LV_LOG_WARN("Couldn't open qoi file: %s", src);
        return NULL;
    }

    uint32_t file_size = 0;
    if(lv_fs_seek(&file, 0, LV_FS_SEEK_END) != LV_FS_RES_OK ||
       lv_fs_tell(&file, &file_size) != LV_FS_RES_OK ||
       lv_fs_seek(&file, 0, LV_FS_SEEK_SET) != LV_FS_RES_OK ||
       file_size < 14) {
        lv_fs_close(&file);
        LV_LOG_WARN("Invalid qoi file size: %s", src);
        return NULL;
    }

    lv_vpg_qoi_cache_t * cache = lv_malloc(sizeof(lv_vpg_qoi_cache_t));
    if(cache == NULL) {
        lv_fs_close(&file);
        return NULL;
    }
    lv_memzero(cache, sizeof(lv_vpg_qoi_cache_t));

#ifndef SIMULATOR
    cache->data = heap_caps_malloc(file_size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
#else
    cache->data = lv_malloc(file_size);
#endif
    if(cache->data == NULL) {
        lv_fs_close(&file);
        lv_free(cache);
        return NULL;
    }

    uint32_t read_size = 0;
    if(lv_fs_read(&file, cache->data, file_size, &read_size) != LV_FS_RES_OK || read_size != file_size) {
        lv_fs_close(&file);
#ifndef SIMULATOR
        heap_caps_free(cache->data);
#else
        lv_free(cache->data);
#endif
        lv_free(cache);
        LV_LOG_WARN("Couldn't read qoi file: %s", src);
        return NULL;
    }
    lv_fs_close(&file);

    uint32_t width = 0;
    uint32_t height = 0;
    if(!lv_vpg_qoi_parse_header(cache->data, file_size, &width, &height)) {
#ifndef SIMULATOR
        heap_caps_free(cache->data);
#else
        lv_free(cache->data);
#endif
        lv_free(cache);
        LV_LOG_WARN("Invalid qoi header: %s", src);
        return NULL;
    }

    cache->data_size = file_size;
    cache->dsc.header.magic = LV_IMAGE_HEADER_MAGIC;
    cache->dsc.header.flags = 0;
    cache->dsc.header.cf = LV_COLOR_FORMAT_RAW;
    cache->dsc.header.w = (uint16_t)width;
    cache->dsc.header.h = (uint16_t)height;
    cache->dsc.header.stride = 0;
    cache->dsc.data_size = file_size;
    cache->dsc.data = cache->data;
    cache->dsc.reserved = NULL;

    return cache;
}

const lv_image_dsc_t * lv_vpg_qoi_cache_dsc(lv_vpg_qoi_cache_t * cache)
{
    if(cache == NULL || cache->data == NULL) {
        return NULL;
    }

    return &cache->dsc;
}

void lv_vpg_qoi_cache_destroy(lv_vpg_qoi_cache_t * cache)
{
    if(cache == NULL) {
        return;
    }

    if(cache->data) {
#ifndef SIMULATOR
        heap_caps_free(cache->data);
#else
        lv_free(cache->data);
#endif
        cache->data = NULL;
    }

    lv_free(cache);
}


static void lv_vpg_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj)
{
    LV_UNUSED(class_p);

    lv_vpg_t * vpgobj = (lv_vpg_t *) obj;

    vpgobj->vpg = NULL;
    vpgobj->timer = lv_timer_create(next_frame_task_cb, 10, obj);
    lv_timer_pause(vpgobj->timer);
}

static void lv_vpg_destructor(const lv_obj_class_t * class_p, lv_obj_t * obj)
{
    LV_UNUSED(class_p);
    lv_vpg_t * vpgobj = (lv_vpg_t *) obj;

    lv_image_cache_drop(lv_image_get_src(obj));

    if(vpgobj->vpg)
        vpg_close(vpgobj->vpg);
    lv_timer_delete(vpgobj->timer);
}

static bool vpg_obj_can_consume(lv_obj_t *obj)
{
    if (obj == NULL || !lv_obj_is_valid(obj)) {
        return false;
    }

    lv_obj_t *screen = lv_obj_get_screen(obj);
    if (screen == NULL || screen != lv_screen_active()) {
        return false;
    }

    for (lv_obj_t *cur = obj; cur != NULL; cur = lv_obj_get_parent(cur)) {
        if (lv_obj_has_flag(cur, LV_OBJ_FLAG_HIDDEN)) {
            return false;
        }
    }

    return true;
}

static void next_frame_task_cb(lv_timer_t * t)
{
    lv_obj_t * obj = t->user_data;
    lv_vpg_t * vpgobj = (lv_vpg_t *) obj;
    if (vpgobj->vpg == NULL || vpgobj->vpg->vpg == NULL) {
        lv_timer_pause(t);
        return;
    }

    uint32_t elaps = lv_tick_elaps(vpgobj->last_call);
    if(elaps < vpgobj->vpg->delay_ms) return;

    if(!vpg_obj_can_consume(obj)) {
        return;
    }

    vpgobj->last_call = lv_tick_get();

#ifndef SIMULATOR
    vpg_t *vpg = vpgobj->vpg;
    if (vpg->use_lvgl_decode) {
        int has_next = vpg_get_frame(vpgobj->vpg);
        if(has_next == 0) {
            lv_result_t res = lv_obj_send_event(obj, LV_EVENT_READY, NULL);
            lv_timer_pause(t);
            if(res != LV_RESULT_OK) return;
        }
        if (vpgobj->vpg->index == vpgobj->vpg->vpg->header.itemNum - 1) {
            uint32_t is_process = 0;
            lv_result_t res = lv_obj_send_event(obj, LV_EVENT_READY, &is_process);
            if (res != LV_RESULT_OK || is_process) {
                return;
            }
        }

        vpg_load_frame(vpgobj->vpg, (uint8_t *)vpgobj->imgdsc.data);
        vpgobj->imgdsc.data_size = vpgobj->vpg->frame_size;
        lv_image_cache_drop(lv_image_get_src(obj));
        lv_obj_invalidate(obj);
        return;
    }

    bool ready_to_notify = false;
    if (xSemaphoreTake(vpg->frame_lock, 0) == pdTRUE) {
        if (vpg->frame_ready && vpg->pending_idx >= 0) {
            vpg->display_idx = vpg->pending_idx;
            vpg->pending_idx = -1;
            vpg->frame_ready = 0;
            vpgobj->imgdsc.data = vpg->decoded_frames[vpg->display_idx];
            vpgobj->imgdsc.data_size = vpg->decoded_size;

            if (vpg->index == vpg->vpg->header.itemNum - 1) {
                ready_to_notify = true;
            }

            /* 通知解码任务可以产出下一帧 */
            xSemaphoreGive(vpg->pending_consumed_sem);

            lv_image_cache_drop(lv_image_get_src(obj));
            lv_obj_invalidate(obj);
        }
        xSemaphoreGive(vpg->frame_lock);
    }

    if (ready_to_notify) {
        uint32_t is_process = 0;
        lv_obj_send_event(obj, LV_EVENT_READY, &is_process);
    }
#else
    int has_next = vpg_get_frame(vpgobj->vpg);
    if(has_next == 0) {
        /*It was the last repeat*/
        lv_result_t res = lv_obj_send_event(obj, LV_EVENT_READY, NULL);
        lv_timer_pause(t);
        if(res != LV_RESULT_OK) return;
    }
    if (vpgobj->vpg->index == vpgobj->vpg->vpg->header.itemNum-1)
    {
        uint32_t is_process = 0;
        lv_result_t res = lv_obj_send_event(obj, LV_EVENT_READY, &is_process);
        if (is_process)
        {
            return;
        }
        
    }

    vpg_load_frame(vpgobj->vpg, (uint8_t *)vpgobj->imgdsc.data);
    vpgobj->imgdsc.data_size = vpgobj->vpg->frame_size;
    lv_image_cache_drop(lv_image_get_src(obj));
    lv_obj_invalidate(obj);
#endif
}

static void vpg_release_source_data(vpg_t *vpg)
{
    if(!vpg) return;

#ifndef SIMULATOR
    vpg_free_decoded_frames(vpg);
#endif

    if(vpg->frame) {
        lv_free(vpg->frame);
        vpg->frame = NULL;
    }
    if(vpg->vpg) {
        lv_free(vpg->vpg);
        vpg->vpg = NULL;
    }
    if(vpg->io && vpg->io->close && vpg->io_ctx) {
        vpg->io->close(vpg->io_ctx);
    }
    vpg->io = NULL;
    vpg->io_ctx = NULL;
}

static void vpg_reset_runtime_state(vpg_t *vpg)
{
    if(!vpg) return;

    vpg->frame_size = 0;
    vpg->delay_ms = 0;
    vpg->index = 0;
    vpg->width = 0;
    vpg->height = 0;

#ifndef SIMULATOR
    vpg->decoded_size = 0;
    vpg->use_lvgl_decode = 0;
    vpg->display_idx = -1;
    vpg->pending_idx = -1;
    vpg->frame_ready = 0;
    vpg->decode_index = 0;
    memset(&vpg->jpeg_io, 0, sizeof(vpg->jpeg_io));
    memset(&vpg->jpeg_header, 0, sizeof(vpg->jpeg_header));
#endif
}

#ifndef SIMULATOR
static void vpg_drain_pending_sem(vpg_t *vpg)
{
    if(!vpg || !vpg->pending_consumed_sem) return;

    while (xSemaphoreTake(vpg->pending_consumed_sem, 0) == pdTRUE) {
    }
}
#endif

static bool vpg_reset_source(vpg_t *vpg, const void *src)
{
    vpg_io_t const *io = NULL;
    void *io_ctx = NULL;

    if(!vpg) return false;

#ifndef SIMULATOR
    xSemaphoreTake(vpg->source_lock, portMAX_DELAY);
    vpg_drain_pending_sem(vpg);
    xSemaphoreTake(vpg->frame_lock, portMAX_DELAY);
    vpg->display_idx = -1;
    vpg->pending_idx = -1;
    vpg->frame_ready = 0;
    xSemaphoreGive(vpg->frame_lock);
#endif

    vpg_release_source_data(vpg);
    vpg_reset_runtime_state(vpg);

    if (src == NULL) {
#ifndef SIMULATOR
        xSemaphoreGive(vpg->source_lock);
#endif
        return true;
    }

    if(lv_image_src_get_type(src) == LV_IMAGE_SRC_FILE) {
        vpg_file_ctx_t *ctx = lv_malloc(sizeof(vpg_file_ctx_t));
        if(!ctx) goto fail;
        if(lv_fs_open(&ctx->f, src, LV_FS_MODE_RD) != LV_FS_RES_OK) {
            lv_free(ctx);
            goto fail;
        }
        io = &VPG_FILE_IO;
        io_ctx = ctx;
    }
    else if(lv_image_src_get_type(src) == LV_IMAGE_SRC_VARIABLE) {
        const lv_image_dsc_t *mem = src;
        vpg_mem_ctx_t *ctx = lv_malloc(sizeof(vpg_mem_ctx_t));
        if(!ctx) goto fail;
        ctx->base = (const uint8_t *)mem->data;
        ctx->size = mem->data_size;
        ctx->pos = 0;
        io = &VPG_MEM_IO;
        io_ctx = ctx;
    }
    else {
        goto fail;
    }

    vpg->io = io;
    vpg->io_ctx = io_ctx;

    vpg_file_header_t header = {0};
    uint32_t total_size = 0;
    vpg->io->seek(vpg->io_ctx, 0, LV_FS_SEEK_END);
    vpg->io->tell(vpg->io_ctx, &total_size);
    vpg->io->seek(vpg->io_ctx, 0, LV_FS_SEEK_SET);
    if(vpg->io->read(vpg->io_ctx, &header, sizeof(header), NULL) != LV_FS_RES_OK || header.magic != 0xAABBCCDD) {
        goto fail;
    }
    if(header.itemNum == 0 || header.fps == 0 || header.width == 0 || header.height == 0) {
        goto fail;
    }

    vpg->vpg = lv_malloc(sizeof(vpg_file_header_t) + header.itemNum * sizeof(vpg_item_header_t));
    if(!vpg->vpg) goto fail;

    vpg->io->seek(vpg->io_ctx, 0, LV_FS_SEEK_SET);
    if(vpg->io->read(vpg->io_ctx, vpg->vpg, sizeof(vpg_file_header_t) + header.itemNum * sizeof(vpg_item_header_t), NULL) != LV_FS_RES_OK) {
        goto fail;
    }

    uint32_t max_size = 0;
    for (uint32_t i = 0; i < vpg->vpg->header.itemNum; i++) {
        if ((vpg->vpg->item[i].offset + vpg->vpg->item[i].size) > total_size) {
            goto fail;
        }
        if (vpg->vpg->item[i].size > max_size) {
            max_size = vpg->vpg->item[i].size;
        }
    }

    vpg->index = vpg->vpg->header.itemNum - 1;
    vpg->frame_size = vpg->vpg->item[vpg->index].size;
    vpg->height = vpg->vpg->header.height;
    vpg->width = vpg->vpg->header.width;
    vpg->delay_ms = 1000 / vpg->vpg->header.fps;

#ifndef SIMULATOR
    if (vpg->delay_ms == 0) {
        vpg->delay_ms = 16;
    }

    vpg->use_lvgl_decode = (vpg->height < VPG_LVGL_DECODE_HEIGHT_THRESHOLD) ? 1 : 0;

    if (!vpg->use_lvgl_decode) {
        vpg->decoded_size = (uint32_t)vpg->width * (uint32_t)vpg->height * VPG_COLOR_BPP;
        if (vpg->decoded_size == 0) {
            goto fail;
        }

        vpg->decoded_frames[0] = heap_caps_aligned_alloc(16, vpg->decoded_size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
        vpg->decoded_frames[1] = heap_caps_aligned_alloc(16, vpg->decoded_size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
        vpg->decoded_frames[2] = heap_caps_aligned_alloc(16, vpg->decoded_size, MALLOC_CAP_SPIRAM | MALLOC_CAP_8BIT);
        if (!vpg->decoded_frames[0] || !vpg->decoded_frames[1] || !vpg->decoded_frames[2]) {
            ESP_LOGW(TAG, "PSRAM不足，回退到LVGL解码模式: %ux%u",
                     (unsigned)vpg->width, (unsigned)vpg->height);
            vpg_free_decoded_frames(vpg);
            vpg->decoded_size = 0;
            vpg->use_lvgl_decode = 1;
        }
    }
#endif

    vpg->frame = lv_malloc(max_size + 1);
    if (vpg->frame == NULL) {
        goto fail;
    }

#ifndef SIMULATOR
    if (vpg->use_lvgl_decode) {
        vpg->io->seek(vpg->io_ctx, vpg->vpg->item[0].offset, LV_FS_SEEK_SET);
        vpg->io->read(vpg->io_ctx, vpg->frame, vpg->vpg->item[0].size, NULL);
        vpg->frame_size = vpg->vpg->item[0].size;
        vpg->index = 0;
    }
    else {
        if (!vpg_decode_frame_to_rgb565(vpg, 0, vpg->decoded_frames[0], vpg->decoded_size)) {
            goto fail;
        }

        xSemaphoreTake(vpg->frame_lock, portMAX_DELAY);
        vpg->display_idx = 0;
        vpg->pending_idx = -1;
        vpg->frame_ready = 0;
        vpg->index = 0;
        vpg->decode_index = 1 % vpg->vpg->header.itemNum;
        xSemaphoreGive(vpg->frame_lock);
        xSemaphoreGive(vpg->pending_consumed_sem);
    }
    xSemaphoreGive(vpg->source_lock);
#else
    vpg->io->seek(vpg->io_ctx, vpg->vpg->item[0].offset, LV_FS_SEEK_SET);
    vpg->io->read(vpg->io_ctx, vpg->frame, vpg->vpg->item[0].size, NULL);
    vpg->frame_size = vpg->vpg->item[0].size;
#endif
    return true;

fail:
    vpg_release_source_data(vpg);
    vpg_reset_runtime_state(vpg);
#ifndef SIMULATOR
    xSemaphoreGive(vpg->source_lock);
#endif
    return false;
}

static vpg_t *vpg_open(const void *src){
    if (src == NULL)
    {
        return NULL;
    }
    vpg_t *vpg = lv_malloc(sizeof(vpg_t));
    if (vpg == NULL)
    {
        return NULL;
    }
    memset(vpg, 0, sizeof(vpg_t));

#ifndef SIMULATOR
    vpg->display_idx = -1;
    vpg->pending_idx = -1;
    vpg->stop_decode = 0;
    vpg->frame_lock = xSemaphoreCreateMutex();
    vpg->source_lock = xSemaphoreCreateMutex();
    if (vpg->frame_lock == NULL || vpg->source_lock == NULL) {
        if (vpg->frame_lock) vSemaphoreDelete(vpg->frame_lock);
        if (vpg->source_lock) vSemaphoreDelete(vpg->source_lock);
        lv_free(vpg);
        return NULL;
    }

    vpg->pending_consumed_sem = xSemaphoreCreateCounting(1, 0);
    if (vpg->pending_consumed_sem == NULL) {
        vSemaphoreDelete(vpg->source_lock);
        vSemaphoreDelete(vpg->frame_lock);
        lv_free(vpg);
        return NULL;
    }

    vpg->decode_exit_sem = xSemaphoreCreateBinary();
    if (vpg->decode_exit_sem == NULL) {
        vSemaphoreDelete(vpg->pending_consumed_sem);
        vSemaphoreDelete(vpg->source_lock);
        vSemaphoreDelete(vpg->frame_lock);
        lv_free(vpg);
        return NULL;
    }

    jpeg_dec_config_t cfg = {
        .output_type = JPEG_PIXEL_FORMAT_RGB565_LE,
        .rotate = JPEG_ROTATE_0D,
    };
    if (jpeg_dec_open(&cfg, &vpg->jpeg_dec) != JPEG_ERR_OK) {
        vSemaphoreDelete(vpg->decode_exit_sem);
        vSemaphoreDelete(vpg->pending_consumed_sem);
        vSemaphoreDelete(vpg->source_lock);
        vSemaphoreDelete(vpg->frame_lock);
        lv_free(vpg);
        return NULL;
    }

    memset(&vpg->jpeg_io, 0, sizeof(vpg->jpeg_io));
    memset(&vpg->jpeg_header, 0, sizeof(vpg->jpeg_header));

    if (xTaskCreatePinnedToCore(vpg_decode_task, "vpg_dec", 8192, vpg, 4, &vpg->decode_task, 0) != pdPASS) {
        jpeg_dec_close(vpg->jpeg_dec);
        vSemaphoreDelete(vpg->decode_exit_sem);
        vSemaphoreDelete(vpg->pending_consumed_sem);
        vSemaphoreDelete(vpg->source_lock);
        vSemaphoreDelete(vpg->frame_lock);
        lv_free(vpg);
        return NULL;
    }
#endif
    if(!vpg_reset_source(vpg, src)) {
        vpg_close(vpg);
        return NULL;
    }
    return vpg;

}

static int vpg_get_frame(vpg_t *vpg){
    vpg->index = (vpg->index+1)%vpg->vpg->header.itemNum;
    return 1;
}

static void vpg_load_frame(vpg_t *vpg, uint8_t *frame){
    uint32_t read;
    vpg_file_t *file = vpg->vpg;
    vpg_item_header_t *it = &file->item[vpg->index];
    vpg->frame_size = it->size;
    vpg->io->seek(vpg->io_ctx, it->offset, LV_FS_SEEK_SET);
    vpg->io->read(vpg->io_ctx, frame, it->size, &read);
}

static void vpg_close(vpg_t *vpg){
    if(!vpg) return;

#ifndef SIMULATOR
    vpg_stop_decode_task(vpg);
    if (vpg->jpeg_dec) {
        jpeg_dec_close(vpg->jpeg_dec);
        vpg->jpeg_dec = NULL;
    }
    if (vpg->decode_exit_sem) {
        vSemaphoreDelete(vpg->decode_exit_sem);
        vpg->decode_exit_sem = NULL;
    }
    if (vpg->pending_consumed_sem) {
        vSemaphoreDelete(vpg->pending_consumed_sem);
        vpg->pending_consumed_sem = NULL;
    }
    if (vpg->source_lock) {
        vSemaphoreDelete(vpg->source_lock);
        vpg->source_lock = NULL;
    }
    if (vpg->frame_lock) {
        vSemaphoreDelete(vpg->frame_lock);
        vpg->frame_lock = NULL;
    }
#endif
    vpg_release_source_data(vpg);
    lv_free(vpg);
}

#ifndef SIMULATOR
static bool vpg_stop_decode_task(vpg_t *vpg)
{
    if (!vpg || !vpg->decode_task) {
        return true;
    }

    vpg->stop_decode = 1;

    if (vpg->pending_consumed_sem) {
        xSemaphoreGive(vpg->pending_consumed_sem);
    }

    if (vpg->decode_exit_sem && xSemaphoreTake(vpg->decode_exit_sem, pdMS_TO_TICKS(200)) == pdTRUE) {
        vpg->decode_task = NULL;
        return true;
    }

    if (vpg->decode_task == NULL) {
        return true;
    }

    ESP_LOGW(TAG, "decode task exit timeout, force delete");
    vTaskDelete(vpg->decode_task);
    vpg->decode_task = NULL;
    return false;
}

static bool vpg_decode_frame_to_rgb565(vpg_t *vpg, uint16_t frame_idx, uint8_t *out, uint32_t out_size)
{
    if (!vpg || !vpg->vpg || !out || out_size < vpg->decoded_size) {
        return false;
    }

    vpg_item_header_t *it = &vpg->vpg->item[frame_idx];
    uint32_t read_size = 0;
    vpg->io->seek(vpg->io_ctx, it->offset, LV_FS_SEEK_SET);
    if (vpg->io->read(vpg->io_ctx, vpg->frame, it->size, &read_size) != LV_FS_RES_OK || read_size != it->size) {
        return false;
    }

    vpg->jpeg_io.inbuf = vpg->frame;
    vpg->jpeg_io.inbuf_len = it->size;
    if (jpeg_dec_parse_header(vpg->jpeg_dec, &vpg->jpeg_io, &vpg->jpeg_header) < 0) {
        return false;
    }

    vpg->jpeg_io.outbuf = out;
    int consumed = vpg->jpeg_io.inbuf_len - vpg->jpeg_io.inbuf_remain;
    vpg->jpeg_io.inbuf = vpg->frame + consumed;
    vpg->jpeg_io.inbuf_len = vpg->jpeg_io.inbuf_remain;

    return jpeg_dec_process(vpg->jpeg_dec, &vpg->jpeg_io) == ESP_OK;
}

static void vpg_decode_task(void *arg)
{
    vpg_t *vpg = (vpg_t *)arg;
    // uint32_t fps_count = 0;
    // int64_t fps_window_us = esp_timer_get_time();

    while (1) {
        /* 阻塞等待 LVGL 消费上一个 pending 帧 */
        if (xSemaphoreTake(vpg->pending_consumed_sem, portMAX_DELAY) != pdTRUE) break;
        if (vpg->stop_decode) break;

        xSemaphoreTake(vpg->source_lock, portMAX_DELAY);
        if (vpg->stop_decode || vpg->vpg == NULL || vpg->decoded_size == 0 || vpg->use_lvgl_decode) {
            xSemaphoreGive(vpg->source_lock);
            if (vpg->stop_decode) {
                break;
            }
            continue;
        }

        /* 找空闲槽：既不是 display_idx 也不是 pending_idx 的那个 */
        int decode_slot = -1;
        xSemaphoreTake(vpg->frame_lock, portMAX_DELAY);
        int d = vpg->display_idx;
        int p = vpg->pending_idx;
        for (int i = 0; i < 3; i++) {
            if (i != d && i != p) { decode_slot = i; break; }
        }
        xSemaphoreGive(vpg->frame_lock);

        if (decode_slot < 0) {
            /* 理论上不会发生，保险归还信号量 */
            xSemaphoreGive(vpg->source_lock);
            xSemaphoreGive(vpg->pending_consumed_sem);
            vTaskDelay(pdMS_TO_TICKS(1));
            continue;
        }

        uint16_t frame_idx = vpg->decode_index;
        if (vpg_decode_frame_to_rgb565(vpg, frame_idx, vpg->decoded_frames[decode_slot], vpg->decoded_size)) {
            xSemaphoreTake(vpg->frame_lock, portMAX_DELAY);
            vpg->pending_idx = decode_slot;
            vpg->index = frame_idx;
            vpg->frame_ready = 1;
            xSemaphoreGive(vpg->frame_lock);
            /* 信号量不在此归还，等 LVGL 消费后归还 */

            // fps_count++;
            // int64_t now_us = esp_timer_get_time();
            // if (now_us - fps_window_us >= 1000000LL) {
            //     ESP_LOGI(TAG, "decode fps: %lu (target: %u)",
            //              (unsigned long)fps_count,
            //              (unsigned)vpg->vpg->header.fps);
            //     fps_count = 0;
            //     fps_window_us = now_us;
            // }
        } else {
            ESP_LOGW(TAG, "decode frame %u failed", (unsigned)frame_idx);
            /* 解码失败，归还信号量以允许继续尝试 */
            xSemaphoreGive(vpg->pending_consumed_sem);
        }

        vpg->decode_index = (vpg->decode_index + 1) % vpg->vpg->header.itemNum;
        xSemaphoreGive(vpg->source_lock);
    }

    if (vpg->decode_exit_sem) {
        xSemaphoreGive(vpg->decode_exit_sem);
    }
    vpg->decode_task = NULL;
    ESP_LOGW(TAG, "decode task exit");
    vTaskDelete(NULL);
}
#endif


/*==============================
 * IO 实现
 *=============================*/
static lv_fs_res_t vpg_file_read(void *ctx, void *buf, uint32_t size, uint32_t *out_read) {
    vpg_file_ctx_t *c = ctx;
    return lv_fs_read(&c->f, buf, size, out_read);
}
static lv_fs_res_t vpg_file_seek(void *ctx, uint32_t pos, uint8_t whence) {
    vpg_file_ctx_t *c = ctx;
    return lv_fs_seek(&c->f, pos, whence);
}
static lv_fs_res_t vpg_file_tell(void *ctx, uint32_t *pos) {
    vpg_file_ctx_t *c = ctx;
    return lv_fs_tell(&c->f, pos);
}
static void vpg_file_close(void *ctx) {
    vpg_file_ctx_t *c = ctx;
    lv_fs_close(&c->f);
    lv_free(c);
}

/* 内存实现 */
static lv_fs_res_t vpg_mem_read(void *ctx, void *buf, uint32_t size, uint32_t *out_read) {
    vpg_mem_ctx_t *c = ctx;
    if(c->pos >= c->size) { if(out_read) *out_read = 0; return LV_FS_RES_OK; }
    uint32_t remain = c->size - c->pos;
    uint32_t n = size < remain ? size : remain;
    if(n) lv_memcpy(buf, c->base + c->pos, n);
    c->pos += n;
    if(out_read) *out_read = n;
    return LV_FS_RES_OK;
}
static lv_fs_res_t vpg_mem_seek(void *ctx, uint32_t pos, uint8_t whence) {
    vpg_mem_ctx_t *c = ctx;
    uint32_t newpos = 0;
    if(whence == LV_FS_SEEK_SET) newpos = pos;
    else if(whence == LV_FS_SEEK_CUR) newpos = c->pos + pos;
    else if(whence == LV_FS_SEEK_END) newpos = (pos <= c->size) ? (c->size - pos) : c->size;
    else return LV_FS_RES_FS_ERR;
    if(newpos > c->size) newpos = c->size;
    c->pos = newpos;
    return LV_FS_RES_OK;
}
static lv_fs_res_t vpg_mem_tell(void *ctx, uint32_t *pos) {
    vpg_mem_ctx_t *c = ctx;
    if(pos) *pos = c->pos;
    return LV_FS_RES_OK;
}
static void vpg_mem_close(void *ctx) {
    lv_free(ctx);
}

static bool lv_vpg_qoi_parse_header(const uint8_t *data, uint32_t data_size, uint32_t *w, uint32_t *h)
{
    if(data == NULL || data_size < 14 || w == NULL || h == NULL) {
        return false;
    }

    if(data[0] != 'q' || data[1] != 'o' || data[2] != 'i' || data[3] != 'f') {
        return false;
    }

    *w = ((uint32_t)data[4] << 24) | ((uint32_t)data[5] << 16) | ((uint32_t)data[6] << 8) | (uint32_t)data[7];
    *h = ((uint32_t)data[8] << 24) | ((uint32_t)data[9] << 16) | ((uint32_t)data[10] << 8) | (uint32_t)data[11];

    return (*w > 0 && *h > 0);
}
