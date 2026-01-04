#ifndef __PG_ALARM_H__
#define __PG_ALARM_H__

#include "page_manager.h"
#include "vw_alarm.h"

typedef struct
{
    const char* name;
    page_vtable_t* page_param;
    alarm_view_t* view;
} alarm_pg_t;

page_vtable_t* alarm_create(const char* name);

#endif /* __PG_ALARM_H__ */
