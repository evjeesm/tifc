#include "composite_interior.h"
#include "display_types.h"
#include "interior.h"
#include "interior_layout.h"

#include "logger.h"
#include "sparse.h"

#include <stdlib.h>

/* only extended part */
typedef struct
{
    sparse_t *components;     /* maps component to an area index */
    interior_t *last_hovered; /* last hovered component */
}
composite_interior_slice_t;


struct composite_interior
{
    interior_t interior;
    composite_interior_slice_t composite;
};


static void *composite_interior_alloc(Arena *arena);
static void composite_interior_init(interior_t *const base, void *opts, Arena *const arena);
static void composite_interior_deinit(interior_t *const base);
static void composite_interior_recalculate(interior_t *const base, disp_area_t *const panel_area);
static void composite_interior_render(const interior_t *base, display_t *const display);
static void composite_interior_enter(interior_t *const base, const disp_pos_t pos);
static void composite_interior_hover(interior_t *const base, const disp_pos_t pos);
static void composite_interior_leave(interior_t *const base, const disp_pos_t pos);
static void composite_interior_scroll(interior_t *const base, const disp_pos_t pos, const int dir);
static void composite_interior_press(interior_t *const base, const disp_pos_t pos, const int btn);
static void composite_interior_release(interior_t *const base, const disp_pos_t pos, const int btn);


interior_interface_t composite_interior_get_impl(void)
{
    return (interior_interface_t){
        .alloc = composite_interior_alloc,
        .init = composite_interior_init,
        .deinit = composite_interior_deinit,
        .recalculate = composite_interior_recalculate,
        .render = composite_interior_render,
        .enter = composite_interior_enter,
        .hover = composite_interior_hover,
        .leave = composite_interior_leave,
        .scroll = composite_interior_scroll,
        .press = composite_interior_press,
        .release = composite_interior_release,
    };
}


static void *composite_interior_alloc(Arena *arena)
{
    return arena_alloc(arena, sizeof(composite_interior_t));
}


static void composite_interior_init(interior_t *const base, void *opts, Arena *const arena)
{
    composite_interior_opts_t *composite_opts = opts;
    composite_interior_t *interior = (composite_interior_t*)base;

    interior->composite = (composite_interior_slice_t) {
        .components = sparse_create(.element_size = sizeof(interior_t*)),
    };

    for (size_t ci = 0; ci < composite_opts->components_amount; ++ci)
    {
        interior_opts_t *comp_opts = composite_opts->component_defs[ci].opts;
        interior_t *component = interior_alloc(comp_opts, arena);
        interior_init(component, comp_opts, arena);

        sparse_status_t status = sparse_insert(&interior->composite.components,
                composite_opts->component_defs[ci].area_idx,
                &component);

        if (status == SPARSE_INSERT_INDEX_OVERRIDE)
        {
            S_LOG(LOGGER_CRITICAL, "Can't assign component to same area twice!\n");
            exit(EXIT_FAILURE);
        }
    }
}

static int deinit_component(const size_t index, void *const element, void *const param)
{
    (void) index;
    (void) param;
    interior_t **component = element;
    interior_deinit(*component);
    return 0;
}


static void composite_interior_deinit(interior_t *const base)
{
    composite_interior_t *interior = (composite_interior_t*)base;
    (void) sparse_transform(interior->composite.components, deinit_component, NULL);
    sparse_destroy(interior->composite.components);
}


static void composite_interior_recalculate(interior_t *const base, disp_area_t *const panel_area)
{
    composite_interior_t *interior = (composite_interior_t*)base;
    (void) panel_area;

    const size_t first = sparse_first_index(interior->composite.components);
    const size_t last = sparse_last_index(interior->composite.components);

    for (size_t ci = first; ci <= last; ++ci)
    {
        interior_t **comp = sparse_get(interior->composite.components, ci);
        interior_area_t *area = dynarr_get(base->layout.areas, ci);
        if (interior_area_is_visible(area))
        {
            interior_recalculate(*comp, &area->area);
        }
    }
}


static void composite_interior_render(const interior_t *base, display_t *const display)
{
    composite_interior_t *interior = (composite_interior_t*)base;

    const size_t areas_count = dynarr_size(base->layout.areas);
    for (size_t ai = 0; ai < areas_count; ++ai)
    {
        interior_area_t *area = dynarr_get(base->layout.areas, ai);
        if (interior_area_is_visible(area))
        {
            interior_t **comp = sparse_get(interior->composite.components, ai);
            if (comp)
            {
                interior_render(*comp, display); // TODO hovered?
            }
        }
    }
}


static void composite_interior_enter(interior_t *const base, const disp_pos_t pos)
{
    (void) base; (void) pos;
}


static void composite_interior_hover(interior_t *const base, const disp_pos_t pos)
{
    composite_interior_t *interior = (composite_interior_t*)base;
    ssize_t area_index = interior_layout_peek_area_index(&base->layout, pos);
    if (-1 == area_index)
    {
        return;
    }

    interior_t *last_hovered = interior->composite.last_hovered;
    interior_t **comp = sparse_get(interior->composite.components, area_index);
    if (comp)
    {
        interior_t *cur_hovered = *comp;
        if (last_hovered != *comp)
        {
            if (last_hovered) { interior_leave(last_hovered, pos); }
            if (cur_hovered) { interior_enter(cur_hovered, pos); }
        }
        else {
            if (cur_hovered) { interior_hover(cur_hovered, pos); }
        }

        interior->composite.last_hovered = cur_hovered;
    }
}


static void composite_interior_leave(interior_t *const base, const disp_pos_t pos)
{
    composite_interior_t *interior = (composite_interior_t*)base;
    (void) pos;
    if (interior->composite.last_hovered)
    {
        interior_leave(interior->composite.last_hovered, pos);
    }
}


static void composite_interior_scroll(interior_t *const base, const disp_pos_t pos, const int dir)
{
    composite_interior_t *interior = (composite_interior_t*)base;

#if __BASED_ON_LAST_HOVERED
    if (!base->last_hovered)
    {
        return;
    }
    size_t index = last_hovered_ptr_to_index(base, base->last_hovered);
#else
    ssize_t index = interior_layout_peek_area_index(&base->layout, pos);
    if (-1 == index) { return; }
#endif
    interior_t **comp = sparse_get(interior->composite.components, index);
    if (comp)
    {
        interior_scroll(*comp, pos, dir);
    }
}


static void composite_interior_press(interior_t *const base, const disp_pos_t pos, const int btn)
{
    composite_interior_t *interior = (composite_interior_t*)base;
#if __BASED_ON_LAST_HOVERED
    if (!base->last_hovered)
    {
        return;
    }
    size_t index = last_hovered_ptr_to_index(base, base->last_hovered);
#else
    ssize_t index = interior_layout_peek_area_index(&base->layout, pos);
    if (-1 == index) { return; }
#endif
    interior_t **comp = sparse_get(interior->composite.components, index);
    if (comp)
    {
        interior_press(*comp, pos, btn);
    }
}


static void composite_interior_release(interior_t *const base, const disp_pos_t pos, const int btn)
{
    composite_interior_t *interior = (composite_interior_t*)base;
#if __BASED_ON_LAST_HOVERED
    if (!base->last_hovered)
    {
        return;
    }
    size_t index = last_hovered_ptr_to_index(base, base->last_hovered);
#else
    ssize_t index = interior_layout_peek_area_index(&base->layout, pos);
    if (-1 == index) { return; }
#endif
    interior_t **comp = sparse_get(interior->composite.components, index);
    if (comp)
    {
        interior_release(*comp, pos, btn);
    }
}
