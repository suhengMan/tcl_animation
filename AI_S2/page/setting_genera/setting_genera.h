#ifndef __setting_genera_H__
#define __setting_genera_H__

#include "page_manager.h"
#include "setting_genera_view.h"

typedef struct 
{
    const char* name;
    page_vtable_t* page_param;
    setting_genera_view_t* setting_genera_view;
} setting_genera_t;

page_vtable_t* setting_genera_create(const char* name);

#endif 