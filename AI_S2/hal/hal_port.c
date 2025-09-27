#include "cl_ui.h"

static int _map(int in, int in_min, int in_max, int out_min, int out_max) {
    return (in_max == in_min) ? 0 : (in - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void xz_hal_set_bl(int val){
#ifndef SIMULATOR
    lcd_bl_set_backlight(val);
#endif
}

u8 xz_hal_get_bl(){
#ifndef SIMULATOR
    return xiaozhi_board_get_bl();
#endif
}

void xz_hal_set_vol(int val){
#ifndef SIMULATOR
    app_tone_set_volume(val);
#endif
}

int xz_hal_get_vol(){
#ifndef SIMULATOR
    int vol = app_audio_get_volume(APP_AUDIO_STATE_MUSIC);
    return _map(vol, 0, 30, 0, 100);
#else
    return 80;
#endif
}

int xz_hal_get_loop_mode(){
#ifndef SIMULATOR
    return music_player_get_repeat_mode();
#else
    return 0;
#endif
}

void xz_hal_set_loop_mode(int mode){
#ifndef SIMULATOR
    return music_player_set_repeat_mode(mode);
#else
    return 0;
#endif
}


const char* xz_hal_get_music_name(){
#ifndef SIMULATOR
    switch (query_mode()) {
        case BLUETOOTH_MODE:
            return "todo";
        break;
        case TF_CARD_MODE:
        case USB_MODE:
        case FLASH_MUSIC_MODE:
            return music_file_get_cur_name_utf8();
        break;
        default:
            return NULL;
        break;
    }
#else
    return NULL;
#endif
}

u8 xz_hal_music_status(){
#ifndef SIMULATOR
    switch (query_mode()) {
        case BLUETOOTH_MODE:
            return bt_a2dp_status_check()==1;
        break;
        case TF_CARD_MODE:
        case USB_MODE:
        case FLASH_MUSIC_MODE:
            return music_player_get_play_status()==1;
        break;
        default:
            return 0;
        break;
    }
#else
    return 0;
#endif
}

void xz_hal_music_togle_play(){
#ifndef SIMULATOR
    switch (query_mode()) {
        case BLUETOOTH_MODE:
            unsigned char music_states = a2dp_get_status();
            if (!bt_a2dp_status_check()) { // 如果音乐暂停或停止
                user_send_cmd_prepare(USER_CTRL_AVCTP_OPID_PLAY, 0, NULL); // 播放音乐
            } else {
                user_send_cmd_prepare(USER_CTRL_AVCTP_OPID_PAUSE, 0, NULL); // 播放音乐
            }
            break;
        case TF_CARD_MODE:
        case USB_MODE:
        case FLASH_MUSIC_MODE:
            app_task_put_key_msg(KEY_MUSIC_PP, 0); // 播放或暂停音乐
            break;
    }
#endif
}

void xz_hal_music_stop(){

}

void xz_hal_music_priv(){

#ifndef SIMULATOR
    switch (query_mode()) {
        case BLUETOOTH_MODE: 
            user_send_cmd_prepare(USER_CTRL_AVCTP_OPID_PREV, 0, NULL); 
        break; // 播放上一首
        case TF_CARD_MODE:
        case USB_MODE:
        case FLASH_MUSIC_MODE: 
            app_task_put_key_msg(KEY_MUSIC_PREV, 0); 
        break; // TF卡或U盘播放上一首
    }
#endif
}
void xz_hal_music_next(){
#ifndef SIMULATOR
    switch (query_mode()) {
        case BLUETOOTH_MODE: 
            user_send_cmd_prepare(USER_CTRL_AVCTP_OPID_NEXT, 0, NULL); 
        break; // 播放下一首
        case TF_CARD_MODE:
        case USB_MODE:
        case FLASH_MUSIC_MODE: 
            app_task_put_key_msg(KEY_MUSIC_NEXT, 0); 
        break; // TF卡或U盘播放下一首
    }
#endif
}


void xz_hal_music_ff(int time){
#ifndef SIMULATOR
    if (query_mode() == USB_MODE || query_mode() == TF_CARD_MODE) {
        music_player_ff(time/1000);
    }
#endif
}


void xz_hal_music_fr(int time){
#ifndef SIMULATOR
if (query_mode() == USB_MODE || query_mode() == TF_CARD_MODE) {
        music_player_fr(time/1000);
    }
#endif
}


music_mode_t xz_hal_music_get_mode(){
#ifndef SIMULATOR
    return query_mode();
#endif
    return TF_CARD_MODE;
}

void xz_hal_music_set_mode(music_mode_t mode){
#ifndef SIMULATOR
    switch (mode)
    {
    case BLUETOOTH_MODE:
            if (query_mode() != BLUETOOTH_MODE) app_kws_music_mode_set(IDEX_TONE_NUM_BT_MODE);
        break;
    case TF_CARD_MODE:
        if (query_mode() != TF_CARD_MODE) app_kws_music_mode_set(IDEX_TONE_NUM_TF_MODE);
        break;
    case USB_MODE:
        
        break;
    
    default:
        break;
    }
#endif  
}

void xz_del_bt(){
#ifndef SIMULATOR
    if(get_bt_connect_status() != BT_STATUS_WAITINT_CONN ){
        user_send_cmd_prepare(USER_CTRL_DISCONNECTION_HCI, 0, NULL);
        user_send_cmd_prepare(USER_CTRL_DEL_ALL_REMOTE_INFO, 0, NULL);
    }else{
        user_send_cmd_prepare(USER_CTRL_DEL_ALL_REMOTE_INFO, 0, NULL);
    }
#endif
}

void xz_recover_fact(){
#ifndef SIMULATOR
    xiaozhi_board_reset_factory();
    xz_del_bt();
    cpu_reset();
#endif
}

void xz_enable_global_wake(u8 enable){
#ifndef SIMULATOR
    if (enable)
    {
        xiaozhi_board_set_global_wake(1);
    }else{
        xiaozhi_board_set_global_wake(0);
    }
#endif
}

u8 xz_is_global_wake_enable(){
#ifndef SIMULATOR
    xiaozhi_board_get_global_wake();
#else
    return 0;
#endif
}

u8 xz_get_ai_theme(){
#ifndef SIMULATOR
    return board_get_theme();
#else
    return 0;
#endif
}
void xz_set_ai_theme(u8 index){
#ifndef SIMULATOR
    board_set_theme(index);
#endif
}

const char* xz_get_devid(){
#ifndef SIMULATOR
    return xiaozhi_board_get_devid(index);
#else
    return "00229933441100883377";
#endif

}
