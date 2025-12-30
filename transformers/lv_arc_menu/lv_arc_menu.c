#ifndef SIMULATOR
#include "lvgl.h"
#include "misc/lv_timer_private.h"
#include "core/lv_obj_class_private.h"
#include "widgets/arc/lv_arc_private.h"
#include "math.h"
#else
#include "lvgl.h"
#include "src/misc/lv_timer_private.h"
#include "src/core/lv_obj_class_private.h"
#include "src/widgets/arc/lv_arc_private.h"
#endif
/*********************
 *      DEFINES
 *********************/
#define MY_CLASS (&lv_arc_menu_class)

/**********************
 *      TYPEDEFS
 **********************/

typedef struct{
    lv_obj_t *btn;
    int32_t angle;
    lv_point_t pos;
}lv_arc_menu_btn_t;

typedef struct {
    lv_arc_t arc;
    int32_t count;
    int32_t press_index;
    int32_t angle_offset;
    lv_arc_menu_btn_t btn_list[10];
} lv_arc_menu_t;
 /**********************
 *  STATIC PROTOTYPES
 **********************/
static void calculate_btn_angle(lv_arc_menu_t * arc_menu, int32_t count);
static void calculate_btn_pos(lv_arc_menu_t * arc_menu, int32_t index);
static void calculate_angle_range(lv_arc_menu_t * arc_menu, int32_t angle, int32_t * start_angle, int32_t * end_angle);
static void lv_arc_menu_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj);
static void lv_arc_menu_destructor(const lv_obj_class_t * class_p, lv_obj_t * obj);
static void lv_arc_menu_event(const lv_obj_class_t * class_p, lv_event_t * e);
static void refresh_btn_pos(lv_arc_menu_t * arc_menu);

 /**********************
 *  STATIC VARIABLES
 **********************/

const lv_obj_class_t lv_arc_menu_class = {
    .constructor_cb = lv_arc_menu_constructor,
    .destructor_cb = lv_arc_menu_destructor,
    .event_cb = lv_arc_menu_event,
    .instance_size = sizeof(lv_arc_menu_t),
    .base_class = &lv_arc_class,
    // .name = "arc_menu",
};


lv_obj_t *lv_arc_menu_create(lv_obj_t * parent)
{
    lv_obj_t * obj = lv_obj_class_create_obj(MY_CLASS, parent);
    lv_obj_class_init_obj(obj);
    return obj;
}

void lv_arc_menu_set_rotate(lv_obj_t *obj, int32_t angles){
    lv_arc_menu_t *arc = (lv_arc_menu_t*)obj;
    arc->angle_offset = angles;
    refresh_btn_pos(arc);
}

