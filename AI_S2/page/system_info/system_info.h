#ifndef __SYSTEM_INFO_H__
#define __SYSTEM_INFO_H__

#include "page_manager.h"
#include "system_info_view.h"

typedef struct 
{
    const char* name;
    page_vtable_t* page_param;
    system_info_view_t* system_info_view;
} system_info_t;

page_vtable_t* system_info_create(const char* name);

#endif 