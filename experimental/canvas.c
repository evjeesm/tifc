
#include "tifc.h"
#include "canvas.h"
#include "display.h"
#include "dynarr.h"
#include "input.h"
#include "frame.h"
#include "sparse.h"

#include <stdio.h>

//
// Mouse events
//
static void on_hover(const mouse_event_t *const, void *const);
static void on_press(const mouse_event_t *const, void *const);
static void on_release(const mouse_event_t *const, void *const);
static void on_drag_begin(const mouse_event_t *const, void *const);
static void on_drag(const mouse_event_t *const, const mouse_event_t *const, void *const);
static void on_drag_end(const mouse_event_t *const, const mouse_event_t *const, void *const);
static void on_scroll(const mouse_event_t *const, void *const);

static input_hooks_t hooks_init(void)
{
    return (input_hooks_t)
    {
        .on_hover = on_hover,
        .on_press = on_press,
        .on_release = on_release,
        .on_drag_begin = on_drag_begin,
        .on_drag = on_drag,
        .on_drag_end = on_drag_end,
        .on_scroll = on_scroll
    };
}

//
// Components
//
static components_t components_init(void);
static void components_deinit(components_t *const components);
static behaviors_t behaviors_init(void);
static void behaviors_deinit(behaviors_t *const behaviors);


//
// Render routines
//
static void render_grid(transform_comp_t *camera_transform, display_t *const display, const disp_pos_t grid);


static size_t create_camera(components_t *const components);


canvas_t canvas_init(void)
{
    return (canvas_t) {
        .components = components_init(),
        .hooks = hooks_init(),
        .behaviors = behaviors_init(),
        .ents.frames = dynarr_create(.element_size = sizeof(size_t)),
    };
}

void canvas_deinit(canvas_t *const canvas)
{
    behaviors_deinit(&canvas->behaviors);
    components_deinit(&canvas->components);
    dynarr_destroy(canvas->ents.frames);
}


void canvas_load_objects(canvas_t *const canvas)
{
    canvas->ents.camera_id = create_camera(&canvas->components);
    //
    // create_styled_frame(canvas,
    //    (vec2_t){.x = 0, .y = 0},
    //    (vec2_t){.x = 10, .y = 10},
    //    BORDER_STYLE_1
    // );
    // create_styled_frame(canvas,
    //    (vec2_t){.x = 50, .y = 10},
    //    (vec2_t){.x = 30, .y = 20},
    //    BORDER_STYLE_2
    // );
    // create_styled_frame(canvas,
    //    (vec2_t){.x = 30, .y = 25},
    //    (vec2_t){.x = 22, .y = 16},
    //    BORDER_STYLE_3
    // );
    // Add more objects here
}


void canvas_render(const canvas_t *const canvas, display_t *const display)
{

    transform_comp_t *t = sparse_get(canvas->components.transform, canvas->ents.camera_id);
    render_grid(t, display, (disp_pos_t){30, 15});

    const size_t max_frames = dynarr_size(canvas->ents.frames);

    for (size_t i = 0; i < max_frames; ++i)
    {
        size_t *frame_id = dynarr_get(canvas->ents.frames, i);
        size_t *frame_behavior = sparse_get(canvas->components.behavior, *frame_id);

        // access render behavior of the frame
        render_t *render = (render_t*) sparse_get(canvas->behaviors.render, *frame_behavior);
        (*render)(canvas, display, *frame_id);
    }
}

size_t canvas_create_behavior(canvas_t *const canvas, behavior_opts_t behavior_opts)
{
    size_t id = canvas->behaviors.last_id++;
    if (behavior_opts.hover)      sparse_insert(&canvas->behaviors.hover, id, &behavior_opts.hover);
    if (behavior_opts.press)      sparse_insert(&canvas->behaviors.press, id, &behavior_opts.press);
    if (behavior_opts.release)    sparse_insert(&canvas->behaviors.release, id,&behavior_opts.release);
    if (behavior_opts.drag_begin) sparse_insert(&canvas->behaviors.drag_begin, id, &behavior_opts.drag_begin);
    if (behavior_opts.drag)       sparse_insert(&canvas->behaviors.drag, id, &behavior_opts.drag);
    if (behavior_opts.drag_end)   sparse_insert(&canvas->behaviors.drag_end, id, &behavior_opts.drag_end);
    if (behavior_opts.scroll)     sparse_insert(&canvas->behaviors.scroll, id, &behavior_opts.scroll);
    if (behavior_opts.render)     sparse_insert(&canvas->behaviors.render, id, &behavior_opts.render);
    return id;
}

vec2_t shifted_camera_position(const transform_comp_t *const camera_transform)
{
    return (vec2_t) {
        .x = camera_transform->location.x - camera_transform->delta.x,
        .y = camera_transform->location.y - camera_transform->delta.y
    };
}

static void on_hover(const mouse_event_t *const hover, void *const param)
{
    (void) param;
    printf(ROW(1) "hover, at %u, %u                         \n",
        hover->position.x, hover->position.y);
}

