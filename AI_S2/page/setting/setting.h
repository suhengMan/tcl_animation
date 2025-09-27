#ifndef __SETTING_H__
#define __SETTING_H__

#include "page_manager.h"
#include "setting_view.h"

typedef struct 
{
    const char* name;
    page_vtable_t* page_param;
    setting_view_t* setting_view;
} setting_t;

page_vtable_t* setting_create(const char* name);

#endif 