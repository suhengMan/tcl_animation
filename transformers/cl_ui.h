#ifndef _CL_UI_H_
#define _CL_UI_H_

#include "page_manager.h"
#include "cl_ui.h"
#ifdef __cplusplus
extern "C"
{
#endif

typedef enum{
    CL_UI_EVENT_MUSIC_STATUS = LV_EVENT_LAST + 1,
    CL_UI_EVENT_MODE_CHANGE,
    CL_UI_EVENT_BT_STATUS,
    CL_UI_EVENT_MUSIC_TITLE,
    CL_UI_EVENT_MUSIC_LYRC,
    CL_UI_EVENT_MUSIC_TIME,
}cl_ui_event_t;

void ui_init();
int page_change(const char* name);
lv_obj_t *ui_get_home();

#ifdef __cplusplus
}
#endif

#endif