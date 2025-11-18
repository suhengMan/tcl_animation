/*******************************************************************************
 * Size: 12 px
 * Bpp: 2
 * Opts: --bpp 2 --size 12 --no-compress --stride 1 --align 1 --font Orbitron-VariableFont_wght.ttf --symbols 1234567890:MonTueWdhFriSatn / --format lvgl -o font_orbitron_12_2.c
 ******************************************************************************/

#ifdef __has_include
    #if __has_include("lvgl.h")
        #ifndef LV_LVGL_H_INCLUDE_SIMPLE
            #define LV_LVGL_H_INCLUDE_SIMPLE
        #endif
    #endif
#endif

#ifdef LV_LVGL_H_INCLUDE_SIMPLE
    #include "lvgl.h"
#else
    #include "lvgl/lvgl.h"
#endif



#ifndef FONT_ORBITRON_12_2
#define FONT_ORBITRON_12_2 1
#endif

#if FONT_ORBITRON_12_2

/*-----------------
 *    BITMAPS
 *----------------*/

/*Store the image of the glyphs*/
static LV_ATTRIBUTE_LARGE_CONST const uint8_t glyph_bitmap[] = {
    /* U+0020 " " */

    /* U+002F "/" */
    0x0, 0x0, 0x0, 0xc0, 0x9, 0x0, 0x60, 0x6,
    0x0, 0x30, 0x3, 0x0, 0x24, 0x0, 0x40, 0x0,

    /* U+0030 "0" */
    0x3f, 0xff, 0xc6, 0x0, 0x1d, 0x60, 0x7, 0xd6,
    0x2, 0xc9, 0x60, 0xb0, 0x96, 0x38, 0x9, 0x7e,
    0x0, 0x97, 0x40, 0x9, 0x3f, 0xff, 0xc0,

    /* U+0031 "1" */
    0xe, 0x3a, 0xa6, 0x6, 0x6, 0x6, 0x6, 0x6,
    0x6,

    /* U+0032 "2" */
    0x3f, 0xff, 0xc2, 0x0, 0x9, 0x0, 0x0, 0x90,
    0x0, 0x9, 0x2f, 0xff, 0xc6, 0x0, 0x0, 0x60,
    0x0, 0x6, 0x0, 0x0, 0x7f, 0xff, 0xd0,

    /* U+0033 "3" */
    0x3f, 0xff, 0x46, 0x0, 0xc, 0x0, 0x0, 0xc0,
    0x0, 0xc, 0xb, 0xff, 0xc0, 0x0, 0x9, 0x0,
    0x0, 0x91, 0x0, 0x9, 0x3f, 0xff, 0xc0,

    /* U+0034 "4" */
    0x0, 0x2c, 0x0, 0x2f, 0x0, 0x2c, 0xc0, 0x2c,
    0x30, 0x2c, 0xc, 0x2c, 0x3, 0xf, 0xff, 0xf4,
    0x0, 0x30, 0x0, 0xc, 0x0,

    /* U+0035 "5" */
    0x7f, 0xff, 0xd6, 0x0, 0x0, 0x60, 0x0, 0x6,
    0x0, 0x0, 0x7f, 0xff, 0xc0, 0x0, 0x9, 0x0,
    0x0, 0x92, 0x0, 0x9, 0x3f, 0xff, 0xc0,

    /* U+0036 "6" */
    0x3f, 0xff, 0x6, 0x0, 0x0, 0x60, 0x0, 0x6,
    0x0, 0x0, 0x7f, 0xff, 0xc6, 0x0, 0x9, 0x60,
    0x0, 0x96, 0x0, 0x9, 0x3f, 0xff, 0xc0,

    /* U+0037 "7" */
    0xff, 0xf8, 0x0, 0xc, 0x0, 0x9, 0x0, 0x9,
    0x0, 0x9, 0x0, 0x9, 0x0, 0x9, 0x0, 0x9,
    0x0, 0x9,

    /* U+0038 "8" */
    0x3f, 0xff, 0xc6, 0x0, 0x9, 0x60, 0x0, 0x96,
    0x0, 0x9, 0x3f, 0xff, 0xc6, 0x0, 0x9, 0x60,
    0x0, 0x96, 0x0, 0x9, 0x3f, 0xff, 0xc0,

    /* U+0039 "9" */
    0x3f, 0xff, 0xc6, 0x0, 0x9, 0x60, 0x0, 0x96,
    0x0, 0x9, 0x3f, 0xff, 0xd0, 0x0, 0x9, 0x0,
    0x0, 0x90, 0x0, 0x9, 0x3f, 0xff, 0xc0,

    /* U+003A ":" */
    0x60, 0x0, 0x0, 0x60,

    /* U+0046 "F" */
    0x7f, 0xff, 0x98, 0x0, 0x6, 0x0, 0x1, 0x80,
    0x0, 0x7f, 0xfc, 0x18, 0x0, 0x6, 0x0, 0x1,
    0x80, 0x0, 0x60, 0x0, 0x0,

    /* U+004D "M" */
    0x74, 0x0, 0x39, 0xf0, 0x3, 0xa6, 0x70, 0x35,
    0x98, 0x72, 0x86, 0x60, 0xbc, 0x19, 0x80, 0x80,
    0x66, 0x0, 0x1, 0x98, 0x0, 0x6, 0x60, 0x0,
    0x18,

    /* U+0053 "S" */
    0x3f, 0xff, 0x86, 0x0, 0x8, 0x60, 0x0, 0x6,
    0x0, 0x0, 0x3f, 0xff, 0x80, 0x0, 0xc, 0x0,
    0x0, 0x96, 0x0, 0xc, 0x3f, 0xff, 0x80,

    /* U+0054 "T" */
    0xff, 0xff, 0xc0, 0x30, 0x0, 0xc, 0x0, 0x3,
    0x0, 0x0, 0xc0, 0x0, 0x30, 0x0, 0xc, 0x0,
    0x3, 0x0, 0x0, 0xc0, 0x0,

    /* U+0057 "W" */
    0x60, 0xb, 0x0, 0x63, 0x0, 0xf0, 0xd, 0x24,
    0x19, 0x80, 0xc1, 0x82, 0x4c, 0x18, 0xc, 0x30,
    0xd3, 0x0, 0x96, 0x6, 0x30, 0x6, 0xd0, 0x3a,
    0x0, 0x3c, 0x2, 0xc0, 0x2, 0x80, 0x1c, 0x0,

    /* U+0061 "a" */
    0x7f, 0xfd, 0x0, 0x6, 0x0, 0x6, 0x7f, 0xfe,
    0x60, 0x6, 0x60, 0x6, 0x3f, 0xfe,

    /* U+0064 "d" */
    0x0, 0x9, 0x0, 0x9, 0x7f, 0xfd, 0x90, 0x9,
    0x90, 0x9, 0x90, 0x9, 0x90, 0x9, 0x90, 0x9,
    0x7f, 0xfd,

    /* U+0065 "e" */
    0x3f, 0xfd, 0x60, 0x6, 0x60, 0x6, 0x7f, 0xfe,
    0x60, 0x0, 0x60, 0x0, 0x3f, 0xfe,

    /* U+0068 "h" */
    0x60, 0x0, 0x60, 0x0, 0x7f, 0xfd, 0x60, 0x6,
    0x60, 0x6, 0x60, 0x6, 0x60, 0x6, 0x60, 0x6,
    0x60, 0x6,

    /* U+0069 "i" */
    0x60, 0x66, 0x66, 0x66, 0x60,

    /* U+006E "n" */
    0x7f, 0xfd, 0x60, 0x6, 0x60, 0x6, 0x60, 0x6,
    0x60, 0x6, 0x60, 0x6, 0x60, 0x6,

    /* U+006F "o" */
    0x3f, 0xfd, 0x60, 0x6, 0x60, 0x6, 0x60, 0x6,
    0x60, 0x6, 0x60, 0x6, 0x3f, 0xfd,

    /* U+0072 "r" */
    0x3f, 0xf6, 0x0, 0x60, 0x6, 0x0, 0x60, 0x6,
    0x0, 0x60, 0x0,

    /* U+0074 "t" */
    0x60, 0x18, 0x7, 0xf9, 0x80, 0x60, 0x18, 0x6,
    0x1, 0x80, 0x3f, 0x80,

    /* U+0075 "u" */
    0x60, 0x6, 0x60, 0x6, 0x60, 0x6, 0x60, 0x6,
    0x60, 0x6, 0x60, 0x6, 0x3f, 0xfd
};


