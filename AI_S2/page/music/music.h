#ifndef MUSIC_H
#define MUSIC_H
#include "page_manager.h"
#include "music_view.h"
typedef struct { 
    page_base_t base;
    const char *name; 
    music_view_t *vw;
    page_vtable_t *page_param; 
}music_t;
page_vtable_t*music_create(const char *name);
#endif 