#include "lvgl.h"
#include "cl_ui.h"
#include "vw_setting_dev.h"
#ifndef SIMULATOR
#include <esp_event.h>
#endif

static setting_dev_view_t vw;
LV_FONT_DECLARE(font_noto_28_4)

#define TAG "vw_setting_dev"

static void _on_btn_cb(lv_event_t *e)
{
    if (!vw.is_act) return;

    cl_button_t *btn = (cl_button_t*)lv_event_get_param(e);
    if (!btn) return;

    switch (btn->id)
    {
        case CL_UI_KEY_POWER:
            if (btn->event == CL_BTN_CLICK)
            {
                page_change("setting");
            }
            break;
        default:
            break;
    }
}

static void _on_dev_mode_sw(lv_event_t *e)
{
    lv_obj_t *sw = lv_event_get_current_target(e);
    if (lv_obj_has_state(sw, LV_STATE_CHECKED)) {
        xz_setting_set_int("debug", 3);
    } else {
        xz_setting_set_int("debug", 0);
    }
}

static void _on_dialog_cancel(lv_event_t *e)
{
    // 获取事件目标（btn），然后获取上层cont
    lv_obj_t *btn = lv_event_get_target(e);
    if (!btn) return;

    lv_obj_t *btn_row = lv_obj_get_parent(btn);
    if (!btn_row) return;

    lv_obj_t *dialog = lv_obj_get_parent(btn_row);
    if (!dialog) return;

    lv_obj_t *cont = lv_obj_get_parent(dialog);
    if (!cont) return;

    lv_obj_del(cont); // 删除整个遮罩层
}

static void _on_dialog_accept(lv_event_t *e)
{
    // 可根据需求在这里处理“确定”事件
    lv_obj_t *btn = lv_event_get_target(e);
    if (!btn) return;

    lv_obj_t *btn_row = lv_obj_get_parent(btn);
    if (!btn_row) return;

    lv_obj_t *dialog = lv_obj_get_parent(btn_row);
    if (!dialog) return;

    lv_obj_t *cont = lv_obj_get_parent(dialog);
    if (!cont) return;

    // 获取 void (*)(void*) 回调
    void (*accept)(void*) = (void (*)(void*))lv_event_get_user_data(e);
    if (accept) {
        accept(NULL);
    }

    lv_obj_del(cont); // 同样删除整个遮罩层
}


