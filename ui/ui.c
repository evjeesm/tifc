#include "ui.h"
#include "logger.h"
#include "panel.h"
#include "display.h"
#include "panel_factory.h"
#include "sparse.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

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
static panel_t *ui_get_hovered_panel(ui_t *const ui, const disp_pos_t pos);

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
    /* this will contain a references to panel objects */
    sparse_t *panels = sparse_create(
        .element_size = sizeof(panel_t*)
    );

    if (!panels)
    {
        exit(EXIT_FAILURE);
    }

    return (ui_t) {
        .panels = panels,
        .hooks = hooks_init(),
    };
}

void ui_deinit(ui_t *const ui)
{
    const size_t panels_amount = sparse_size(ui->panels);
    for (size_t i = 0; i < panels_amount; ++i)
    {
        panel_deinit(*(panel_t**)sparse_get(ui->panels, i));
    }
    sparse_destroy(ui->panels);
}

void ui_recalculate(ui_t *const ui, const display_t *const display)
{
    disp_area_t bounds = {
        .first = {0, 0},
        .second = {display->size.x - 1, display->size.y - 1}
    };
    size_t size = sparse_size(ui->panels);
    for (size_t i = 0; i < size; ++i)
    {
        panel_t *panel = *(panel_t**)sparse_get(ui->panels, i);
        if (panel)
        {
            panel_recalculate(panel, &bounds);
        }
    }
}

void ui_resize_hook(const display_t *const display, void *data)
{
    ui_recalculate((ui_t*)data, display);
}

void ui_render(const ui_t *const ui,
               display_t *const display)
{
    size_t size = sparse_size(ui->panels);
    for (size_t i = 0; i < size; ++i)
    {
        panel_t *panel = *(panel_t**)sparse_get(ui->panels, i);
        if (panel)
        {
            panel_render(panel, display);
        }
    }
}

panel_t *ui_add_panel(ui_t *const ui, panel_factory_t *const pf)
{
    const size_t new_panel_index = sparse_last_free_index(ui->panels);
    panel_t *panel = panel_factory_create(pf);
    sparse_insert(&ui->panels, new_panel_index, &panel);
    return panel;
}


static panel_t *ui_get_hovered_panel(ui_t *const ui, const disp_pos_t pos)
{
    const size_t size = sparse_size(ui->panels);
    for (size_t i = 0; i < size; ++i)
    {
        panel_t *panel = *(panel_t**)sparse_get(ui->panels, i);
        if (pos.x >= panel->area.first.x && pos.x <= panel->area.second.x
            && pos.y >= panel->area.first.y && pos.y <= panel->area.second.y)
        {
            return panel;
        }
    }
    return NULL;
}


static void on_hover(const mouse_event_t *const hover, void *const param)
{
    ui_t *ui = param;
    S_LOG(LOGGER_DEBUG, "UI::hover, at %u, %u\n",
        hover->position.x, hover->position.y);

    /* zero based position */
    disp_pos_t norm_pos = {hover->position.x - 1, hover->position.y - 1};
    panel_t *panel = ui_get_hovered_panel(ui, norm_pos);
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
    panel_t *panel = ui_get_hovered_panel(ui, norm_pos);

    if (!panel) { return; }

    panel_scroll(panel, scroll->mouse_button);
}

