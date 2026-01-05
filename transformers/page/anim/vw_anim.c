#include "lvgl.h"
#include "cl_ui.h"
#include "vw_anim.h"
#include "string.h"
#include "lv_vpg/lv_vpg.h"

static anim_view_t vw;

#define TAG "vw_anim"
#define ANIM_PATH "P:/VIDEO"

LV_IMG_DECLARE(icon_photo_64)

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
                page_change("home");
            }
            break;
        case CL_UI_KEY_MODE:
            if (btn->event == CL_BTN_CLICK)
            {
                page_change("music_fft");
            }
            break;
        default:
            break;
    }
}

// 扫描视频
static void _scan_vpg() {
    lv_fs_dir_t dir;
    lv_fs_res_t res = lv_fs_dir_open(&dir, ANIM_PATH);
    if(res != LV_FS_RES_OK) {
        LV_LOG_WARN("scan_file: could not open dir %s", ANIM_PATH);
        return;
    }
    char fn[256];
    // 动态分配vw.vpg_list空间，存文件名到vw.vpg_list，数量存vw.vpg_count
    char **list = NULL;
    int count = 0;
    int capacity = 8; // 初始容量
    list = (char **)malloc(sizeof(char*) * capacity);
    char *def_vpg = xz_setting_get_string("anim", NULL);
    while(lv_fs_dir_read(&dir, fn, sizeof(fn)) == LV_FS_RES_OK) {
        if(fn[0] == '\0') break;

        int len = strlen(fn);
        if(len > 4) {
            const char *ext = &fn[len - 4];
            if(strcasecmp(ext, ".vpg") == 0) {
                ESP_LOGI(TAG, "Found vpg: %s", fn);
                // 扩容
                if(count >= capacity) {
                    capacity *= 2;
                    char **tmp = (char **)realloc(list, sizeof(char*) * capacity);
                    if(tmp) list = tmp;
                    else break; // 分配失败退出
                }
                list[count] = strdup(fn);
                if (def_vpg && strcmp(def_vpg, fn) == 0)
                {
                    vw.vpg_index = count;
                }
                count++;
            }
        }

    }
    if(list) {
        // 如果刚好有文件，最后缩小容量
        char **tmp = (char **)realloc(list, sizeof(char*) * count);
        if(tmp) list = tmp;
    }
    // 释放旧的vpg_list
    for (int i = 0; i < vw.vpg_count; i++)
    {
        if (vw.vpg_list[i])
        {
            free(vw.vpg_list[i]);
        }
    }
    if (vw.vpg_list)
    {
        free(vw.vpg_list);
        vw.vpg_list = NULL;
    }
    if (def_vpg != NULL)
    {
        free(def_vpg);
    }
    
    
    vw.vpg_list = list;
    vw.vpg_count = count;  // 注意home_view_t结构需有vpg_count成员
    lv_fs_dir_close(&dir);
}

// 0: current 1: next -1: previous
static void _vpg_switch(lv_obj_t *vpg, int offset){
    if (vw.vpg_count <= 0)
    {
        lv_vpg_set_img(vpg, &icon_photo_64);
        lv_obj_center(vpg);    
        lv_obj_remove_flag(vw.label, LV_OBJ_FLAG_HIDDEN);
        return;
    }else
    {
        char path[64] = {0};
        vw.vpg_index = (vw.vpg_index + offset + vw.vpg_count) % vw.vpg_count;
        snprintf(path, sizeof(path), ANIM_PATH"/%s", vw.vpg_list[vw.vpg_index]);
        lv_vpg_set_src(vw.anim, path);
    }
}

static void _on_swipe_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_GESTURE)
    {
        lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_active());
        if (dir == LV_DIR_LEFT)
        {
            _vpg_switch(vw.anim, 1);
        }else if (dir == LV_DIR_RIGHT)
        {
            _vpg_switch(vw.anim, -1);
        }else if (dir == LV_DIR_TOP)
        {
            cl_arc_menu_show(1);
        }
        
    }
}

static void _vpg_play_done(lv_event_t *e){
    uint32_t *process = lv_event_get_param(e);
    if (process == NULL || vw.is_act == 0)
    {
        return;
    }
    if (vw.auto_play)
    {
        _vpg_switch(vw.anim, 1);
    }
    
}

anim_view_t* anim_view_create(lv_obj_t *root)
{
    lv_obj_remove_style_all(root);
    lv_obj_set_size(root, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_bg_color(root, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(root, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_remove_flag(root, LV_OBJ_FLAG_SCROLLABLE);

    _scan_vpg();

    vw.anim = lv_vpg_create(root);
    lv_obj_t *label = lv_label_create(root);
    lv_obj_center(vw.anim);
    vw.label = label;
    lv_obj_set_style_text_font(label, cl_ui_get_font(), 0);
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 60);
    lv_label_set_text(label, "请先上传视频");
    lv_obj_add_flag(label, LV_OBJ_FLAG_HIDDEN);
    _vpg_switch(vw.anim, 0);

    vw.auto_play = xz_setting_get_int("anim_play", 1);

    lv_obj_add_event_cb(root, _on_btn_cb, CL_UI_EVENT_BUTTON, NULL);
    lv_obj_add_event_cb(root, _on_swipe_cb, LV_EVENT_GESTURE, NULL);
    lv_obj_clear_flag(root, LV_OBJ_FLAG_GESTURE_BUBBLE);
    lv_obj_add_event_cb(vw.anim, _vpg_play_done, LV_EVENT_READY, NULL);
    return &vw;
}

void anim_view_delete(void)
{
    /* 如需释放资源在此处理 */
}
