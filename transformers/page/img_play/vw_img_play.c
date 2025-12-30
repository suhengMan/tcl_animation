#include "lvgl.h"
#include "cl_ui.h"
#include "string.h"
#include <ctype.h>
#include "vw_img_play.h"

static img_play_view_t vw;

#define TAG "vw_img_play"
#define IMAGE_PATH "P:/HOME"

LV_IMG_DECLARE(icon_photo_64)


LV_IMG_DECLARE(icon_alarm_28)
LV_IMG_DECLARE(icon_setting_28)
LV_IMG_DECLARE(icon_music_28)
LV_IMG_DECLARE(icon_video_28)
LV_IMG_DECLARE(icon_image_28)
LV_IMG_DECLARE(icon_countdown_32)

// 扫描照片
static void _scan_jpg(){
    lv_fs_dir_t dir;
    lv_fs_res_t res = lv_fs_dir_open(&dir, IMAGE_PATH);
    if(res != LV_FS_RES_OK) {
        LV_LOG_WARN("scan_file: could not open dir %s", IMAGE_PATH);
        return;
    }
    char fn[256];
    // 动态分配vw.vpg_list空间，存文件名到vw.vpg_list，数量存vw.vpg_count
    char **list = NULL;
    int count = 0;
    int capacity = 8; // 初始容量
    list = (char **)malloc(sizeof(char*) * capacity);
    char *def_jpg = xz_setting_get_string("img", NULL);
    while(lv_fs_dir_read(&dir, fn, sizeof(fn)) == LV_FS_RES_OK) {
        if(fn[0] == '\0') break;

        int len = strlen(fn);
        if(len > 4) {
            const char *ext = &fn[len - 4];
            if(strcasecmp(ext, ".jpg") == 0) {
                // ESP_LOGI(TAG, "Found jpg: %s", fn);
                // 扩容
                if(count >= capacity) {
                    capacity *= 2;
                    char **tmp = (char **)realloc(list, sizeof(char*) * capacity);
                    if(tmp) list = tmp;
                    else break; // 分配失败退出
                }
                #ifndef SIMULATOR
                // 转小写
                for (int k = 0; fn[k]; k++) {
                    fn[k] = tolower((unsigned char)fn[k]);
                }
                #endif
                list[count] = strdup(fn);
                if (def_jpg && strcmp(def_jpg, fn) == 0)
                {
                    vw.jpg_index = count;
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
    for (int i = 0; i < vw.jpg_count; i++)
    {
        if (vw.jpg_list[i])
        {
            free(vw.jpg_list[i]);
        }
    }
    if (vw.jpg_list)
    {
        free(vw.jpg_list);
        vw.jpg_list = NULL;
    }
    if (def_jpg != NULL)
    {
        free(def_jpg);
    }
    
    
    vw.jpg_list = list;
    vw.jpg_count = count;  // 注意home_view_t结构需有vpg_count成员
    lv_fs_dir_close(&dir);
}

// 0: current 1: next -1: previous
static void _jpg_switch(lv_obj_t *img, int offset){
    if (vw.jpg_count <= 0)
    {
        lv_img_set_src(img, &icon_photo_64);
        lv_obj_center(img);    
        lv_obj_remove_flag(vw.label, LV_OBJ_FLAG_HIDDEN);
        return;
    }else
    {
        char path[64] = {0};
        vw.jpg_index = (vw.jpg_index + offset + vw.jpg_count) % vw.jpg_count;
        snprintf(path, sizeof(path), IMAGE_PATH"/%s", vw.jpg_list[vw.jpg_index]);
        lv_img_set_src(img, path);
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
            _jpg_switch(vw.img, 1);
        }else if (dir == LV_DIR_RIGHT)
        {
            _jpg_switch(vw.img, -1);
        }else if (dir == LV_DIR_TOP)
        {
            cl_arc_menu_show(1);
        }
        
    }
}


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

img_play_view_t* img_play_view_create(lv_obj_t *root)
{
    lv_obj_remove_style_all(root);
    lv_obj_set_size(root, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_bg_color(root, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(root, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_remove_flag(root, LV_OBJ_FLAG_SCROLLABLE);
    _scan_jpg();

    vw.img = lv_img_create(root);
    
    
    lv_obj_t *label = lv_label_create(root);
    vw.label = label;
    lv_obj_set_style_text_font(label, cl_ui_get_font(), 0);
    lv_obj_set_style_text_color(label, lv_color_white(), 0);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 60);
    lv_label_set_text(label, "请先上传图片");
    lv_obj_add_flag(label, LV_OBJ_FLAG_HIDDEN);
    _jpg_switch(vw.img, 0);

    lv_obj_add_event_cb(root, _on_btn_cb, CL_UI_EVENT_BUTTON, NULL);
    lv_obj_add_event_cb(root, _on_swipe_cb, LV_EVENT_GESTURE, NULL);
    lv_obj_clear_flag(root, LV_OBJ_FLAG_GESTURE_BUBBLE);
    return &vw;
}

void img_play_view_delete(void)
{
    /* 如需释放资源在此处理 */
}
