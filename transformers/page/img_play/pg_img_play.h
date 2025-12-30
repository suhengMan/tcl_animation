#ifndef __PG_IMG_PLAY_H__
#define __PG_IMG_PLAY_H__

#include "page_manager.h"
#include "vw_img_play.h"

typedef struct
{
    const char* name;
    page_vtable_t* page_param;
    img_play_view_t* view;
} img_play_pg_t;

page_vtable_t* img_play_create(const char* name);

#endif /* __PG_IMG_PLAY_H__ */
