#include "lvgl.h"
#include "cl_ui.h"
#include "vw_home.h"
#include <string.h>
#include "ctype.h"
#include "lv_arc_menu/lv_arc_menu.h"
#include "../../lv_toast/lv_toast.h"
#include "../../lv_vpg/lv_vpg.h"
static home_view_t vw = {0};

#define TAG "vw_home"

#ifndef SIMULATOR
#define ASSERT_PREXI "P:/SYS/"
#else
#define ASSERT_PREXI "P:/SYS/"
#endif

#define STATE_CONNECTING "连接中..."
#define STATE_ERROR      "错误"
#define STATE_STANDBY    "待命"
#define STATE_SPEAKING   "说话中..."
#define STATE_LISTENING  "聆听中..."


LV_IMG_DECLARE(icon_mic);
LV_IMG_DECLARE(icon_speaker_zzz);
LV_IMG_DECLARE(icon_WiFi_failed);
LV_IMG_DECLARE(icon_wifi);

LV_IMG_DECLARE(icon_alarm_28)
LV_IMG_DECLARE(icon_setting_28)
LV_IMG_DECLARE(icon_music_28)
LV_IMG_DECLARE(icon_video_28)
LV_IMG_DECLARE(icon_image_28)
LV_IMG_DECLARE(icon_countdown_32)

typedef struct 
{
    char *emoji;
    char *file;
    bool loop;
}emoji_map_t;

// static void cl_arc_menu_show(bool show);
/*
[
    {"emote": "happy",       "src": "Happy.eaf",     "loop": true,  "fps": 20},
    {"emote": "laughing",    "src": "Happy.eaf",     "loop": true,  "fps": 20},
    {"emote": "funny",       "src": "Happy.eaf",     "loop": true,  "fps": 20},
    {"emote": "loving",      "src": "Happy.eaf",     "loop": true,  "fps": 20},
    {"emote": "embarrassed", "src": "Happy.eaf",     "loop": true,  "fps": 20},
    {"emote": "confident",   "src": "Happy.eaf",     "loop": true,  "fps": 20},
    {"emote": "delicious",   "src": "Happy.eaf",     "loop": true,  "fps": 20},
    {"emote": "sad",         "src": "Sad.eaf",       "loop": true,  "fps": 20},
    {"emote": "crying",      "src": "cry.eaf",       "loop": true,  "fps": 20},
    {"emote": "sleepy",      "src": "sleep.eaf",     "loop": true,  "fps": 20},
    {"emote": "silly",       "src": "Happy.eaf",     "loop": true,  "fps": 20},
    {"emote": "angry",       "src": "angry.eaf",     "loop": true,  "fps": 20},
    {"emote": "surprised",   "src": "Happy.eaf",     "loop": true,  "fps": 20},
    {"emote": "shocked",     "src": "shocked.eaf",   "loop": true,  "fps": 20},
    {"emote": "thinking",    "src": "confused.eaf",  "loop": true,  "fps": 20},
    {"emote": "winking",     "src": "neutral.eaf",   "loop": true,  "fps": 20},
    {"emote": "relaxed",     "src": "Happy.eaf",     "loop": true,  "fps": 20},
    {"emote": "confused",    "src": "confused.eaf",  "loop": true,  "fps": 20},
    {"emote": "neutral",     "src": "winking.eaf",   "loop": false, "fps": 20},
    {"emote": "idle",        "src": "neutral.eaf",   "loop": false, "fps": 20}
]



// */
// static emoji_map_t emoji_map[] = {
//     {"happy", ASSERT_PREXI"happy.vpg", true},
//     {"laughing", ASSERT_PREXI"happy.vpg", true},
//     {"funny", ASSERT_PREXI"happy.vpg", true},
//     {"loving", ASSERT_PREXI"happy.vpg", true},
//     {"embarrassed", ASSERT_PREXI"happy.vpg", true},
//     {"confident", ASSERT_PREXI"happy.vpg", true},
//     {"delicious", ASSERT_PREXI"happy.vpg", true},
//     {"sad", ASSERT_PREXI"sad.vpg", true},
//     {"crying", ASSERT_PREXI"cry.vpg", true},
//     {"sleepy", ASSERT_PREXI"sleep.vpg", true},
//     {"silly", ASSERT_PREXI"happy.vpg", true},
//     {"angry", ASSERT_PREXI"angry.vpg", true},
//     {"surprised", ASSERT_PREXI"happy.vpg", true},
//     {"shocked", ASSERT_PREXI"shocked.vpg", true},
//     {"thinking", ASSERT_PREXI"confused.vpg", true},
//     {"winking", ASSERT_PREXI"winking.vpg", true},
//     {"relaxed", ASSERT_PREXI"happy.vpg", true},
//     {"confused", ASSERT_PREXI"confused.vpg", true},
//     {"neutral", ASSERT_PREXI"neutral.vpg", true},
//     {"idle", ASSERT_PREXI"neutral.vpg", false}
// };

