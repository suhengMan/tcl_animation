/**
 * @file lv_sdl_window.h
 *
 */

/*********************
 *      INCLUDES
 *********************/
#include "lv_sdl_window.h"
#if LV_USE_SDL
#include <stdbool.h>
#include "../../core/lv_refr.h"
#include "../../stdlib/lv_string.h"
#include "../../core/lv_global.h"
#include "../../display/lv_display_private.h"
#include "../../lv_init.h"
#include "../../draw/lv_draw_buf.h"

/* for aligned_alloc */
#ifndef __USE_ISOC11
    #define __USE_ISOC11
#endif
#include <stdlib.h>

#define SDL_MAIN_HANDLED /*To fix SDL's "undefined reference to WinMain" issue*/
#include "lv_sdl_private.h"

/*********************
 *      DEFINES
 *********************/
#define lv_deinit_in_progress  LV_GLOBAL_DEFAULT()->deinit_in_progress

/**********************
 *      TYPEDEFS
 **********************/
typedef struct {
    SDL_Window * window;
    SDL_Renderer * renderer;
#if LV_USE_DRAW_SDL == 0
    SDL_Texture * texture;
    uint8_t * fb1;
    uint8_t * fb2;
    uint8_t * fb_act;
    uint8_t * buf1;
    uint8_t * buf2;
    uint8_t * rotated_buf;
    size_t rotated_buf_size;
#endif
    uint8_t zoom;
    uint8_t ignore_size_chg;
} lv_sdl_window_t;

/**********************
 *  STATIC PROTOTYPES
 **********************/
static inline int sdl_render_mode(void);
static void flush_cb(lv_display_t * disp, const lv_area_t * area, uint8_t * color_p);
static void window_create(lv_display_t * disp);
static void window_update(lv_display_t * disp);
#if LV_USE_DRAW_SDL == 0
    static void texture_resize(lv_display_t * disp);
    static void * sdl_draw_buf_realloc_aligned(void * ptr, size_t new_size);
    static void sdl_draw_buf_free(void * ptr);
#endif
static void sdl_event_handler(lv_timer_t * t);
static void release_disp_cb(lv_event_t * e);
static void res_chg_event_cb(lv_event_t * e);

/**********************
 *  STATIC VARIABLES
 **********************/
static bool inited = false;
static lv_timer_t * event_handler_timer;

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

lv_display_t * lv_sdl_window_create(int32_t hor_res, int32_t ver_res)
{
    if(!inited) {
        SDL_Init(SDL_INIT_VIDEO);
        SDL_StartTextInput();
        event_handler_timer = lv_timer_create(sdl_event_handler, 5, NULL);
        lv_tick_set_cb(SDL_GetTicks);

        inited = true;
    }

    lv_sdl_window_t * dsc = lv_malloc_zeroed(sizeof(lv_sdl_window_t));
    LV_ASSERT_MALLOC(dsc);
    if(dsc == NULL) return NULL;

    lv_display_t * disp = lv_display_create(hor_res, ver_res);
    if(disp == NULL) {
        lv_free(dsc);
        return NULL;
    }
    lv_display_add_event_cb(disp, release_disp_cb, LV_EVENT_DELETE, disp);
    lv_display_set_driver_data(disp, dsc);
    window_create(disp);

    lv_display_set_flush_cb(disp, flush_cb);

#if LV_USE_DRAW_SDL == 0
    if(sdl_render_mode() == LV_DISPLAY_RENDER_MODE_PARTIAL) {
        dsc->buf1 = sdl_draw_buf_realloc_aligned(NULL, 32 * 1024);
#if LV_SDL_BUF_COUNT == 2
        dsc->buf2 = sdl_draw_buf_realloc_aligned(NULL, 32 * 1024);
#endif
        lv_display_set_buffers(disp, dsc->buf1, dsc->buf2,
                               32 * 1024, LV_DISPLAY_RENDER_MODE_PARTIAL);
    }
    /*LV_DISPLAY_RENDER_MODE_DIRECT or FULL */
    else {
        uint32_t stride = lv_draw_buf_width_to_stride(disp->hor_res,
                                                      lv_display_get_color_format(disp));
        lv_display_set_buffers(disp, dsc->fb1, dsc->fb2, stride * disp->ver_res,
                               LV_SDL_RENDER_MODE);
    }
#else /*LV_USE_DRAW_SDL == 1*/
    /*It will render directly to default Texture, so the buffer is not used, so just set something*/
    static lv_draw_buf_t draw_buf;
    static uint8_t dummy_buf; /*It won't be used as it will render to the SDL textures directly*/
    lv_draw_buf_init(&draw_buf, 4096, 4096, LV_COLOR_FORMAT_ARGB8888, 4096 * 4, &dummy_buf, 4096 * 4096 * 4);

    lv_display_set_draw_buffers(disp, &draw_buf, NULL);
    lv_display_set_render_mode(disp, LV_DISPLAY_RENDER_MODE_DIRECT);
#endif /*LV_USE_DRAW_SDL == 0*/
    lv_display_add_event_cb(disp, res_chg_event_cb, LV_EVENT_RESOLUTION_CHANGED, NULL);

    /*Process the initial events*/
    sdl_event_handler(NULL);

    return disp;
}

