#include "help_view.h"
#include <stdio.h>
#include <stdlib.h>
#include "lvgl.h"
#include "cl_ui.h"
#include "page_manager.h"

static help_view_t vw;
LV_FONT_DECLARE(font_puhui_18_4)
LV_IMG_DECLARE(img_qr_help)
LV_IMG_DECLARE(icon_back_14)

// 返回按钮点击事件
static void _on_back_click(lv_event_t * e)
{
    page_change("setting");
}

help_view_t* help_view_create(lv_obj_t *root)
{
    lv_obj_remove_style_all(root);
    lv_obj_set_size(root, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_pad_all(root, 0, 0);
    lv_obj_set_style_border_width(root, 0, 0);
    lv_obj_set_style_bg_opa(root, LV_OPA_COVER, 0);
    lv_obj_set_style_bg_color(root, lv_color_hex(0x000000), 0);
    lv_obj_clear_flag(root, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_center(root);
    
    // 初始化视图结构
    memset(&vw, 0, sizeof(help_view_t));
    
    lv_obj_t *btn_back = lv_button_create(root);
    lv_obj_align(btn_back, LV_ALIGN_TOP_LEFT, 20, 10);
    lv_obj_set_style_shadow_width(btn_back, 0, NULL);
    lv_obj_set_style_bg_color(btn_back, lv_color_black(),0);
    lv_obj_clear_flag(btn_back, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_add_event_cb(btn_back, _on_back_click, LV_EVENT_CLICKED, NULL);
    lv_obj_t *btn_img =  lv_img_create(btn_back);
    lv_img_set_src(btn_img, &icon_back_14);
    lv_obj_center(btn_img);

    lv_obj_t *title_label = lv_label_create(root);
    lv_label_set_text(title_label, LT_USE_HELP);
    lv_obj_set_style_text_color(title_label, lv_color_hex(0xe9feff), 0);
    lv_obj_align_to(title_label, btn_back, LV_ALIGN_OUT_RIGHT_MID, 5, 0);

    // 二维码图片
    // vw.qr_code_img = lv_img_create(root);
    // lv_img_set_src(vw.qr_code_img, &img_qr_help);
    // lv_obj_align_to(vw.qr_code_img, title_label, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);
    lv_obj_t * qr = lv_qrcode_create(root);
    vw.qr_code_img = qr;
    lv_qrcode_set_size(qr, 150);
    lv_qrcode_set_dark_color(qr, lv_color_white());
    lv_qrcode_set_light_color(qr, lv_color_black());
    
    /*Set data*/
    const char * data = "https://tui.doit.am/web/ai_speaker/index.html";
    lv_qrcode_update(qr, data, strlen(data));
    lv_obj_center(qr);


    // 帮助文字
    vw.help_text_label = lv_label_create(root);
    lv_obj_set_style_text_font(vw.help_text_label, &font_puhui_18_4, 0);
    lv_label_set_text(vw.help_text_label, LT_SCAN_GET_HELP);
    lv_obj_set_width(vw.help_text_label, 200);
    lv_label_set_long_mode(vw.help_text_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_color(vw.help_text_label, lv_color_hex(0xcccccc), 0);
    lv_obj_set_style_text_align(vw.help_text_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align_to(vw.help_text_label, vw.qr_code_img, LV_ALIGN_OUT_BOTTOM_MID, 0, 10);

    return &vw;
}

void help_view_delete(void)
{
}

void help_view_appear_anim_start(bool reverse)
{
} 