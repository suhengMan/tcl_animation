#ifndef __LABORATORY_H__
#define __LABORATORY_H__

#include "page_manager.h"
#include "laboratory_view.h"


typedef struct 
{
    const char* name;
    page_vtable_t* page_param;
    // laboratory_model_t* laboratory_model;
    laboratory_view_t* laboratory_view;
}laboratory_t;

page_vtable_t* laboratory_create(const char* name);

#endif