static emoji_map_t emoji_map[] = {
    {"neutral",     ASSERT_PREXI"微笑30.vpg",      true},
    {"happy",       ASSERT_PREXI"大笑.vpg",      true},
    {"laughing",    ASSERT_PREXI"大笑.vpg",      true},
    {"funny",       ASSERT_PREXI"调皮.vpg",      true},
    {"sad",         ASSERT_PREXI"委屈.vpg",      true},
    {"angry",       ASSERT_PREXI"生气.vpg",      true},
    {"crying",      ASSERT_PREXI"流泪.vpg",      true},
    {"loving",      ASSERT_PREXI"心动.vpg",      true},
    {"embarrassed", ASSERT_PREXI"尴尬.vpg",      true},
    {"surprised",   ASSERT_PREXI"无语.vpg",      true},
    {"shocked",     ASSERT_PREXI"震惊.vpg",      true},
    {"thinking",    ASSERT_PREXI"思考.vpg",      true},
    {"winking",     ASSERT_PREXI"微笑.vpg",      true},
    {"cool",        ASSERT_PREXI"发呆.vpg",      true},
    {"relaxed",     ASSERT_PREXI"睡觉.vpg",      true},
    {"delicious",   ASSERT_PREXI"晕.vpg",        true},  // ⚠️ 暂代，建议替换为“流口水/馋”类资源
    {"kissy",       ASSERT_PREXI"亲吻.vpg",      true},
    {"confident",   ASSERT_PREXI"工作学习.vpg",  true},
    {"sleepy",      ASSERT_PREXI"困.vpg",        true},
    {"silly",       ASSERT_PREXI"玩耍.vpg",      true},
    {"confused",    ASSERT_PREXI"疑惑.vpg",      true},
};

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
                if (cl_ui_vol_bar_is_show())
                {
                    cl_ui_vol_bar_hide();
                    return;
                }
                #ifndef SIMULATOR
                extern void toggleChatState();
                toggleChatState();
                #endif
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

void cl_set_emoji(const char *emoji)
{
    if (vw.emoji == NULL) return;
    if (emoji == NULL) {
        lv_vpg_set_src(vw.emoji, NULL);
        return;
    }
    for (int i = 0; i < sizeof(emoji_map) / sizeof(emoji_map[0]); i++)
    {
        if (strcmp(emoji_map[i].emoji, emoji) == 0)
        {
            lv_vpg_set_src(vw.emoji, emoji_map[i].file);
            vw.loop = emoji_map[i].loop;
        }
    }
    // if (vw.float_emote != NULL) {
    //   // 脸上悬浮的小表情贴图
    //   if (strcmp(emoji, "relaxed") == 0) {
    //     lv_image_set_src(vw.float_emote, ASSERT_PREXI "睡觉-1.png");
    //   } else if (strcmp(emoji, "angry") == 0) {
    //     lv_image_set_src(vw.float_emote, ASSERT_PREXI "生气-1.png");
    //   } else if (strcmp(emoji, "surprised") == 0) {
    //     lv_image_set_src(vw.float_emote, ASSERT_PREXI "无语-1.png");
    //   } else if (strcmp(emoji, "shocked") == 0) {
    //     lv_image_set_src(vw.float_emote, ASSERT_PREXI "震惊-1.png");
    //   } else if (strcmp(emoji, "confused") == 0) {
    //     lv_image_set_src(vw.float_emote, ASSERT_PREXI "疑惑-1.png");
    //   } else {
    //     lv_image_set_src(vw.float_emote, NULL);
    //   }
    // }
        if (vw.float_emote != NULL) {
      // 脸上悬浮的小表情贴图
      if (strcmp(emoji, "relaxed") == 0) {
        lv_image_set_src(vw.float_emote, ASSERT_PREXI "睡觉-1.qoi");
      } else if (strcmp(emoji, "angry") == 0) {
        lv_image_set_src(vw.float_emote, ASSERT_PREXI "生气-1.qoi");
      } else if (strcmp(emoji, "surprised") == 0) {
        lv_image_set_src(vw.float_emote, ASSERT_PREXI "无语-1.qoi");
      } else if (strcmp(emoji, "shocked") == 0) {
        lv_image_set_src(vw.float_emote, ASSERT_PREXI "震惊-1.qoi");
      } else if (strcmp(emoji, "confused") == 0) {
        lv_image_set_src(vw.float_emote, ASSERT_PREXI "疑惑-1.qoi");
      } else {
        lv_image_set_src(vw.float_emote, NULL);
      }
    }
}

