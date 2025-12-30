#ifndef __PG_ANIM_H__
#define __PG_ANIM_H__

#include "page_manager.h"
#include "vw_anim.h"

typedef struct
{
    const char* name;
    page_vtable_t* page_param;
    anim_view_t* view;
} anim_pg_t;

page_vtable_t* anim_create(const char* name);

#endif /* __PG_ANIM_H__ */
