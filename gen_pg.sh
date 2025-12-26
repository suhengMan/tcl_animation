#!/usr/bin/env bash
set -euo pipefail

if [[ $# -lt 1 || $# -gt 2 ]]; then
  echo "Usage: $0 <name> [outdir]"
  exit 1
fi

raw="$1"
# 规范化文件命名：小写做文件名，大写做 include guard
name="$(echo "$raw" | tr '[:upper:]' '[:lower:]')"
NAME="$(echo "$name" | tr '[:lower:]' '[:upper:]')"
outdir="${2:-$name}"

mkdir -p "$outdir"

# ---------- pg_{name}.h ----------
cat > "${outdir}/pg_${name}.h" <<EOF
#ifndef __PG_${NAME}_H__
#define __PG_${NAME}_H__

#include "page_manager.h"
#include "vw_${name}.h"

typedef struct
{
    const char* name;
    page_vtable_t* page_param;
    ${name}_view_t* view;
} ${name}_pg_t;

page_vtable_t* ${name}_create(const char* name);

#endif /* __PG_${NAME}_H__ */
EOF

# ---------- pg_{name}.c ----------
cat > "${outdir}/pg_${name}.c" <<EOF
#include "lvgl.h"
#include "cl_ui.h"
#include "pg_${name}.h"
#include "vw_${name}.h"

static ${name}_pg_t pg;

static void on_custom_attr_config(page_base_t *self)
{
    page_set_custom_cache_enable(self, false);
    page_set_custom_load_anim_type(self, LOAD_ANIM_NONE, 100, lv_anim_path_ease_out);
}

/* Page load */
static void on_view_load(page_base_t *self)
{
    // ${name}_model_init(); // 如有模型初始化可放开
    pg.view = ${name}_view_create(self->root);
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
    ${name}_view_delete();
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

page_vtable_t* ${name}_create(const char *name)
{
    pg.page_param = &page_param;
    pg.name = name;
    return pg.page_param;
}

REGIST_PAGE(${name})
EOF

# ---------- vw_{name}.h ----------
cat > "${outdir}/vw_${name}.h" <<EOF
#ifndef __${NAME}_VIEW_H__
#define __${NAME}_VIEW_H__

#include "lvgl.h"

typedef struct
{
    uint8_t is_act; /* 如果没有 u8，改成 uint8_t 并 #include <stdint.h> */
} ${name}_view_t;

${name}_view_t* ${name}_view_create(lv_obj_t *root);
void ${name}_view_delete(void);

#endif /* __${NAME}_VIEW_H__ */
EOF

# ---------- vw_{name}.c ----------
cat > "${outdir}/vw_${name}.c" <<EOF
#include "lvgl.h"
#include "cl_ui.h"
#include "vw_${name}.h"

static ${name}_view_t vw;

#define TAG "vw_${name}"

static void _on_btn_cb(lv_event_t *e)
{
    if (!vw.is_act) return;

    cl_button_t *btn = (cl_button_t*)lv_event_get_param(e);
    if (!btn) return;

    switch (btn->id)
    {
        case CL_UI_KEY_POWER:
            if (btn->event == CL_BTN_CLICK)
            {
            }
            break;
        default:
            break;
    }
}

${name}_view_t* ${name}_view_create(lv_obj_t *root)
{
    lv_obj_remove_style_all(root);
    lv_obj_set_size(root, LV_HOR_RES, LV_VER_RES);

    lv_obj_add_event_cb(root, _on_btn_cb, CL_UI_EVENT_BUTTON, NULL);

    return &vw;
}

void ${name}_view_delete(void)
{
    /* 如需释放资源在此处理 */
}
EOF

echo "✔ Generated page template in: ${outdir}"
echo "   - ${outdir}/pg_${name}.h"
echo "   - ${outdir}/pg_${name}.c"
echo "   - ${outdir}/vw_${name}.h"
echo "   - ${outdir}/vw_${name}.c"
