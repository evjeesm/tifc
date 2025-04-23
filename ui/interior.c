#include "interior.h"
#include "interior_layout.h"
#include "vector.h"


interior_t *interior_alloc(const interior_opts_t *const opts, Arena *const arena)
{
    assert(opts);

    return opts->impl.alloc(arena);
}


void interior_init(interior_t *const interior, const interior_opts_t *const opts, Arena *const arena)
{
    assert(interior);
    assert(opts);

    *interior = (interior_t){
        .impl = opts->impl,
    };

    interior_layout_init(&interior->layout, &opts->layout);
    interior->impl.init(interior, (void*)opts, arena);
}


void interior_deinit(interior_t *const interior)
{
    assert(interior);
    interior_layout_deinit(&interior->layout);
    interior->impl.deinit(interior);
}


void interior_recalculate(interior_t *interior, disp_area_t *const panel_area)
{
    assert(interior);

    interior_layout_recalculate(&interior->layout, panel_area);
    interior->impl.recalculate(interior, panel_area);
}


void interior_render(const interior_t *interior, display_t *const display)
{
    assert(interior);
    interior->impl.render(interior, display);
}


void interior_hover(interior_t *const interior, const disp_pos_t pos)
{
    assert(interior);
    interior_area_t *hovered = interior_layout_peek_area(&interior->layout, pos);
    if (hovered) { interior->last_hovered = hovered; }

    interior->impl.hover(interior, pos);
}


void interior_scroll(interior_t *const interior, const int direction)
{
    assert(interior);
    interior->impl.scroll(interior, direction);
}


/*
* TODO: this pretend to be a legit functionality of a vector btw!
*/
size_t last_hovered_ptr_to_index(const interior_t *const interior, const interior_area_t *const ptr)
{
    const interior_area_t *origin = (interior_area_t*) vector_data(interior->layout.areas);
    assert(ptr >= origin);
    return (ptr - origin);
}