void lv_sdl_window_set_resizeable(lv_display_t * disp, bool value)
{
    lv_sdl_window_t * dsc = lv_display_get_driver_data(disp);
    SDL_SetWindowResizable(dsc->window, value);
}

void lv_sdl_window_set_zoom(lv_display_t * disp, uint8_t zoom)
{
    lv_sdl_window_t * dsc = lv_display_get_driver_data(disp);
    dsc->zoom = zoom;
    lv_display_send_event(disp, LV_EVENT_RESOLUTION_CHANGED, NULL);
    lv_refr_now(disp);
}

uint8_t lv_sdl_window_get_zoom(lv_display_t * disp)
{
    lv_sdl_window_t * dsc = lv_display_get_driver_data(disp);
    return dsc->zoom;
}

lv_display_t * lv_sdl_get_disp_from_win_id(uint32_t win_id)
{
    lv_display_t * disp = lv_display_get_next(NULL);
    if(win_id == UINT32_MAX) return disp;

    while(disp) {
        lv_sdl_window_t * dsc = lv_display_get_driver_data(disp);
        if(dsc != NULL && SDL_GetWindowID(dsc->window) == win_id) {
            return disp;
        }
        disp = lv_display_get_next(disp);
    }
    return NULL;
}

void lv_sdl_window_set_title(lv_display_t * disp, const char * title)
{
    lv_sdl_window_t * dsc = lv_display_get_driver_data(disp);
    SDL_SetWindowTitle(dsc->window, title);
}

void * lv_sdl_window_get_renderer(lv_display_t * disp)
{
    lv_sdl_window_t * dsc = lv_display_get_driver_data(disp);
    return dsc->renderer;
}

