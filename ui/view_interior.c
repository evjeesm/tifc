#include "view_interior.h"
#include "display_types.h"
#include "dynarr.h"
#include "interior.h"
#include "interior_layout.h"
#include "input.h"

struct view_interior
{
    interior_t interior;
    data_source_t source;
    size_t scroll_offset;
};

static void *view_interior_alloc(Arena *arena);
static void view_interior_init(interior_t *const interior, void *opts);
static void view_interior_deinit(interior_t *const interior);
static void view_interior_recalculate(interior_t *const interior, disp_area_t *const panel_area);
static void view_interior_render(const interior_t *interior, display_t *const display);
static void view_interior_hover(interior_t *const interior, const disp_pos_t pos);
static void view_interior_scroll(interior_t *const interior, const int dir);

static void adjust_scroll_position(view_interior_t *const view, const size_t limit);

static size_t data_source_get_amount(const data_source_t *const source);

static void data_source_render(const data_source_t *const source,
        display_t *const display, const interior_area_t *const area,
        const size_t limit, const size_t index, const bool hovered);


interior_interface_t view_interior_get_impl(void)
{
    return (interior_interface_t){
        .alloc = view_interior_alloc,
        .init = view_interior_init,
        .deinit = view_interior_deinit,
        .recalculate = view_interior_recalculate,
        .render = view_interior_render,
        .hover = view_interior_hover,
        .scroll = view_interior_scroll,
    };
}


static void *view_interior_alloc(Arena *arena)
{
    return arena_alloc(arena, sizeof(view_interior_t));
}


static void view_interior_init(interior_t *const interior, void *opts)
{
    view_interior_opts_t *view_opts = opts;
    view_interior_t *view = (view_interior_t*)interior;
    view->source = view_opts->source;
}


static void view_interior_deinit(interior_t *const interior)
{
    (void) interior;
}


static void view_interior_recalculate(interior_t *const interior, disp_area_t *const bounds)
{
    view_interior_t *view = (view_interior_t*)interior;
    interior_layout_recalculate(&view->interior.layout, bounds);
    adjust_scroll_position(view, 0);
}


static void view_interior_render(const interior_t *interior, display_t *const display)
{
    view_interior_t *view = (view_interior_t*)interior;

    const size_t limit = view->source.get_amount(view->source.data);
    const size_t areas_count = dynarr_size(interior->layout.areas);
    for (size_t ai = 0; ai < areas_count; ++ai)
    {
        interior_area_t *area = dynarr_get(interior->layout.areas, ai);
        const bool hovered = (area == interior->last_hovered);

        if (interior_area_is_visible(area))
        {
            data_source_render(&view->source, display, area,
                    limit, ai + view->scroll_offset, hovered);
        }
    }
}


static void view_interior_hover(interior_t *const interior, const disp_pos_t pos)
{
    interior_area_t *hovered = interior_layout_peek_area(&interior->layout, pos);

    if (!hovered) { return; }
    interior->last_hovered = hovered;
}


static void view_interior_scroll(interior_t *const interior, const int dir)
{
    view_interior_t *view = (view_interior_t*)interior;
    const size_t limit = data_source_get_amount(&view->source);
    if (SCROLL_UP == dir)
    {
        if (view->scroll_offset == 0) { return; }
        --view->scroll_offset;
        return;
    }

    ++view->scroll_offset;

    adjust_scroll_position(view, limit);
}


static void adjust_scroll_position(view_interior_t *const view, const size_t limit)
{
    // CACHE? or maybe use last_hovered
    const size_t valid_areas = interior_layout_count_valid_areas(&view->interior.layout);
    const size_t max_scroll_offset = (limit <= valid_areas) ? 0 : limit - valid_areas;

    if (view->scroll_offset >= max_scroll_offset)
    {
        view->scroll_offset = max_scroll_offset;
    }
}



static size_t data_source_get_amount(const data_source_t *const source)
{
    return source->get_amount(source->data);
}


static void data_source_render(const data_source_t *const source,
        display_t *const display, const interior_area_t *const area,
        const size_t limit, const size_t index, const bool hovered)
{
    source->render(display, area, source->data, limit, index, hovered);
}
