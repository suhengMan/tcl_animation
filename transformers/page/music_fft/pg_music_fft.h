#ifndef __PG_MUSIC_FFT_H__
#define __PG_MUSIC_FFT_H__

#include "page_manager.h"
#include "vw_music_fft.h"

typedef struct
{
    const char* name;
    page_vtable_t* page_param;
    music_fft_view_t* view;
} music_fft_pg_t;

page_vtable_t* music_fft_create(const char* name);

#endif /* __PG_MUSIC_FFT_H__ */

