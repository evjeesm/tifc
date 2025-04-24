#include "panel_manager.h"

#include "display_types.h"
#include "dynarr.h"
#include "logger.h"
#include "panel.h"

#define ARENA_IMPLEMENTATION
#include "arena.h"

#include <assert.h>
#include <stdlib.h>


static int delete_panel(void *const panel, void *const _);
static int recalc_panel(void *const panel, void *const bounds);
static int render_panel(const void *const panel, void *const display);


void pm_init(panel_manager_t *const pm)
{
    assert(pm);
    *pm = (panel_manager_t){
        .arena = (Arena){0},
        .panels = dynarr_create(.element_size = sizeof(panel_t)),
    };

    if (!pm->panels)
    {
        S_LOG(LOGGER_CRITICAL, "Failed to create panels array!");
        exit(EXIT_FAILURE);
    }
}


void pm_add_panel(panel_manager_t *const pm, const panel_opts_t *const opts)
{
    assert(pm);
    panel_t panel; panel_init(&panel, opts, &pm->arena);
    (void) dynarr_append(&pm->panels, &panel); /* !! panel order is important */
}


void pm_delete_panels(panel_manager_t *const pm)
{
    assert(pm);

    dynarr_transform(pm->panels, delete_panel, NULL);
    dynarr_remove_range(&pm->panels, 0, dynarr_size(pm->panels));
}


void pm_recalculate(panel_manager_t *const pm, disp_area_t *const bounds)
{
    assert(pm);
    assert(bounds);

    dynarr_transform(pm->panels, recalc_panel, bounds);
}


void pm_hover(panel_manager_t *const pm, const disp_pos_t pos)
{
    assert(pm);
    panel_t *cur_hovered = pm_peek_panel(pm, pos);
    if (pm->last_hovered != cur_hovered)
    {
        if (pm->last_hovered) { panel_leave(pm->last_hovered, pos); }
        if (cur_hovered) { panel_enter(cur_hovered, pos); }
    }
    else {
        if (cur_hovered) { panel_hover(cur_hovered, pos); }
    }

    pm->last_hovered = cur_hovered;
}


void pm_press(panel_manager_t *const pm, const disp_pos_t pos, const int btn)
{
    assert(pm);
    panel_t *cur_pressed = pm_peek_panel(pm, pos);
    if (cur_pressed) panel_press(cur_pressed, pos, btn);
}


void pm_release(panel_manager_t *const pm, const disp_pos_t pos, const int btn)
{
    assert(pm);
    panel_t *cur_released = pm_peek_panel(pm, pos);
    if (cur_released) panel_release(cur_released, pos, btn);
}


void pm_scroll(panel_manager_t *const pm, const disp_pos_t pos, const int dir)
{
    assert(pm);
    panel_t *cur_scrolled = pm_peek_panel(pm, pos);
    if (cur_scrolled) panel_scroll(cur_scrolled, pos, dir);
}


panel_t *pm_peek_panel(panel_manager_t *const pm, const disp_pos_t pos)
{
    assert(pm);

    const size_t panels_count = dynarr_size(pm->panels);

    for (size_t pi = 0; pi < panels_count; ++pi)
    {
        panel_t *panel = dynarr_get(pm->panels, pi);

        if ( pos.x >= panel->area.first.x && pos.x <= panel->area.second.x
          && pos.y >= panel->area.first.y && pos.y <= panel->area.second.y)
        {
            return panel;
        }
    }

    return NULL;
}


void pm_render(const panel_manager_t *const pm, display_t *const display)
{
    assert(pm);

    dynarr_foreach(pm->panels, render_panel, display);
}


void pm_deinit(panel_manager_t *const pm)
{
    assert(pm);
    pm_delete_panels(pm);

    dynarr_destroy(pm->panels);
    arena_free(&pm->arena);
}


static int delete_panel(void *const panel, void *const _)
{ (void) _;
    panel_deinit(panel);
    return 0;
}


static int recalc_panel(void *const panel, void *const bounds)
{
    panel_recalculate(panel, bounds);
    return 0;
}


static int render_panel(const void *const panel, void *const display)
{
    panel_render(panel, display);
    return 0;
}
