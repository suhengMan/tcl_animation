#include "lvgl.h"
#include "cl_ui.h"
#include "pg_countdown.h"
#include "vw_countdown.h"

static countdown_pg_t pg;

static void on_custom_attr_config(page_base_t *self)
{
    page_set_custom_cache_enable(self, false);
    page_set_custom_load_anim_type(self, LOAD_ANIM_NONE, 100, lv_anim_path_ease_out);
}

/* Page load */
static void on_view_load(page_base_t *self)
{
    // countdown_model_init(); // 如有模型初始化可放开
    pg.view = countdown_view_create(self->root);
    uint32_t time_ms = 0;
    if (page_get_stash(self, &time_ms, sizeof(uint32_t)))
    {
        start_countdown(time_ms);
    }
    
}

/* Page load complete */
static void on_view_did_load(page_base_t *self)
{
}

/* Page will be displayed soon  */
static void on_view_will_appear(page_base_t *self)
{
}

/* The page is displayed  */
static void on_view_did_appear(page_base_t *self)
{
    pg.view->is_act = 1;
}

/* Page is about to disappear */
static void on_view_will_disappear(page_base_t *self)
{
    pg.view->is_act = 0;
}

/* Page disappeared complete  */
static void on_view_did_disappear(page_base_t *self)
{
}

/* Page uninstall complete  */
static void on_view_did_unload(page_base_t *self)
{
    countdown_view_delete();
}

static page_vtable_t page_param = {
    .on_custom_attr_config = on_custom_attr_config,
    .on_view_load          = on_view_load,
    .on_view_did_load      = on_view_did_load,
    .on_view_will_appear   = on_view_will_appear,
    .on_view_did_appear    = on_view_did_appear,
    .on_view_will_disappear= on_view_will_disappear,
    .on_view_did_disappear = on_view_did_disappear,
    .on_view_did_unload    = on_view_did_unload,
};

page_vtable_t* countdown_create(const char *name)
{
    pg.page_param = &page_param;
    pg.name = name;
    return pg.page_param;
}

REGIST_PAGE(countdown)
