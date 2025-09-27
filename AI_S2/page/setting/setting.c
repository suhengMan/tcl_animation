#include "lvgl.h"
#include "cl_ui.h"
#include "setting.h"
#include "setting_view.h"

static setting_t pg;

static void on_custom_attr_config(page_base_t *self)
{
    page_set_custom_cache_enable(self, false);
    page_set_custom_load_anim_type(self, LOAD_ANIM_NONE, 300, lv_anim_path_ease_out);
}

static void on_view_load(page_base_t *self)
{   
    pg.setting_view = setting_view_create(self->root);
}   

static void on_view_did_load(page_base_t *self)
{
}

static void on_view_will_appear(page_base_t *self) {}
static void on_view_did_appear(page_base_t *self) {
    pg.setting_view->is_act = 1;
}
static void on_view_will_disappear(page_base_t *self) { pg.setting_view->is_act = 0; }
static void on_view_did_disappear(page_base_t *self) {}
static void on_view_did_unload(page_base_t *self) { setting_view_delete(); }

static page_vtable_t page_param = {
    .on_custom_attr_config = on_custom_attr_config,
    .on_view_load = on_view_load,
    .on_view_did_load = on_view_did_load,
    .on_view_will_appear = on_view_will_appear,
    .on_view_did_appear = on_view_did_appear,
    .on_view_will_disappear = on_view_will_disappear,
    .on_view_did_disappear = on_view_did_disappear,
    .on_view_did_unload = on_view_did_unload,
};

page_vtable_t* setting_create(const char *name)
{
    pg.page_param = &page_param;
    pg.name = name;
    return pg.page_param;
}

REGIST_PAGE(setting) 