#ifndef __PG_CHAT_EYE_H__
#define __PG_CHAT_EYE_H__

#include "page_manager.h"
#include "vw_chat_eye.h"

typedef struct
{
    const char* name;
    page_vtable_t* page_param;
    chat_eye_view_t* view;
} chat_eye_pg_t;

page_vtable_t* chat_eye_create(const char* name);

#endif /* __PG_CHAT_EYE_H__ */
