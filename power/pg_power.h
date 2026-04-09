#ifndef __PG_POWER_H__
#define __PG_POWER_H__

#include "page_manager.h"
#include "vw_power.h"

typedef struct
{
    const char* name;
    page_vtable_t* page_param;
    power_view_t* view;
} power_pg_t;

page_vtable_t* power_create(const char* name);

#endif /* __PG_POWER_H__ */
