#include "ui.h"
#include "display_types.h"
#include "input.h"
#include "logger.h"
#include "panel.h"
#include "display.h"
#include "panel_manager.h"

//
// Mouse events
//
static void ui_hover(const mouse_event_t *const, void *const);
static void ui_press(const mouse_event_t *const, void *const);
static void ui_release(const mouse_event_t *const, void *const);
static void ui_drag_begin(const mouse_event_t *const, void *const);
static void ui_drag(const mouse_event_t *const, const mouse_event_t *const, void *const);
static void ui_drag_end(const mouse_event_t *const, const mouse_event_t *const, void *const);
static void ui_scroll(const mouse_event_t *const, void *const);

//
// Keyboard
//
static void ui_keystroke(const keystroke_event_t *const, void *const);

static input_hooks_t hooks_init(void)
{
    return (input_hooks_t)
    {
        .on_hover = ui_hover,
        .on_press = ui_press,
        .on_release = ui_release,
        .on_drag_begin = ui_drag_begin,
        .on_drag = ui_drag,
        .on_drag_end = ui_drag_end,
        .on_scroll = ui_scroll,
        .on_keystroke = ui_keystroke,
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


void ui_render(const ui_t *const ui, display_t *const display)
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


static void ui_hover(const mouse_event_t *const hover, void *const param)
{
    ui_t *ui = param;
    S_LOG(LOGGER_DEBUG, "UI::hover, at %u, %u\n",
        hover->position.x, hover->position.y);

    pm_hover(&ui->pm, hover->position);
}


static void ui_press(const mouse_event_t *const press, void *const param)
{
    ui_t *ui = param;
    S_LOG(LOGGER_DEBUG, "UI::press %d, at %u, %u\n",
        press->mouse_button, press->position.x, press->position.y);

    pm_press(&ui->pm, press->position, press->mouse_button);
}


static void ui_release(const mouse_event_t *const press, void *const param)
{
    ui_t *ui = param;
    S_LOG(LOGGER_DEBUG, "UI::release %d, at %u, %u\n",
        press->mouse_button, press->position.x, press->position.y);

    pm_release(&ui->pm, press->position, press->mouse_button);
}


static void ui_drag_begin(const mouse_event_t *const begin,
        void *const param)
{
    UNUSED(param);
    S_LOG(LOGGER_DEBUG, "UI::drag %d begin, at %u, %u\n",
        begin->mouse_button, begin->position.x, begin->position.y);
}


static void ui_drag(const mouse_event_t *const begin, const mouse_event_t *const moved, void *const param)
{
    UNUSED(param);
    S_LOG(LOGGER_DEBUG, "UI::drag %d drag moving to %u, %u\n",
        begin->mouse_button, moved->position.x, moved->position.y);
}


static void ui_drag_end(const mouse_event_t *const begin,
        const mouse_event_t *const end, void *const param)
{
    UNUSED(param);
    S_LOG(LOGGER_DEBUG, "UI::drag %d from %u, %u to %u, %u\n",
        begin->mouse_button,
        begin->position.x, begin->position.y,
        end->position.x, end->position.y);
}


static void ui_scroll(const mouse_event_t *const scroll, void *const param)
{
    ui_t *ui = param;

    S_LOG(LOGGER_DEBUG, "UI::scroll %d at %u, %u\n",
        scroll->mouse_button, scroll->position.x, scroll->position.y);

    pm_scroll(&ui->pm, scroll->position, scroll->mouse_button);
}


static void ui_keystroke(const keystroke_event_t *const event, void *const param)
{
    ui_t *ui = param;

    S_LOG(LOGGER_DEBUG, "UI::keystroke %x(%d)\n mod:%x\n", event->stroke, event->code, event->modifier);

    if (KEY_D == event->code && MOD_CTRL == event->modifier)
    {
        ui->exit_requested = true;
        return;
    }

    pm_keystroke(&ui->pm, event);
}
