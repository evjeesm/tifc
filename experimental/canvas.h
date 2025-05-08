#ifndef _CANVAS_H_
#define _CANVAS_H_

#include "input.h"
#include "display.h"
#include "sparse.h"

typedef struct vec2
{
    long long x;
    long long y;
}
vec2_t;

typedef struct transform_comp
{
    vec2_t location;
    vec2_t delta;
}
transform_comp_t;

typedef struct components
{
    sparse_t *transform; // transform_comp_t
    sparse_t *box;       // vec2_t - width and heigh
    sparse_t *behavior;  // size_t

    sparse_t *data;      // contains pointer to a data blob
}
components_t;

typedef struct behaviors
{
    size_t last_id;
    sparse_t *hover;
    sparse_t *press;
    sparse_t *release;
    sparse_t *drag_begin;
    sparse_t *drag;
    sparse_t *drag_end;
    sparse_t *scroll;
    sparse_t *render;
} 
behaviors_t;

typedef struct entities
{
    size_t camera_id;
    dynarr_t *frames;    // size_t
}
entities;

typedef enum canvas_mode
{
    NORMAL_MODE,  /* just scrolling around */
    COMMAND_MODE, /* enter command */
    RESIZE_MODE,  /* resize frames */
    EDIT_MODE     /* edit selected entity */
}
canvas_mode_t;

typedef struct canvas
{
    components_t components;
    input_hooks_t hooks;
    behaviors_t behaviors;

    entities ents;
    canvas_mode_t mode;
}
canvas_t;

typedef void (*on_hover_t)(const mouse_event_t *const, void *const);
typedef void (*on_press_t)(const mouse_event_t *const, void *const);
typedef void (*on_release_t)(const mouse_event_t *const, void *const);
typedef void (*on_drag_begin_t)(const mouse_event_t *const, void *const);
typedef void (*on_drag_t)(const mouse_event_t *const, const mouse_event_t *const, void *const);
typedef void (*on_drag_end_t)(const mouse_event_t *const, const mouse_event_t *const, void *const);
typedef void (*on_scroll_t)(const mouse_event_t *const, void *const);
typedef void (*render_t)(const canvas_t *const canvas, display_t *const display, const size_t id);

typedef struct
{
    on_hover_t hover;
    on_press_t press;
    on_release_t release;
    on_drag_begin_t drag_begin;
    on_drag_t drag;
    on_drag_end_t drag_end;
    on_scroll_t scroll;
    render_t render;
}
behavior_opts_t;

canvas_t canvas_init(void);
void canvas_deinit(canvas_t *const canvas);
void canvas_load_objects(canvas_t *const canvas);
void canvas_render(const canvas_t *const canvas, display_t *const display);
void canvas_init_behaviors(canvas_t *const canvas);
size_t canvas_create_behavior(canvas_t *const canvas, behavior_opts_t behavior_opts);
vec2_t shifted_camera_position(const transform_comp_t *const camera_transform);

#endif//_CANVAS_H_
