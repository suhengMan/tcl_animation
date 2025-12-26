#include "lvgl.h"
#include "cl_ui.h"
#include "vw_phone_call.h"
#ifndef SIMULATOR
#include "vb_adapter.h"
#endif

static phone_call_view_t vw;
LV_IMG_DECLARE(icon_phone_answer_32)
LV_IMG_DECLARE(icon_phone_handup_32)

#define TAG "vw_phone_call"

static void _on_btn_cb(lv_event_t *e)
{
    if (!vw.is_act) return;

    cl_button_t *btn = (cl_button_t*)lv_event_get_param(e);
    if (!btn) return;
#ifndef SIMULATOR
    if (btn->id == CL_UI_KEY_POWER)
    {
        if (btn->event == CL_BTN_CLICK)
        {    
            vb_api_phone_call_answer(true);
            lv_obj_add_flag(vw.btn_answer, LV_OBJ_FLAG_HIDDEN);
            lv_obj_align(vw.btn_hangup, LV_ALIGN_BOTTOM_MID, 0, -80);
        }else if(btn->event == CL_BTN_DOUBLE_CLICK){

            vb_api_phone_call_answer(false);
            page_change("home");
        }
    }
#endif
}

static void _on_answer_btn(lv_event_t *e)
{
    bool answer = ((intptr_t)lv_event_get_user_data(e))==1?true:false;
    vb_api_phone_call_answer(answer);
    if (answer)
    {
        lv_obj_add_flag(vw.btn_answer, LV_OBJ_FLAG_HIDDEN);
        lv_obj_align(vw.btn_hangup, LV_ALIGN_BOTTOM_MID, 0, -80);
    }
    
}

static void _on_hangup(lv_event_t *e)
{
    page_change("home");    
}

phone_call_view_t* phone_call_view_create(lv_obj_t *root, const char *phone)
{
    lv_obj_remove_style_all(root);
    lv_obj_set_size(root, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_bg_color(root, lv_color_black(),0);
    lv_obj_set_style_bg_opa(root, LV_OPA_COVER, 0);
    lv_obj_clear_flag(root, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(root, LV_SCROLLBAR_MODE_OFF);
    lv_obj_center(root);
    
    // 创建挂断按钮（左边，红色）
    lv_obj_t *btn_hangup = lv_btn_create(root);
    lv_obj_set_size(btn_hangup, 80, 80);
    lv_obj_align(btn_hangup, LV_ALIGN_BOTTOM_MID, -80, -80);
    lv_obj_set_style_bg_color(btn_hangup, lv_color_hex(0xFF3B30), LV_PART_MAIN); // 红色
    lv_obj_set_style_radius(btn_hangup, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_user_data(btn_hangup, (void*)(intptr_t)CL_UI_KEY_POWER); // 假定使用power key表示挂断
    lv_obj_add_flag(btn_hangup, LV_OBJ_FLAG_EVENT_BUBBLE);
    lv_obj_add_event_cb(btn_hangup, _on_btn_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *label_phone = lv_label_create(root);
    lv_obj_set_style_text_font(label_phone, cl_ui_get_font(), LV_PART_MAIN);
    lv_obj_set_style_text_color(label_phone, lv_color_white(), LV_PART_MAIN);
    lv_obj_align(label_phone, LV_ALIGN_TOP_MID, 0, 100);
    lv_obj_set_style_text_align(label_phone, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(label_phone, 200);
    lv_obj_set_style_transform_zoom(label_phone, 400, 0); // 256=2x LVGL zoom
    lv_obj_set_style_transform_pivot_x(label_phone, 100, 0);
    lv_obj_set_style_transform_pivot_y(label_phone, 10, 0);
    
#ifdef SIMULATOR
    lv_label_set_text(label_phone, "130xxxxxxxx");
#else 
    ESP_LOGI(TAG, "NUMBER:%s", phone);
    lv_label_set_text(label_phone, phone);
#endif


    lv_obj_t *img_hangup = lv_img_create(btn_hangup);
    lv_img_set_src(img_hangup, &icon_phone_handup_32);
    lv_obj_center(img_hangup);

    // 创建接听按钮（右边，绿色）
    lv_obj_t *btn_accept = lv_btn_create(root);
    lv_obj_set_size(btn_accept, 80, 80);
    lv_obj_align(btn_accept, LV_ALIGN_BOTTOM_MID, 80, -80);
    lv_obj_set_style_bg_color(btn_accept, lv_color_hex(0x4CD964), LV_PART_MAIN); // 绿色
    lv_obj_set_style_radius(btn_accept, LV_RADIUS_CIRCLE, LV_PART_MAIN);
    lv_obj_set_user_data(btn_accept, (void*)(intptr_t)CL_UI_KEY_MODE); // 假定使用mode key表示接听
    lv_obj_add_flag(btn_accept, LV_OBJ_FLAG_EVENT_BUBBLE);
    lv_obj_add_event_cb(btn_accept, _on_btn_cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *img_accept = lv_img_create(btn_accept);
    lv_img_set_src(img_accept, &icon_phone_answer_32);
    lv_obj_center(img_accept);

    vw.btn_answer = btn_accept;
    vw.btn_hangup = btn_hangup;

    lv_obj_add_event_cb(btn_hangup, _on_answer_btn, LV_EVENT_CLICKED, (void*)(intptr_t)0);
    lv_obj_add_event_cb(btn_accept, _on_answer_btn, LV_EVENT_CLICKED, (void*)(intptr_t)1);
    lv_obj_add_event_cb(root, _on_btn_cb, CL_UI_EVENT_BUTTON, NULL);
    lv_obj_add_event_cb(root, _on_hangup, CL_UI_EVENT_PHONE_CALL_HANGUP, NULL);

    return &vw;
}

void phone_call_view_delete(void)
{
    /* 如需释放资源在此处理 */
}