/*---------------------
 *  GLYPH DESCRIPTION
 *--------------------*/

static const lv_font_fmt_txt_glyph_dsc_t glyph_dsc[] = {
    {.bitmap_index = 0, .adv_w = 0, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0} /* id = 0 reserved */,
    {.bitmap_index = 0, .adv_w = 52, .box_w = 0, .box_h = 0, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 0, .adv_w = 100, .box_w = 7, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 16, .adv_w = 160, .box_w = 10, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 39, .adv_w = 75, .box_w = 4, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 48, .adv_w = 159, .box_w = 10, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 71, .adv_w = 159, .box_w = 10, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 94, .adv_w = 140, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 115, .adv_w = 159, .box_w = 10, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 138, .adv_w = 157, .box_w = 10, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 161, .adv_w = 127, .box_w = 8, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 179, .adv_w = 160, .box_w = 10, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 202, .adv_w = 159, .box_w = 10, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 225, .adv_w = 41, .box_w = 2, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 229, .adv_w = 139, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 250, .adv_w = 178, .box_w = 11, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 275, .adv_w = 158, .box_w = 10, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 298, .adv_w = 146, .box_w = 9, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 319, .adv_w = 226, .box_w = 14, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 351, .adv_w = 133, .box_w = 8, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 365, .adv_w = 128, .box_w = 8, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 383, .adv_w = 133, .box_w = 8, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 397, .adv_w = 128, .box_w = 8, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 415, .adv_w = 40, .box_w = 2, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 420, .adv_w = 134, .box_w = 8, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 434, .adv_w = 133, .box_w = 8, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 448, .adv_w = 98, .box_w = 6, .box_h = 7, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 459, .adv_w = 79, .box_w = 5, .box_h = 9, .ofs_x = 0, .ofs_y = 0},
    {.bitmap_index = 471, .adv_w = 133, .box_w = 8, .box_h = 7, .ofs_x = 0, .ofs_y = 0}
};

