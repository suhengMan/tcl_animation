#ifndef __PG_ALARM_RING_H__
#define __PG_ALARM_RING_H__

#include "page_manager.h"
#include "vw_alarm_ring.h"

typedef struct
{
    const char* name;
    page_vtable_t* page_param;
    alarm_ring_view_t* view;
} alarm_ring_pg_t;

page_vtable_t* alarm_ring_create(const char* name);

#endif /* __PG_ALARM_RING_H__ */