static void _create_dialog(lv_obj_t *parent, const char *msg, void (*accept)(void*), void *arg){
    lv_obj_t *cont = lv_obj_create(parent);
    lv_obj_set_style_bg_color(cont, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(cont, LV_OPA_40, 0);
    lv_obj_set_size(cont, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_pad_bottom(cont, 0, 0);
    lv_obj_set_style_border_width(cont, 0, 0);
    
    lv_obj_t *dialog = lv_obj_create(cont);
    lv_obj_set_width(dialog, 200);
    lv_obj_set_style_bg_color(dialog, lv_color_hex(0x333333), 0);
    lv_obj_set_style_bg_opa(dialog, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(dialog, 20, 0);
    lv_obj_set_style_pad_bottom(dialog, 0, 0);
    lv_obj_set_style_border_width(dialog, 0, 0);
    lv_obj_center(dialog);

    lv_obj_t *label = lv_label_create(dialog);
    lv_obj_set_style_text_color(label, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_text_font(label, cl_ui_get_font(), 0);
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP); // 自动换行
    lv_obj_set_width(label, LV_PCT(100)); // 设置label宽度，便于换行
    lv_label_set_text(label, msg);

    lv_obj_set_style_min_height(dialog, 100, 0);
    lv_obj_set_style_max_height(dialog, 200, 0);
    lv_obj_set_height(dialog, LV_SIZE_CONTENT);
    // // 创建一个容器用于放置两个按钮，并设置为水平居中
    lv_obj_t *btn_row = lv_obj_create(dialog);
    lv_obj_remove_style_all(btn_row);
    lv_obj_set_style_pad_all(btn_row, 0, 0);
    lv_obj_align_to(btn_row, label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 10);
    lv_obj_set_width(btn_row, LV_PCT(100));
    lv_obj_set_height(btn_row, 40);
    lv_obj_set_flex_flow(btn_row, LV_FLEX_FLOW_ROW);
    lv_obj_set_flex_align(btn_row, LV_FLEX_ALIGN_SPACE_EVENLY, LV_FLEX_ALIGN_CENTER, LV_FLEX_ALIGN_CENTER);
    lv_obj_set_style_bg_opa(btn_row, LV_OPA_TRANSP, 0);
    lv_obj_set_style_pad_all(btn_row, 0, 0);
    lv_obj_set_style_border_width(btn_row, 0, 0);

    // 左按钮
    lv_obj_t *btn_left = lv_btn_create(btn_row);
    lv_obj_align_to(btn_left, label, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 80);
    lv_obj_set_width(btn_left, 80);
    lv_obj_set_height(btn_left, 40);
    lv_obj_center(btn_left);
    lv_obj_t *lbl_left = lv_label_create(btn_left);
    lv_obj_set_style_text_font(lbl_left, cl_ui_get_font(), 0);
    lv_label_set_text(lbl_left, "取消");
    lv_obj_center(lbl_left);

    // 右按钮
    lv_obj_t *btn_right = lv_btn_create(btn_row);
    lv_obj_set_width(btn_right, 80);
    lv_obj_set_height(btn_right, 40);
    lv_obj_center(btn_right);
    lv_obj_t *lbl_right = lv_label_create(btn_right);
    lv_obj_set_style_text_font(lbl_right, cl_ui_get_font(), 0);
    lv_label_set_text(lbl_right, "确定");
    lv_obj_center(lbl_right);

    // 按钮回调（如需实现，可在此处添加事件）
    lv_obj_add_event_cb(btn_right, _on_dialog_accept, LV_EVENT_SHORT_CLICKED, accept);
    lv_obj_add_event_cb(btn_left, _on_dialog_cancel, LV_EVENT_SHORT_CLICKED, accept);

}

static void _on_reboot(void *arg){
#ifndef SIMULATOR
    esp_restart();
#endif
}

static void _on_reboot_cb(lv_event_t *e)
{
    lv_obj_t *root = lv_event_get_user_data(e);
    _create_dialog(root, "确定重启吗", _on_reboot, NULL);
}

setting_dev_view_t* setting_dev_view_create(lv_obj_t *root)
{
    lv_obj_remove_style_all(root);
    lv_obj_set_size(root, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_bg_color(root, lv_color_hex(0x000000), 0);
    lv_obj_set_style_bg_opa(root, LV_OPA_COVER, 0);
    lv_obj_set_scroll_dir(root, LV_DIR_VER);
    lv_obj_set_scrollbar_mode(root, LV_SCROLLBAR_MODE_OFF);

    lv_obj_t *title = lv_label_create(root);
    lv_obj_set_style_text_font(title, &font_noto_28_4, 0);
    lv_obj_set_style_text_color(title, lv_color_hex(0xffffff), 0);
    lv_label_set_text(title, "开发者设置");
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 40);

    lv_obj_t *cont_dev_mode = lv_obj_create(root);
    lv_obj_align(cont_dev_mode, LV_ALIGN_TOP_MID, 0, 80);
    lv_obj_set_size(cont_dev_mode, 280, 80);
    lv_obj_set_style_border_width(cont_dev_mode, 0, 0);
    lv_obj_set_style_bg_color(cont_dev_mode, lv_color_hex(0x222222), 0);
    lv_obj_set_style_radius(cont_dev_mode, LV_RADIUS_CIRCLE, 0);
    
    lv_obj_t *label_dev_mode = lv_label_create(cont_dev_mode);
    lv_obj_align(label_dev_mode, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_set_style_text_font(label_dev_mode, &font_noto_28_4, 0);
    lv_obj_set_style_text_color(label_dev_mode, lv_color_hex(0xffffff), 0);
    lv_label_set_text(label_dev_mode, "开发者模式");

    lv_obj_t *switch_dev_mode = lv_switch_create(cont_dev_mode);
    lv_obj_set_size(switch_dev_mode, 80, 40);
    lv_obj_align(switch_dev_mode, LV_ALIGN_RIGHT_MID, 0, 0);
    lv_obj_set_style_bg_color(switch_dev_mode, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_radius(switch_dev_mode, LV_RADIUS_CIRCLE, 0);
    lv_obj_set_style_border_width(switch_dev_mode, 0, 0);
    lv_obj_set_style_border_color(switch_dev_mode, lv_color_hex(0xffffff), 0);
    /* 关闭状态下使用灰底 + 白色圆点，保持开启时默认配色 */
    lv_obj_set_style_bg_color(switch_dev_mode, lv_color_hex(0x5a5a5a), LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(switch_dev_mode, LV_OPA_COVER, LV_PART_MAIN | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_color(switch_dev_mode, lv_color_hex(0xffffff), LV_PART_KNOB | LV_STATE_DEFAULT);
    lv_obj_set_style_bg_opa(switch_dev_mode, LV_OPA_COVER, LV_PART_KNOB | LV_STATE_DEFAULT);

    // 新增一个重启菜单
    lv_obj_t *cont_reboot = lv_obj_create(root);
    lv_obj_align(cont_reboot, LV_ALIGN_TOP_MID, 0, 180);
    lv_obj_set_size(cont_reboot, 280, 80);
    lv_obj_set_style_border_width(cont_reboot, 0, 0);
    lv_obj_set_style_bg_color(cont_reboot, lv_color_hex(0x222222), 0);
    lv_obj_set_style_radius(cont_reboot, LV_RADIUS_CIRCLE, 0);

    lv_obj_t *label_reboot = lv_label_create(cont_reboot);
    lv_obj_align(label_reboot, LV_ALIGN_LEFT_MID, 0, 0);
    lv_obj_set_style_text_font(label_reboot, &font_noto_28_4, 0);
    lv_obj_set_style_text_color(label_reboot, lv_color_hex(0xffffff), 0);
    lv_label_set_text(label_reboot, "重启设备");

    lv_obj_add_event_cb(cont_reboot, _on_reboot_cb, LV_EVENT_CLICKED, root);

    if (xz_setting_get_int("debug", 0) == 3)
    {
        lv_obj_add_state(switch_dev_mode, LV_STATE_CHECKED);
    }
    lv_obj_add_event_cb(switch_dev_mode, _on_dev_mode_sw, LV_EVENT_VALUE_CHANGED, NULL);
    lv_obj_add_event_cb(root, _on_btn_cb, CL_UI_EVENT_BUTTON, NULL);

    return &vw;
}

void setting_dev_view_delete(void)
{
    /* 如需释放资源在此处理 */
}