/*---------------------
 *  CHARACTER MAPPING
 *--------------------*/

static const uint16_t unicode_list_0[] = {
    0x0, 0xf, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15,
    0x16, 0x17, 0x18, 0x19, 0x1a, 0x26, 0x2d, 0x33,
    0x34, 0x37, 0x41, 0x44, 0x45, 0x48, 0x49, 0x4e,
    0x4f, 0x52, 0x54, 0x55
};

/*Collect the unicode lists and glyph_id offsets*/
static const lv_font_fmt_txt_cmap_t cmaps[] =
{
    {
        .range_start = 32, .range_length = 86, .glyph_id_start = 1,
        .unicode_list = unicode_list_0, .glyph_id_ofs_list = NULL, .list_length = 28, .type = LV_FONT_FMT_TXT_CMAP_SPARSE_TINY
    }
};

/*-----------------
 *    KERNING
 *----------------*/


/*Pair left and right glyphs for kerning*/
static const uint8_t kern_pair_glyph_ids[] =
{
    14, 17,
    16, 18,
    17, 25,
    17, 28,
    18, 19,
    18, 21,
    18, 23,
    18, 25,
    18, 28,
    19, 24,
    19, 26,
    20, 21,
    20, 25,
    20, 28,
    21, 21,
    21, 26,
    22, 19,
    22, 23,
    23, 19,
    23, 24,
    24, 25,
    25, 19,
    25, 24,
    25, 25,
    25, 26,
    25, 27,
    25, 28,
    26, 25,
    27, 20,
    27, 22,
    27, 23,
    28, 20,
    28, 21,
    28, 24,
    28, 27
};

/* Kerning between the respective left and right glyphs
 * 4.4 format which needs to scaled with `kern_scale`*/
static const int8_t kern_pair_values[] =
{
    2, -5, -24, -19, -3, -7, -2, -5,
    -4, -3, -1, -4, -4, -4, -3, -1,
    1, 1, -7, -3, -3, -2, -3, -4,
    -2, -4, -4, -1, 4, 1, 1, 2,
    -3, 1, -3
};

/*Collect the kern pair's data in one place*/
static const lv_font_fmt_txt_kern_pair_t kern_pairs =
{
    .glyph_ids = kern_pair_glyph_ids,
    .values = kern_pair_values,
    .pair_cnt = 35,
    .glyph_ids_size = 0
};

/*--------------------
 *  ALL CUSTOM DATA
 *--------------------*/

#if LVGL_VERSION_MAJOR == 8
/*Store all the custom data of the font*/
static  lv_font_fmt_txt_glyph_cache_t cache;
#endif

#if LVGL_VERSION_MAJOR >= 8
static const lv_font_fmt_txt_dsc_t font_dsc = {
#else
static lv_font_fmt_txt_dsc_t font_dsc = {
#endif
    .glyph_bitmap = glyph_bitmap,
    .glyph_dsc = glyph_dsc,
    .cmaps = cmaps,
    .kern_dsc = &kern_pairs,
    .kern_scale = 16,
    .cmap_num = 1,
    .bpp = 2,
    .kern_classes = 0,
    .bitmap_format = 0,
#if LVGL_VERSION_MAJOR == 8
    .cache = &cache
#endif

};



/*-----------------
 *  PUBLIC FONT
 *----------------*/

/*Initialize a public general font descriptor*/
#if LVGL_VERSION_MAJOR >= 8
const lv_font_t font_orbitron_12_2 = {
#else
lv_font_t font_orbitron_12_2 = {
#endif
    .get_glyph_dsc = lv_font_get_glyph_dsc_fmt_txt,    /*Function pointer to get glyph's data*/
    .get_glyph_bitmap = lv_font_get_bitmap_fmt_txt,    /*Function pointer to get glyph's bitmap*/
    .line_height = 9,          /*The maximum line height required by the font*/
    .base_line = 0,             /*Baseline measured from the bottom of the line*/
#if !(LVGL_VERSION_MAJOR == 6 && LVGL_VERSION_MINOR == 0)
    .subpx = LV_FONT_SUBPX_NONE,
#endif
#if LV_VERSION_CHECK(7, 4, 0) || LVGL_VERSION_MAJOR >= 8
    .underline_position = -1,
    .underline_thickness = 1,
#endif
    .dsc = &font_dsc,          /*The custom font data. Will be accessed by `get_glyph_bitmap/dsc` */
#if LV_VERSION_CHECK(8, 2, 0) || LVGL_VERSION_MAJOR >= 9
    .fallback = NULL,
#endif
    .user_data = NULL,
};



#endif /*#if FONT_ORBITRON_12_2*/