void lv_sdl_quit(void)
{
    if(inited) {
        SDL_Quit();
        lv_timer_delete(event_handler_timer);
        event_handler_timer = NULL;
        inited = false;
    }
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

static inline int sdl_render_mode(void)
{
    return LV_SDL_RENDER_MODE;
}

static void flush_cb(lv_display_t * disp, const lv_area_t * area, uint8_t * px_map)
{
#if LV_USE_DRAW_SDL == 0
    lv_area_t rotated_area;
    lv_sdl_window_t * dsc = lv_display_get_driver_data(disp);
    lv_color_format_t cf = lv_display_get_color_format(disp);

    /* 解析像素大小 */
    uint32_t px_size = lv_color_format_get_size(cf);

    /* 获取屏幕分辨率（若获取失败则默认 360x360） */
    int32_t sw = 0, sh = 0;
#ifdef LVGL_VERSION_MAJOR /* v9 建议使用 API 获取 */
    sw = lv_display_get_horizontal_resolution(disp);
    sh = lv_display_get_vertical_resolution(disp);
#endif
    if(sw <= 0) sw = (int32_t)disp->hor_res;   /* 兼容已有成员用法 */
    if(sh <= 0) sh = (int32_t)disp->ver_res;
    if(sw <= 0 || sh <= 0) { sw = 360; sh = 360; }  /* 最终兜底 */

    /* 圆参数（切线圆：与短边相切） */
    const int32_t cx = sw / 2;
    const int32_t cy = sh / 2;
    const int32_t r  = (sw < sh ? sw : sh) / 2;
    const int64_t r2 = (int64_t)r * (int64_t)r;

    /* 一个小工具：写黑色像素（按字节清零） */
    #define WRITE_BLACK(dst) do { memset((dst), 0, px_size); } while(0)

    if(sdl_render_mode() == LV_DISPLAY_RENDER_MODE_PARTIAL) {
        lv_display_rotation_t rotation = lv_display_get_rotation(disp);

        if(rotation != LV_DISPLAY_ROTATION_0) {
            int32_t w = lv_area_get_width(area);
            int32_t h = lv_area_get_height(area);
            uint32_t w_stride = lv_draw_buf_width_to_stride(w, cf);
            uint32_t h_stride = lv_draw_buf_width_to_stride(h, cf);
            size_t buf_size = (size_t)w * (size_t)h * px_size;

            /* (Re)allocate temporary buffer if needed */
            if(!dsc->rotated_buf || dsc->rotated_buf_size != buf_size) {
                dsc->rotated_buf = sdl_draw_buf_realloc_aligned(dsc->rotated_buf, buf_size);
                dsc->rotated_buf_size = buf_size;
            }

            switch(rotation) {
                case LV_DISPLAY_ROTATION_0:
                    break;
                case LV_DISPLAY_ROTATION_90:
                    lv_draw_sw_rotate(px_map, dsc->rotated_buf, w, h, w_stride, h_stride, rotation, cf);
                    break;
                case LV_DISPLAY_ROTATION_180:
                    lv_draw_sw_rotate(px_map, dsc->rotated_buf, w, h, w_stride, w_stride, rotation, cf);
                    break;
                case LV_DISPLAY_ROTATION_270:
                    lv_draw_sw_rotate(px_map, dsc->rotated_buf, w, h, w_stride, h_stride, rotation, cf);
                    break;
            }

            px_map = dsc->rotated_buf;

            rotated_area = *area;
            lv_display_rotate_area(disp, &rotated_area);
            area = &rotated_area;
        }

        /* 将 px_map 的内容拷贝到 framebuffer，同时做圆形遮罩 */
        uint32_t src_stride_bytes = lv_draw_buf_width_to_stride(lv_area_get_width(area), cf);
        uint8_t * fb_line = dsc->fb_act
                          + (size_t)area->y1 * (size_t)disp->hor_res * px_size
                          + (size_t)area->x1 * px_size;
        const uint8_t * src_line = px_map;

        for(int32_t y = area->y1; y <= area->y2; y++) {
            const int32_t dy = y - cy;
            const int64_t dy2 = (int64_t)dy * (int64_t)dy;

            uint8_t * fb_px = fb_line;
            const uint8_t * src_px = src_line;

            for(int32_t x = area->x1; x <= area->x2; x++) {
                const int32_t dx = x - cx;
                const int64_t d2 = (int64_t)dx * (int64_t)dx + dy2;

                if(d2 <= r2) {
                    /* 圆内：拷贝原像素 */
                    memcpy(fb_px, src_px, px_size);
                } else {
                    /* 圆外：写黑色 */
                    WRITE_BLACK(fb_px);
                }
                fb_px  += px_size;
                src_px += px_size;
            }

            fb_line  += (size_t)disp->hor_res * px_size;
            src_line += src_stride_bytes;
        }
    }

    /* 如果是全缓冲渲染（非 PARTIAL），对整块缓冲做一次圆形遮罩后再更新窗口 */
    if(lv_display_flush_is_last(disp)) {
        if(sdl_render_mode() != LV_DISPLAY_RENDER_MODE_PARTIAL) {
            /* 在非 PARTIAL 模式下，px_map 指向完整帧，直接遮罩并交给窗口 */
            uint8_t * fb_full = px_map;
            const int32_t w = sw;
            const int32_t h = sh;

            for(int32_t y = 0; y < h; y++) {
                const int32_t dy = y - cy;
                const int64_t dy2 = (int64_t)dy * (int64_t)dy;

                uint8_t * fb_px = fb_full + (size_t)y * (size_t)w * px_size;

                for(int32_t x = 0; x < w; x++) {
                    const int32_t dx = x - cx;
                    const int64_t d2 = (int64_t)dx * (int64_t)dx + dy2;

                    if(d2 > r2) {
                        WRITE_BLACK(fb_px);
                    }
                    fb_px += px_size;
                }
            }
            dsc->fb_act = px_map;
        }
        window_update(disp);
    }
#else
    LV_UNUSED(area);
    LV_UNUSED(px_map);
    if(lv_display_flush_is_last(disp)) {
        window_update(disp);
    }
#endif /* LV_USE_DRAW_SDL == 0 */

    lv_display_flush_ready(disp);
}
/**
 * SDL main thread. All SDL related task have to be handled here!
 * It initializes SDL, handles drawing and the mouse.
 */
static void sdl_event_handler(lv_timer_t * t)
{
    LV_UNUSED(t);

    /*Refresh handling*/
    SDL_Event event;
    while(SDL_PollEvent(&event)) {
        lv_sdl_mouse_handler(&event);
#if LV_SDL_MOUSEWHEEL_MODE == LV_SDL_MOUSEWHEEL_MODE_ENCODER
        lv_sdl_mousewheel_handler(&event);
#endif
        lv_sdl_keyboard_handler(&event);

        if(event.type == SDL_WINDOWEVENT) {
            lv_display_t * disp = lv_sdl_get_disp_from_win_id(event.window.windowID);
            if(disp == NULL) continue;
            lv_sdl_window_t * dsc = lv_display_get_driver_data(disp);
            switch(event.window.event) {
#if SDL_VERSION_ATLEAST(2, 0, 5)
                case SDL_WINDOWEVENT_TAKE_FOCUS:
#endif
                case SDL_WINDOWEVENT_EXPOSED:
                    window_update(disp);
                    break;
                case SDL_WINDOWEVENT_RESIZED:
                    dsc->ignore_size_chg = 1;
                    lv_display_set_resolution(disp, event.window.data1 / dsc->zoom, event.window.data2 / dsc->zoom);
                    dsc->ignore_size_chg = 0;
                    lv_refr_now(disp);
                    break;
                case SDL_WINDOWEVENT_CLOSE:
                    lv_display_delete(disp);
                    break;
                default:
                    break;
            }
        }
        if(event.type == SDL_QUIT) {
            SDL_Quit();
            lv_deinit();
            inited = false;
#if LV_SDL_DIRECT_EXIT
            exit(0);
#endif
        }
    }
}

static void window_create(lv_display_t * disp)
{
    lv_sdl_window_t * dsc = lv_display_get_driver_data(disp);
    dsc->zoom = 1;

    int flag = SDL_WINDOW_RESIZABLE;
#if LV_SDL_FULLSCREEN
    flag |= SDL_WINDOW_FULLSCREEN;
#endif

    int32_t hor_res = disp->hor_res;
    int32_t ver_res = disp->ver_res;
    dsc->window = SDL_CreateWindow("LVGL Simulator",
                                   SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
                                   hor_res * dsc->zoom, ver_res * dsc->zoom, flag);       /*last param. SDL_WINDOW_BORDERLESS to hide borders*/

    dsc->renderer = SDL_CreateRenderer(dsc->window, -1,
                                       LV_SDL_ACCELERATED ? SDL_RENDERER_ACCELERATED : SDL_RENDERER_SOFTWARE);
#if LV_USE_DRAW_SDL == 0
    texture_resize(disp);

    uint32_t px_size = lv_color_format_get_size(lv_display_get_color_format(disp));
    lv_memset(dsc->fb1, 0xff, hor_res * ver_res * px_size);
#if LV_SDL_BUF_COUNT == 2
    lv_memset(dsc->fb2, 0xff, hor_res * ver_res * px_size);
#endif
#endif /*LV_USE_DRAW_SDL == 0*/
    /*Some platforms (e.g. Emscripten) seem to require setting the size again */
    SDL_SetWindowSize(dsc->window, hor_res * dsc->zoom, ver_res * dsc->zoom);
#if LV_USE_DRAW_SDL == 0
    texture_resize(disp);
#endif /*LV_USE_DRAW_SDL == 0*/
}

static void window_update(lv_display_t * disp)
{
    lv_sdl_window_t * dsc = lv_display_get_driver_data(disp);
#if LV_USE_DRAW_SDL == 0
    int32_t hor_res = disp->hor_res;
    uint32_t stride = lv_draw_buf_width_to_stride(hor_res, lv_display_get_color_format(disp));
    SDL_UpdateTexture(dsc->texture, NULL, dsc->fb_act, stride);

    SDL_RenderClear(dsc->renderer);

    /*Update the renderer with the texture containing the rendered image*/
    SDL_RenderCopy(dsc->renderer, dsc->texture, NULL, NULL);
#endif
    SDL_RenderPresent(dsc->renderer);
}

#if LV_USE_DRAW_SDL == 0
static void texture_resize(lv_display_t * disp)
{
    uint32_t stride = lv_draw_buf_width_to_stride(disp->hor_res, lv_display_get_color_format(disp));
    lv_sdl_window_t * dsc = lv_display_get_driver_data(disp);

    dsc->fb1 = sdl_draw_buf_realloc_aligned(dsc->fb1, stride * disp->ver_res);
    lv_memzero(dsc->fb1, stride * disp->ver_res);

    if(sdl_render_mode() == LV_DISPLAY_RENDER_MODE_PARTIAL) {
        dsc->fb_act = dsc->fb1;
    }
    else {
#if LV_SDL_BUF_COUNT == 2
        dsc->fb2 = sdl_draw_buf_realloc_aligned(dsc->fb2, stride * disp->ver_res);
        memset(dsc->fb2, 0x00, stride * disp->ver_res);
#endif
        lv_display_set_buffers(disp, dsc->fb1, dsc->fb2, stride * disp->ver_res, LV_SDL_RENDER_MODE);
    }
    if(dsc->texture) SDL_DestroyTexture(dsc->texture);

#if LV_COLOR_DEPTH == 32
    SDL_PixelFormatEnum px_format =
        SDL_PIXELFORMAT_RGB888; /*same as SDL_PIXELFORMAT_RGB888, but it's not supported in older versions*/
#elif LV_COLOR_DEPTH == 24
    SDL_PixelFormatEnum px_format = SDL_PIXELFORMAT_BGR24;
#elif LV_COLOR_DEPTH == 16
    SDL_PixelFormatEnum px_format = SDL_PIXELFORMAT_RGB565;
#else
#error("Unsupported color format")
#endif
    //    px_format = SDL_PIXELFORMAT_BGR24;

    dsc->texture = SDL_CreateTexture(dsc->renderer, px_format,
                                     SDL_TEXTUREACCESS_STATIC, disp->hor_res, disp->ver_res);
    SDL_SetTextureBlendMode(dsc->texture, SDL_BLENDMODE_BLEND);
}

static void * sdl_draw_buf_realloc_aligned(void * ptr, size_t new_size)
{
    if(ptr) {
        sdl_draw_buf_free(ptr);
    }

    /* No need copy for drawing buffer */

#ifndef _WIN32
    /* Size must be multiple of align, See: https://en.cppreference.com/w/c/memory/aligned_alloc */

#define BUF_ALIGN (LV_DRAW_BUF_ALIGN < sizeof(void *) ? sizeof(void *) : LV_DRAW_BUF_ALIGN)
    return aligned_alloc(BUF_ALIGN, LV_ALIGN_UP(new_size, BUF_ALIGN));
#else
    return _aligned_malloc(LV_ALIGN_UP(new_size, LV_DRAW_BUF_ALIGN), LV_DRAW_BUF_ALIGN);
#endif /* _WIN32 */
}

static void sdl_draw_buf_free(void * ptr)
{
#ifndef _WIN32
    free(ptr);
#else
    _aligned_free(ptr);
#endif /* _WIN32 */
}
#endif

static void res_chg_event_cb(lv_event_t * e)
{
    lv_display_t * disp = lv_event_get_current_target(e);

    lv_sdl_window_t * dsc = lv_display_get_driver_data(disp);
    if(dsc->ignore_size_chg == false) {
        SDL_SetWindowSize(dsc->window, disp->hor_res * dsc->zoom, disp->ver_res * dsc->zoom);
    }

#if LV_USE_DRAW_SDL == 0
    texture_resize(disp);
#endif
}

static void release_disp_cb(lv_event_t * e)
{
    if(lv_deinit_in_progress) {
        lv_sdl_quit();
    }

    lv_display_t * disp = (lv_display_t *) lv_event_get_user_data(e);

    lv_sdl_window_t * dsc = lv_display_get_driver_data(disp);
#if LV_USE_DRAW_SDL == 0
    SDL_DestroyTexture(dsc->texture);
#endif
    SDL_DestroyRenderer(dsc->renderer);
    SDL_DestroyWindow(dsc->window);
#if LV_USE_DRAW_SDL == 0
    if(dsc->fb1) sdl_draw_buf_free(dsc->fb1);
    if(dsc->fb2) sdl_draw_buf_free(dsc->fb2);
    if(dsc->buf1) sdl_draw_buf_free(dsc->buf1);
    if(dsc->buf2) sdl_draw_buf_free(dsc->buf2);
#endif
    lv_free(dsc);
    lv_display_set_driver_data(disp, NULL);
}

#endif /*LV_USE_SDL*/
