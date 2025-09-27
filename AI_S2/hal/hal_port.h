#ifndef _HAL_PORT_H_
#define _HAL_PORT_H_

#include "cl_ui.h"

typedef enum{
    BLUETOOTH_MODE,
    TF_CARD_MODE,
    USB_MODE,
    FLASH_MUSIC_MODE
}music_mode_t;

void xz_hal_set_bl(int val);
u8 xz_hal_get_bl();
void xz_hal_set_vol(int val);
int xz_hal_get_vol();
int xz_hal_get_loop_mode();
void xz_hal_set_loop_mode(int mode);

u8 xz_hal_music_status();
const char* xz_hal_get_music_name();
void xz_hal_music_play();
void xz_hal_music_stop();
void xz_hal_music_togle_play();
void xz_hal_music_priv();
void xz_hal_music_next();
void xz_hal_music_ff(int time);//快进
void xz_hal_music_fr(int time);//快退
void xz_hal_music_set_mode(music_mode_t mode);
music_mode_t xz_hal_music_get_mode();

void xz_recover_fact();
void xz_enable_global_wake(u8 enable);
u8 xz_is_global_wake_enable();

u8 xz_get_ai_theme();
void xz_set_ai_theme(u8 index);
const char* xz_get_devid();
#endif