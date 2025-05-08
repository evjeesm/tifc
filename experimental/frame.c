#include "frame.h"
#include "canvas.h"
#include "sparse.h"

#include <stdlib.h>
#include <stdio.h>

static void draw_border(display_t *const display, wchar_t border_char, disp_pos_t pos, style_t *style);

/// !!! not thread safe !!!
///
/// Initializes behavior if yet not initialized,
/// otherwise just returns a cached behavior id.
size_t init_frame_behavior(canvas_t *canvas)
{
    static bool initialized = false;
    static size_t frame_behavior_id;
    if (initialized) return frame_behavior_id;

    frame_behavior_id = canvas_create_behavior(canvas,
        (behavior_opts_t){
            .render = render_frame,
        }
    );
    initialized = true;

    return frame_behavior_id;
}

size_t create_frame(canvas_t *const canvas, vec2_t pos, vec2_t box)
{
    components_t *const components = &canvas->components;
    size_t new_frame_index = sparse_last_free_index(components->transform);
    size_t behavior_id = init_frame_behavior(canvas); // first time init
    transform_comp_t transform = {.location = pos};
    sparse_insert(&components->transform, new_frame_index, &transform);
    sparse_insert(&components->box, new_frame_index, &box);
    sparse_insert(&components->behavior, new_frame_index, &behavior_id);
    dynarr_append(&canvas->ents.frames, &new_frame_index);
    return new_frame_index;
}

size_t create_styled_frame(canvas_t *const canvas, vec2_t pos, vec2_t box, const style_t style)
{
    size_t frame_id = create_frame(canvas, pos, box);
    style_t *data = malloc(sizeof(style_t));
    *data = style;
    sparse_insert(&canvas->components.data, frame_id, &data);
    return frame_id;
}

void destroy_frame(canvas_t *const canvas, const size_t frame_id)
{
    components_t *const components = &canvas->components;
    style_t *data = sparse_get(components->data, frame_id);
    sparse_remove(&components->transform, frame_id);
    sparse_remove(&components->box, frame_id);
    sparse_remove(&components->behavior, frame_id);
    if (data)
    {
        sparse_remove(&components->data, frame_id);
        free(data);
    }
}

void render_frame(const canvas_t *const canvas, display_t *const display, const size_t frame_id)
{
    const components_t *const components = &canvas->components;
    transform_comp_t *camera = sparse_get(components->transform, canvas->ents.camera_id);
    vec2_t camera_pos = shifted_camera_position(camera);
    transform_comp_t *frame_transform = sparse_get(components->transform, frame_id);
    vec2_t *frame_box = sparse_get(components->box, frame_id);
    style_t **style = sparse_get(components->data, frame_id);

    vec2_t beg = {
        .x = frame_transform->location.x + frame_transform->delta.x - camera_pos.x,
        .y = frame_transform->location.y + frame_transform->delta.y - camera_pos.y,
    };
    vec2_t end = {
        beg.x + frame_box->x - 1,
        beg.y + frame_box->y - 1
    };

    for (long long y = beg.y; y <= end.y; ++y)
    {
        for (long long x = beg.x; x <= end.x; ++x)
        {
            // render
            if (x >= 0 && y >= 0 && x < display->size.x && y < display->size.y) // inside screen
            {
                disp_pos_t pos = {x, y};

                display_set_char(display, U' ', pos);
                if (x == beg.x && y == beg.y) draw_border(display, U'╔', pos, *style);
                else if (x == end.x && y == beg.y) draw_border(display, U'╗', pos, *style);
                else if (x == end.x && y == end.y) draw_border(display, U'╝', pos, *style);
                else if (x == beg.x && y == end.y) draw_border(display, U'╚', pos, *style);
                else if (x == beg.x || x == end.x) draw_border(display, U'║', pos, *style);
                else if (y == beg.y || y == end.y) draw_border(display, U'═', pos, *style);
            }
        }
    }
}

static void draw_border(display_t *const display, wchar_t border_char, disp_pos_t pos, style_t *style)
{
    if (style)
    {
        display_set_style(display, *style, pos);
    }
    display_set_char(display, border_char, pos);
}
