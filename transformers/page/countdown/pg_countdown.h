#ifndef __PG_COUNTDOWN_H__
#define __PG_COUNTDOWN_H__

#include "page_manager.h"
#include "vw_countdown.h"

typedef struct
{
    const char* name;
    page_vtable_t* page_param;
    countdown_view_t* view;
} countdown_pg_t;

page_vtable_t* countdown_create(const char* name);

#endif /* __PG_COUNTDOWN_H__ */
