#include "interior.h"
#include "interior_layout.h"
#include "utils.h"


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


void interior_enter(interior_t *const interior, const disp_pos_t pos)
{
    assert(interior);
    interior->impl.enter(interior, pos);
    interior_hover(interior, pos);
}


void interior_hover(interior_t *const interior, const disp_pos_t pos)
{
    assert(interior);
    interior->impl.hover(interior, pos);
}


void interior_leave(interior_t *const interior, const disp_pos_t pos)
{
    assert(interior);
    interior->impl.leave(interior, pos);
}


void interior_scroll(interior_t *const interior, const disp_pos_t pos, const int direction)
{
    assert(interior);
    interior->impl.scroll(interior, pos, direction);
}


void interior_press(interior_t *const interior, const disp_pos_t pos, const int btn)
{
    assert(interior);
    interior->impl.press(interior, pos, btn);
}


void interior_release(interior_t *const interior, const disp_pos_t pos, const int btn)
{
    assert(interior);
    interior->impl.release(interior, pos, btn);
}


void interior_keystroke(interior_t *const interior, const keystroke_event_t *const event)
{
    assert(interior);
    interior->impl.keystroke(interior, event);
}


void interior_recv_focus(interior_t *const interior)
{
    UNUSED(interior);
    interior->impl.recv_focus(interior);
}


void interior_lost_focus(interior_t *const interior)
{
    UNUSED(interior);
    interior->impl.lost_focus(interior);
}


void interior_keystroke_stub(interior_t *const interior, const keystroke_event_t *const event)
{
    UNUSED(interior, event);
}


void interior_scroll_stub(interior_t *const interior, const disp_pos_t pos, const int direction)
{
    UNUSED(interior, pos, direction);
}


void interior_press_release_stub(interior_t *const interior, const disp_pos_t pos, const int btn)
{
    UNUSED(interior, pos, btn);
}


void interior_focus_stub(interior_t *const interior)
{
    UNUSED(interior);
}
