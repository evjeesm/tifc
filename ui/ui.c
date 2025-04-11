#include "ui.h"
#include "display_types.h"
#include "logger.h"
#include "panel.h"
#include "display.h"
#include "panel_manager.h"

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
static panel_t *ui_peek_panel(ui_t *const ui, const disp_pos_t pos);


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


ui_t ui_init(void)
{
    ui_t ui = {
        .hooks = hooks_init(),
    };
    pm_init(&ui.pm);
    return ui;
}


void ui_deinit(ui_t *const ui)
{
    assert(ui);
    pm_deinit(&ui->pm);
}


void ui_recalculate(ui_t *const ui, const display_t *const display)
{
    assert(ui);
    assert(display);

    disp_area_t bounds = {
        .first = {0, 0},
        .second = {display->size.x - 1, display->size.y - 1}
    };
    pm_recalculate(&ui->pm, &bounds);
}


void ui_resize_hook(const display_t *const display, void *data)
{
    assert(display);
    assert(data);

    ui_recalculate((ui_t*)data, display);
}


void ui_render(const ui_t *const ui,
               display_t *const display)
{
    assert(ui);
    assert(display);

    pm_render(&ui->pm, display);
}


void ui_add_panel(ui_t *const ui, const panel_opts_t *const opts)
{
    assert(ui);
    assert(opts);

    pm_add_panel(&ui->pm, opts);
}


static panel_t *ui_peek_panel(ui_t *const ui, const disp_pos_t pos)
{
    return pm_peek_panel(&ui->pm, pos);
}


static void on_hover(const mouse_event_t *const hover, void *const param)
{
    ui_t *ui = param;
    S_LOG(LOGGER_DEBUG, "UI::hover, at %u, %u\n",
        hover->position.x, hover->position.y);

    /* zero based position */
    disp_pos_t norm_pos = {hover->position.x - 1, hover->position.y - 1};
    panel_t *panel = ui_peek_panel(ui, norm_pos);
    if (!panel) { return; }
    panel_hover(panel, norm_pos);
}


static void on_press(const mouse_event_t *const press, void *const param)
{
    (void) param;
    S_LOG(LOGGER_DEBUG, "UI::press %d, at %u, %u\n",
        press->mouse_button, press->position.x, press->position.y);
}


static void on_release(const mouse_event_t *const press, void *const param)
{
    (void) param;
    S_LOG(LOGGER_DEBUG, "UI::release %d, at %u, %u\n",
        press->mouse_button, press->position.x, press->position.y);
}


static void on_drag_begin(const mouse_event_t *const begin,
        void *const param)
{
    (void) param;
    S_LOG(LOGGER_DEBUG, "UI::drag %d begin, at %u, %u\n",
        begin->mouse_button, begin->position.x, begin->position.y);
}


static void on_drag(const mouse_event_t *const begin, const mouse_event_t *const moved, void *const param)
{
    (void) param;
    S_LOG(LOGGER_DEBUG, "UI::drag %d drag moving to %u, %u\n",
        begin->mouse_button, moved->position.x, moved->position.y);
}


static void on_drag_end(const mouse_event_t *const begin,
        const mouse_event_t *const end, void *const param)
{
    (void) param;
    S_LOG(LOGGER_DEBUG, "UI::drag %d from %u, %u to %u, %u\n",
        begin->mouse_button,
        begin->position.x, begin->position.y,
        end->position.x, end->position.y);
}


static void on_scroll(const mouse_event_t *const scroll, void *const param)
{
    ui_t *ui = param;

    S_LOG(LOGGER_DEBUG, "UI::scroll %d at %u, %u\n",
        scroll->mouse_button, scroll->position.x, scroll->position.y);

    /* zero based position */
    disp_pos_t norm_pos = {scroll->position.x - 1, scroll->position.y - 1};
    panel_t *panel = ui_peek_panel(ui, norm_pos);

    if (!panel) { return; }

    panel_scroll(panel, scroll->mouse_button);
}