lv_obj_t *lv_arc_menu_add_btn(lv_obj_t * obj, void * image, lv_event_cb_t event_cb){
    lv_arc_menu_t * arc_menu = (lv_arc_menu_t *) obj;
    arc_menu->btn_list[arc_menu->count].btn = lv_btn_create(obj);
    lv_obj_add_flag(arc_menu->btn_list[arc_menu->count].btn, LV_OBJ_FLAG_EVENT_BUBBLE);
    lv_obj_set_style_bg_opa(arc_menu->btn_list[arc_menu->count].btn, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_shadow_width(arc_menu->btn_list[arc_menu->count].btn, 0, LV_PART_MAIN);
    int32_t width = lv_obj_get_style_arc_width(obj, LV_PART_MAIN);
    lv_obj_set_size(arc_menu->btn_list[arc_menu->count].btn, width, width);
    if (image != NULL)
    {
        lv_obj_t *img = lv_img_create(arc_menu->btn_list[arc_menu->count].btn);
        lv_img_set_src(img, image);
        lv_obj_center(img);
    }
    if (event_cb)
    {
        lv_obj_add_event_cb(arc_menu->btn_list[arc_menu->count].btn, event_cb, LV_EVENT_ALL, NULL);
    }            
    arc_menu->count++;
    refresh_btn_pos(arc_menu);
    return arc_menu->btn_list[arc_menu->count-1].btn;
    
}


static void lv_arc_menu_constructor(const lv_obj_class_t * class_p, lv_obj_t * obj)
{
    LV_UNUSED(class_p);
    lv_arc_menu_t * arc_menu = (lv_arc_menu_t *) obj;
    arc_menu->count = 0;
    arc_menu->press_index = -1;

    lv_arc_set_bg_angles(obj, 0, 360);   // 整圈
    lv_arc_set_angles(obj, 0, 0);   // 整圈
    
    lv_obj_add_flag(obj, LV_OBJ_FLAG_EVENT_BUBBLE);
    lv_obj_set_style_arc_width(obj, 60, LV_PART_MAIN); // 底条宽度
    lv_obj_set_style_arc_color(obj, lv_color_hex(0x333333), LV_PART_MAIN); // 底色
    lv_obj_set_style_arc_width(obj, 60, LV_PART_INDICATOR); // 前景宽度
    lv_obj_set_style_arc_color(obj, lv_color_hex(0x00a6dd), LV_PART_INDICATOR); // 前景色
    lv_obj_remove_style(obj, NULL, LV_PART_KNOB);
}

static void lv_arc_menu_destructor(const lv_obj_class_t * class_p, lv_obj_t * obj)
{
    LV_UNUSED(class_p);

}

static void refresh_btn_pos(lv_arc_menu_t * arc_menu){
    calculate_btn_angle(arc_menu, arc_menu->count);
    for (int32_t i = 0; i < arc_menu->count; i++)
    {
        calculate_btn_pos(arc_menu, i);
        if (arc_menu->btn_list[i].btn != NULL)
        {
            lv_obj_align(arc_menu->btn_list[i].btn, LV_ALIGN_CENTER, arc_menu->btn_list[i].pos.x, arc_menu->btn_list[i].pos.y);
        }
    }
}

static void calculate_btn_angle(lv_arc_menu_t * arc_menu, int32_t count){
    if (count == 0)
    {
        return;
    }
    
    int32_t angle_range = 360 / count;
    
    for (int32_t i = 0; i < count; i++)
    {
        arc_menu->btn_list[i].angle = (i * angle_range + 360 + arc_menu->angle_offset)%360;
    }
}

static void get_center(const lv_obj_t * obj, lv_point_t * center, int32_t * arc_r)
{
    int32_t left_bg = lv_obj_get_style_pad_left(obj, LV_PART_MAIN);
    int32_t right_bg = lv_obj_get_style_pad_right(obj, LV_PART_MAIN);
    int32_t top_bg = lv_obj_get_style_pad_top(obj, LV_PART_MAIN);
    int32_t bottom_bg = lv_obj_get_style_pad_bottom(obj, LV_PART_MAIN);

    int32_t r = (LV_MIN(lv_obj_get_width(obj) - left_bg - right_bg,
                        lv_obj_get_height(obj) - top_bg - bottom_bg)) / 2;

    center->x = obj->coords.x1 + r + left_bg;
    center->y = obj->coords.y1 + r + top_bg;

    if(arc_r) *arc_r = r;
}

static void calculate_btn_pos(lv_arc_menu_t * arc_menu, int32_t index){
    lv_point_t center;
    int32_t r;
    get_center((lv_obj_t*)&arc_menu->arc, &center, &r);
    int32_t arc_width = (int32_t)lv_obj_get_style_arc_width((lv_obj_t*)&arc_menu->arc, LV_PART_MAIN);
    r = r - arc_width / 2;

    // 根据角度和半径计算按钮中心坐标
    // 取 angle, r, center, 得到 pos.x, pos.y
    int32_t angle = arc_menu->btn_list[index].angle;
    // 将角度转换为弧度（LVGL的0度是3点钟方向，正方向为顺时针）
    float rad = (angle) * (3.1415926f / 180.0f); // -90修正让0度指向12点钟方向
    arc_menu->btn_list[index].pos.x = (int32_t)(r * cosf(rad));
    arc_menu->btn_list[index].pos.y = (int32_t)(r * sinf(rad));

    // printf("btn%d angle: %d, pos(rel to arc): %d, %d\n", index, angle, arc_menu->btn_list[index].pos.x, arc_menu->btn_list[index].pos.y);
}

static void calculate_angle_range(lv_arc_menu_t * arc_menu, int32_t angle, int32_t * start_angle, int32_t * end_angle){
    int32_t angle_range = 360 / arc_menu->count;
    *start_angle = (angle - angle_range / 2 + 360) % 360;
    *end_angle = (angle + angle_range / 2 + 360) % 360;
}

static bool is_in_angle(int32_t angle, int32_t target_angle, int32_t count){
    int32_t angle_range = 360 / count;
    int32_t start_angle = (target_angle - angle_range / 2 + 360) % 360;
    int32_t end_angle = (target_angle + angle_range / 2 + 360) % 360;
    if(start_angle <= end_angle) {
        return (angle >= start_angle && angle <= end_angle);
    }
    else {
        // 跨越0度的区间，比如 350~10
        return (angle >= start_angle || angle <= end_angle);
    }
}

static void lv_arc_menu_event(const lv_obj_class_t * class_p, lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    if (code == LV_EVENT_PRESSED)
    {
        lv_obj_t * obj = lv_event_get_current_target(e);
        lv_arc_menu_t * arc_menu = (lv_arc_menu_t *) obj;
        lv_indev_t * indev = lv_indev_active();
        if(indev == NULL) return;

        lv_point_t p;
        lv_indev_get_point(indev, &p);

        /* 获取中心点和半径 */
        lv_point_t center;
        int32_t r;
        get_center(obj, &center, &r);

        /* 计算点击点到中心的距离 */
        int32_t dx = p.x - center.x;
        int32_t dy = p.y - center.y;
        int32_t dist_sq = dx * dx + dy * dy;
        int32_t r_out = r; // 外圈半径
        int32_t r_in = r - lv_obj_get_style_arc_width(obj, LV_PART_MAIN); // 内圈半径

        if(dist_sq < r_in*r_in || dist_sq > r_out*r_out) {
            // printf("未点在圈上\n");
            arc_menu->press_index = -1;        
           lv_obj_add_flag(obj, LV_OBJ_FLAG_EVENT_BUBBLE);
        } else {
            /* 计算点击角度（以正东为0度，逆时针为正），用整数 */
            lv_obj_remove_flag(obj, LV_OBJ_FLAG_EVENT_BUBBLE);
            lv_event_stop_bubbling(e);
            int32_t angle = (int32_t)(atan2f(dy, dx) * 180.0f / 3.1415926f);
            if(angle < 0) angle += 360;
            for (int32_t i = 0; i < arc_menu->count; i++)
            {
                if(is_in_angle(angle, arc_menu->btn_list[i].angle, arc_menu->count)) {
                    // printf("点在按钮%d上\n", i);
                    arc_menu->press_index = i;
                    int32_t start_angle, end_angle;
                    calculate_angle_range(arc_menu, arc_menu->btn_list[i].angle, &start_angle, &end_angle);
                    lv_obj_set_style_arc_opa(obj, LV_OPA_80, LV_PART_INDICATOR);
                    lv_arc_set_angles(obj, start_angle, end_angle);
                    // printf("点在按钮%d上,角度: %ld\n", i, angle);
                    break;
                }
            }
            
            
        }
    }else if (code == LV_EVENT_RELEASED || code == LV_EVENT_PRESS_LOST)
    {
        lv_obj_t * obj = lv_event_get_current_target(e);
        lv_arc_menu_t * arc_menu = (lv_arc_menu_t *) obj;
        lv_obj_set_style_arc_opa(obj, LV_OPA_COVER, LV_PART_INDICATOR);
    }
    
    else if (code == LV_EVENT_CLICKED || code == LV_EVENT_PRESSING)
    {
    }
    else if (code == LV_EVENT_SIZE_CHANGED)
    {    
        lv_obj_t * obj = lv_event_get_current_target(e);
        refresh_btn_pos((lv_arc_menu_t *)obj);
    }
    
    else{
        lv_arc_class.event_cb(class_p, e);
    }
    
}