static void on_press(const mouse_event_t *const press, void *const param)
{
    tifc_t *tifc = param;
    printf(ROW(1) "press %d, at %u, %u                      \n",
        press->mouse_button,
        press->position.x, press->position.y);

    if (press->mouse_button == MOUSE_3) tifc_ui_mode(tifc);
    //
    // search target object
    // activate act on press
}

static void on_release(const mouse_event_t *const press, void *const param)
{
    (void) param;
    printf(ROW(1) "release %d, at %u, %u                    \n",
        press->mouse_button,
        press->position.x, press->position.y);
    //
    // same target object
    // act on release
}

static void on_drag_begin(const mouse_event_t *const begin,
        void *const param)
{
    (void) param;
    printf(ROW(1) "drag %d begin, at %u, %u                 \n",
        begin->mouse_button,
        begin->position.x, begin->position.y);
}

static void on_drag(const mouse_event_t *const begin, const mouse_event_t *const moved, void *const param)
{
    tifc_t *tifc = param;
    canvas_t *canvas = &tifc->canvas;

    printf(ROW(1) "drag %d drag moving to %u, %u            \n",
        begin->mouse_button,
        moved->position.x, moved->position.y);

    if (begin->mouse_button == MOUSE_2)
    {
        transform_comp_t *t = sparse_get(canvas->components.transform, canvas->ents.camera_id);

        t->delta = (vec2_t){
            .x = (long long)moved->position.x - begin->position.x,
            .y = (long long)moved->position.y - begin->position.y
        };
    }
}

static void on_drag_end(const mouse_event_t *const begin,
        const mouse_event_t *const end, void *const param)
{
    tifc_t *tifc = param;
    canvas_t *canvas = &tifc->canvas;

    printf(ROW(1) "drag %d from %u, %u to %u, %u            \n",
        begin->mouse_button,
        begin->position.x, begin->position.y,
        end->position.x, end->position.y);

    if (begin->mouse_button == MOUSE_2)
    {
        transform_comp_t *t = sparse_get(canvas->components.transform, canvas->ents.camera_id);

        t->location = (vec2_t){
            t->location.x - t->delta.x,
            t->location.y - t->delta.y
        };
        t->delta.x = t->delta.y = 0;
    }
}

static void on_scroll(const mouse_event_t *const scroll, void *const param)
{
    (void) param;
    printf(ROW(1) "scroll %d at %u, %u                      \n",
        scroll->mouse_button,
        scroll->position.x, scroll->position.y);
}


static components_t components_init(void)
{
    return (components_t)
    {
        .transform = sparse_create(.element_size = sizeof(transform_comp_t)),
        .box = sparse_create(.element_size = sizeof(vec2_t)),
        .behavior = sparse_create(.element_size = sizeof(size_t)),
        .data = sparse_create(.element_size = sizeof(void*)),
    };
}


static void components_deinit(components_t *const components)
{
    sparse_destroy(components->transform);
    sparse_destroy(components->box);
    sparse_destroy(components->behavior);
    sparse_destroy(components->data);
}

static behaviors_t behaviors_init(void)
{
    return (behaviors_t)
    {
        .hover = sparse_create(.element_size = sizeof(on_hover_t)),
        .press = sparse_create(.element_size = sizeof(on_press_t)),
        .release = sparse_create(.element_size = sizeof(on_release_t)),
        .drag_begin = sparse_create(.element_size = sizeof(on_drag_begin_t)),
        .drag = sparse_create(.element_size = sizeof(on_drag_t)),
        .drag_end = sparse_create(.element_size = sizeof(on_drag_end_t)),
        .scroll = sparse_create(.element_size = sizeof(on_scroll_t)),
        .render = sparse_create(.element_size = sizeof(render_t)),
    };
}

static void behaviors_deinit(behaviors_t *const behaviors)
{
    sparse_destroy(behaviors->hover);
    sparse_destroy(behaviors->press);
    sparse_destroy(behaviors->release);
    sparse_destroy(behaviors->drag_begin);
    sparse_destroy(behaviors->drag);
    sparse_destroy(behaviors->drag_end);
    sparse_destroy(behaviors->scroll);
    sparse_destroy(behaviors->render);
}


static void render_grid(transform_comp_t *camera_transform, display_t *const display, const disp_pos_t grid)
{
    vec2_t camera_pos = shifted_camera_position(camera_transform);

    for (unsigned int line = 0; line < display->size.y; ++line)
    {
        for (unsigned int col = 0; col < display->size.x; ++col)
        {
            if (((line + camera_pos.y) % grid.y) == 0
                && ((col + camera_pos.x) % grid.x) == 0)
            {
                display_set_char(display, U'+', (disp_pos_t){col, line});
            }
        }
    }

    printf(HOME "CAMERA: [%lld, %lld]      \n", camera_pos.x, camera_pos.y);
}

static size_t create_camera(components_t *const components)
{
    const size_t new_component_index = sparse_last_free_index(components->transform);
    transform_comp_t camera_transform = {0};
    sparse_insert(&components->transform, new_component_index, &camera_transform);
    return new_component_index;
}

