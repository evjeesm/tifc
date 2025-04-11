#include "interior.h"
#include "interior_layout.h"


interior_t *interior_alloc(const interior_opts_t *const opts)
{
    assert(opts);

    return opts->impl.alloc(opts->arena);
}


void interior_init(interior_t *const interior, const interior_opts_t *const opts)
{
    assert(interior);
    assert(opts);

    *interior = (interior_t){
        .impl = opts->impl,
    };

    interior_layout_init(&interior->layout, &opts->layout);
    interior->impl.init(interior, (void*)opts);
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
    interior->impl.hover(interior, pos);
}


void interior_scroll(interior_t *const interior, const int direction)
{
    assert(interior);
    interior->impl.scroll(interior, direction);
}
