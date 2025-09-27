 #ifndef __HELP_H__
#define __HELP_H__

#include "page_manager.h"
#include "help_view.h"

typedef struct 
{
    const char* name;
    page_vtable_t* page_param;
    help_view_t* help_view;
} help_t;

page_vtable_t* help_create(const char* name);

#endif