#ifndef __PG_APP_LIST_H__
#define __PG_APP_LIST_H__

#include "page_manager.h"
#include "vw_app_list.h"

typedef struct
{
    const char* name;
    page_vtable_t* page_param;
    app_list_view_t* view;
} app_list_pg_t;

page_vtable_t* app_list_create(const char* name);

#endif /* __PG_APP_LIST_H__ */
