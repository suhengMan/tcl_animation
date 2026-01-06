#ifndef __PG_SETTING_DEV_H__
#define __PG_SETTING_DEV_H__

#include "page_manager.h"
#include "vw_setting_dev.h"

typedef struct
{
    const char* name;
    page_vtable_t* page_param;
    setting_dev_view_t* view;
} setting_dev_pg_t;

page_vtable_t* setting_dev_create(const char* name);

#endif /* __PG_SETTING_DEV_H__ */
