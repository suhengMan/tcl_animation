#ifndef __PG_PHONE_CALL_H__
#define __PG_PHONE_CALL_H__

#include "page_manager.h"
#include "vw_phone_call.h"

typedef struct
{
    const char* name;
    page_vtable_t* page_param;
    phone_call_view_t* view;
} phone_call_pg_t;

page_vtable_t* phone_call_create(const char* name);

#endif /* __PG_PHONE_CALL_H__ */