void cl_set_chat_message(const char *message)
{
    if (vw.chat_message == NULL)
    {
        return;
    }
    
    if (message == NULL){
        lv_label_set_text(vw.chat_message, "");
        return;
    }
    lv_label_set_text(vw.chat_message, message);
}

void cl_set_status(const char *status)
{
    if (strcmp(status, STATE_STANDBY) == 0)
    {   
        vw.standby = true;
    }

    if (vw.status == NULL)
    {
        return;
    }
    ESP_LOGW(TAG, "status:%s", status);
    if (strcmp(status, STATE_LISTENING) == 0)
    {
        lv_label_set_text(vw.status, "");
        if (lv_obj_has_flag(vw.listen, LV_OBJ_FLAG_HIDDEN))
        {
            lv_obj_clear_flag(vw.listen, LV_OBJ_FLAG_HIDDEN);
        }
        lv_vpg_set_img(vw.listen, &icon_mic);
        cl_set_emoji("happy");
        lv_label_set_text(vw.chat_message, "");
    }else if (strcmp(status, STATE_STANDBY) == 0)
    {
        lv_obj_add_flag(vw.listen, LV_OBJ_FLAG_HIDDEN);
        lv_label_set_text(vw.status, "");
    }else if (strcmp(status, STATE_SPEAKING) == 0)
    {    
        ESP_LOGW(TAG, "speaking:%s", status);
        lv_label_set_text(vw.status, "");
        if (lv_obj_has_flag(vw.listen, LV_OBJ_FLAG_HIDDEN))
        {
            lv_obj_clear_flag(vw.listen, LV_OBJ_FLAG_HIDDEN);
        }
        lv_vpg_set_img(vw.listen, &icon_speaker_zzz);
    }else if (strcmp(status, STATE_ERROR) == 0) {
        lv_label_set_text(vw.status, "");
        if (lv_obj_has_flag(vw.listen, LV_OBJ_FLAG_HIDDEN))
        {
            lv_obj_clear_flag(vw.listen, LV_OBJ_FLAG_HIDDEN);
        }
        lv_vpg_set_img(vw.listen, &icon_WiFi_failed);
    }else if (strcmp(status, STATE_CONNECTING) == 0)
    {
        lv_label_set_text(vw.status, "");
        if (lv_obj_has_flag(vw.listen, LV_OBJ_FLAG_HIDDEN))
        {
            lv_obj_clear_flag(vw.listen, LV_OBJ_FLAG_HIDDEN);
        }
        lv_vpg_set_src(vw.listen, ASSERT_PREXI"listen.vpg");
    }
    
    else{    
        lv_label_set_text(vw.status, "");
        lv_obj_add_flag(vw.listen, LV_OBJ_FLAG_HIDDEN);
        lv_toast_show(status, 1000);
    }
}

static void _on_emoji_event(lv_event_t *e)
{
    // uint32_t *is_process = lv_event_get_param(e);
    // lv_vpg_set_src(vw.emoji, NULL);
    // if (vw.loop == false)
    // {
    //     *is_process = 1;
    // }
    
}



// static void _arc_menu_show(bool show){
//     int32_t y = lv_obj_get_y(vw.cont_menu);
//     lv_anim_t anim;
//     lv_anim_init(&anim);
//     lv_anim_set_var(&anim, vw.cont_menu);
//     lv_anim_set_time(&anim, 300); // 动画时长300ms，可根据需求调整
//     lv_anim_set_exec_cb(&anim, (lv_anim_exec_xcb_t)lv_obj_set_y);
//     if (show && y > 0)
//     {
//         lv_anim_set_values(&anim, y, 0);
//         lv_anim_start(&anim);
//     }else if (!show && y < LV_HOR_RES)
//     {
//         lv_anim_set_values(&anim, y, LV_HOR_RES);
//         lv_anim_start(&anim);
//     }
// }


static void _root_ges_cb(lv_event_t *e){
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_GESTURE)
    {
        lv_dir_t dir = lv_indev_get_gesture_dir(lv_indev_active());
        if (dir == LV_DIR_TOP)
        {
            lv_indev_wait_release(lv_indev_active());
            cl_arc_menu_show(1);
        }
    }
}

