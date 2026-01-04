#ifndef __PG_SETTING_H__
#define __PG_SETTING_H__

#include "page_manager.h"
#include "vw_setting.h"

typedef struct
{
    const char* name;
    page_vtable_t* page_param;
    setting_view_t* view;
} setting_pg_t;

page_vtable_t* setting_create(const char* name);

#endif /* __PG_SETTING_H__ */
