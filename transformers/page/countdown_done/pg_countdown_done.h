#ifndef __PG_COUNTDOWN_DONE_H__
#define __PG_COUNTDOWN_DONE_H__

#include "page_manager.h"
#include "vw_countdown_done.h"

typedef struct
{
    const char* name;
    page_vtable_t* page_param;
    countdown_done_view_t* view;
} countdown_done_pg_t;

page_vtable_t* countdown_done_create(const char* name);

#endif /* __PG_COUNTDOWN_DONE_H__ */
