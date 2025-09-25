#include "cl_ui.h"
#include "lvgl.h"


#define TAG "CL_UI"

static page_manager_t *g_page_manager = NULL;
extern page_handle_t __start_cl_ui_page[];
extern page_handle_t __stop_cl_ui_page[];


LV_FONT_DECLARE(font_puhui_18_4)
static lv_style_t s_global_font_style;

static void _page_install(){
    g_page_manager = page_manager_create();
    
    for (u8* t = __start_cl_ui_page; t < __stop_cl_ui_page; t+=32)
    {
        page_handle_t *p = (page_handle_t*)t;
        // LOGD("%s %p", t->name, t);
        pm_install(g_page_manager, p->name, p->create(p->name));
    }
    
    pm_push(g_page_manager, "app_list", NULL);
}

int page_change(const char* name){
    
    page_base_t *page = pm_find_page(g_page_manager, name);
    if (page == NULL) {
        LOGE("page_change: 页面未找到");
        return -1;
    }
    LOGD("切换页面:%s", name);
    pm_pop(g_page_manager);
    pm_push(g_page_manager, name, NULL);
    return 1;
}

void ui_init(){
    lv_style_init(&s_global_font_style);
    lv_style_set_text_font(&s_global_font_style, &font_puhui_18_4);
    lv_obj_add_style(lv_screen_active(), &s_global_font_style, LV_PART_MAIN);
    _page_install();
}