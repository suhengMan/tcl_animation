#ifndef __SETTING_THEME_H__
#define __SETTING_THEME_H__

#include "page_manager.h"
#include "setting_theme_view.h"

typedef struct 
{
    const char* name;
    page_vtable_t* page_param;
    struct setting_theme_view_t* setting_theme_view;
} setting_theme_t;

page_vtable_t* setting_theme_create(const char* name);

#endif 