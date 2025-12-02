#include "ui_element.h"
#include "display.h"

#include <assert.h>

#define MIN_UI_ELEMENT_SIZE 2


void *ui_element_alloc(const ui_element_opts_t *const opts)
{
    assert(opts);

    return opts->impl.alloc(opts->arena);
}


void ui_element_init(ui_element_t *const ui_element, const ui_element_opts_t *const opts)
{
    assert(ui_element);
    assert(opts);

    *ui_element = (ui_element_t){
        .impl = opts->impl,
        .area = INVALID_AREA,
        .arena = opts->arena,
    };

    ui_element->impl.init(ui_element, (void*) opts);
}


void ui_element_deinit(ui_element_t *const ui_element)
{
    assert(ui_element);
    ui_element->impl.deinit(ui_element);
}


void ui_element_recalculate(ui_element_t *ui_element, disp_area_t *const bounds)
{
    assert(ui_element);
    assert(bounds);

    // ui_element->area = calc_area(&ui_element->layout, bounds);

    if (IS_INVALID_AREA(&ui_element->area)) return;

    ui_element->impl.recalculate(ui_element, bounds);
}


void ui_element_render(const ui_element_t *ui_element, display_t *const display)
{
    assert(ui_element);
    assert(display);

    // dont render ui_element if has no valid area
    if (IS_INVALID_AREA(&ui_element->area)) return;

    // disp_area_t area = ui_element->area;
    // border_set_t border = {._ = L"╭╮╯╰│─"};
    // display_draw_border(display, ui_element->style, border, area);
    // display_fill_area(display, ui_element->style, area);
    // ui_element_draw_title(ui_element, display);

    ui_element->impl.render(ui_element, display);
}


void ui_element_enter(ui_element_t *const ui_element, const disp_pos_t pos)
{
    ui_element->impl.enter(ui_element, pos);
}


void ui_element_hover(ui_element_t *const ui_element, const disp_pos_t pos)
{
    ui_element->impl.hover(ui_element, pos);
}


void ui_element_leave(ui_element_t *const ui_element, const disp_pos_t pos)
{
    ui_element->impl.leave(ui_element, pos);
}


void ui_element_scroll(ui_element_t *const ui_element, const disp_pos_t pos, const int direction)
{
    ui_element->impl.scroll(ui_element, pos, direction);
}


void ui_element_press(ui_element_t *const ui_element, const disp_pos_t pos, const int btn)
{
    ui_element->impl.press(ui_element, pos, btn);
}


void ui_element_release(ui_element_t *const ui_element, const disp_pos_t pos, const int btn)
{
    ui_element->impl.release(ui_element, pos, btn);
}


void ui_element_keystroke(ui_element_t *const ui_element, const keystroke_event_t *const event)
{
    ui_element->impl.keystroke(ui_element, event);
}


void ui_element_recv_focus(ui_element_t *const ui_element)
{
    ui_element->impl.recv_focus(ui_element);
}


void ui_element_lost_focus(ui_element_t *const ui_element)
{
    ui_element->impl.lost_focus(ui_element);
}