void _create_chat_cont(lv_obj_t *root){
    vw.emoji = lv_vpg_create(root);
    lv_obj_align(vw.emoji, LV_ALIGN_CENTER, 0, -20);
    cl_set_emoji("neutral");
    lv_obj_add_event_cb(vw.emoji, _on_emoji_event, LV_EVENT_READY, NULL);

    // vw.hair = lv_image_create(root);
    // lv_obj_align(vw.hair, LV_ALIGN_TOP_MID, 0, 0);
    // // lv_image_set_src(vw.hair, ASSERT_PREXI"头发.png");
    // lv_image_set_src(vw.hair, ASSERT_PREXI"头发.qoi");

    vw.float_emote = lv_image_create(root);
    lv_obj_align(vw.float_emote, LV_ALIGN_CENTER, 0, 0);

    vw.chat_message = lv_label_create(root);
    lv_obj_set_style_text_font(vw.chat_message, cl_ui_get_font(), 0);
    lv_label_set_text(vw.chat_message, "");
    lv_obj_set_width(vw.chat_message, 220);
    lv_label_set_long_mode(vw.chat_message, LV_LABEL_LONG_SCROLL_CIRCULAR);
    lv_obj_set_style_text_color(vw.chat_message, lv_color_hex(0xffffff), 0);
    lv_obj_align(vw.chat_message, LV_ALIGN_BOTTOM_MID, 0, -40);
    //文字居中对齐
    lv_obj_set_style_text_align(vw.chat_message, LV_TEXT_ALIGN_CENTER, 0);

    
    vw.status = lv_label_create(root);
    lv_obj_align(vw.status, LV_ALIGN_TOP_MID, 0, 20);
    lv_label_set_text(vw.status, " ");   
    lv_obj_set_style_text_font(vw.status, cl_ui_get_font(), 0);
    lv_obj_set_style_text_color(vw.status, lv_color_hex(0xffffff), 0);
    lv_obj_set_style_text_align(vw.status, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_width(vw.status, 120);
    lv_label_set_long_mode(vw.status, LV_LABEL_LONG_SCROLL);

    vw.listen = lv_vpg_create(root);
    // lv_obj_align(vw.listen, LV_ALIGN_TOP_MID, 0, 10);
    // lv_vpg_set_src(vw.listen, ASSERT_PREXI"listen.vpg");
    if (vw.standby)
    {
        lv_obj_add_flag(vw.listen, LV_OBJ_FLAG_HIDDEN);
    }   
}


static void _create_menu_cont(lv_obj_t *root){
    lv_obj_t *cont_menu = lv_obj_create(root);
    vw.cont_menu = cont_menu;
    lv_obj_set_style_border_width(cont_menu, 0, 0);
    lv_obj_set_size(cont_menu, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_bg_opa(cont_menu, LV_OPA_40, 0);
    lv_obj_set_style_bg_color(cont_menu, lv_color_black(), 0);
    lv_obj_set_pos(cont_menu, 0, LV_VER_RES);

    lv_obj_t *arc_menu = lv_arc_menu_create(cont_menu);
    lv_obj_set_size(arc_menu, 240, 240);
    lv_obj_center(arc_menu);
    // lv_arc_menu_add_btn(arc_menu, &icon_alarm_28, _alarm_click);
    lv_arc_menu_add_btn(arc_menu, &icon_setting_28, NULL);
    // lv_arc_menu_add_btn(arc_menu, &icon_music_28, _music_click);
    // lv_arc_menu_add_btn(arc_menu, &icon_video_28, _video_click);
    // lv_arc_menu_add_btn(arc_menu, &icon_image_28, _img_click);
    lv_arc_menu_add_btn(arc_menu, &icon_countdown_32, NULL);

    // lv_obj_add_event_cb(cont_menu, _menu_event_cb, LV_EVENT_CLICKED, NULL);
}

home_view_t* home_view_create(lv_obj_t *root)
{
    lv_obj_remove_style_all(root);
    lv_obj_set_size(root, LV_HOR_RES, LV_VER_RES);
    lv_obj_set_style_bg_color(root, lv_color_black(),0);
    lv_obj_set_style_bg_opa(root, LV_OPA_COVER, 0);
    lv_obj_clear_flag(root, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_scrollbar_mode(root, LV_SCROLLBAR_MODE_OFF);
    lv_obj_center(root);


    _create_chat_cont(root);
    // _create_menu_cont(root);
   
    lv_obj_add_event_cb(root, _on_btn_cb, CL_UI_EVENT_BUTTON, NULL);
    lv_obj_add_event_cb(root, _root_ges_cb, LV_EVENT_GESTURE, NULL);
    lv_obj_add_event_cb(root, _root_ges_cb, LV_EVENT_PRESSED, NULL);
    lv_obj_clear_flag(root, LV_OBJ_FLAG_GESTURE_BUBBLE);

#ifdef SIMULATOR
    lv_label_set_text(vw.chat_message, "哈哈，这是一条测试消息");
#endif

    return &vw;
}

void home_view_delete(void)
{
    /* 如需释放资源在此处理 */
}
