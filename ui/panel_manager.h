#ifndef _PANEL_MANAGER_H_
#define _PANEL_MANAGER_H_

#include "dynarr.h"
#include "arena.h"
#include "panel.h"

typedef struct
{
    Arena    arena;   /* arena that is used for creating panels */
    dynarr_t *panels; /* storage for panel references */
    panel_t  *last_hovered;
    panel_t  *focused; /* panel that has focus (type events will go there) */
}
panel_manager_t;

void pm_init(panel_manager_t *const pm);
void pm_add_panel(panel_manager_t *const pm, const panel_opts_t *const opts);
void pm_delete_panels(panel_manager_t *const pm);
void pm_recalculate(panel_manager_t *const pm, disp_area_t *const bounds);
void pm_hover(panel_manager_t *const pm, const disp_pos_t pos);
void pm_press(panel_manager_t *const pm, const disp_pos_t pos, const int btn);
void pm_release(panel_manager_t *const pm, const disp_pos_t pos, const int btn);
void pm_scroll(panel_manager_t *const pm, const disp_pos_t pos, const int dir);
void pm_keystroke(panel_manager_t *const pm, const keystroke_event_t *const event);
panel_t *pm_peek_panel(panel_manager_t *const pm, const disp_pos_t pos);
panel_t *pm_get_focused_panel(panel_manager_t *const pm);
void pm_set_focused_panel(panel_manager_t *const pm, panel_t *const panel);
void pm_clear_focus(panel_manager_t *const pm);
void pm_focus_next_panel(panel_manager_t *const pm);
void pm_focus_prev_panel(panel_manager_t *const pm);
void pm_render(const panel_manager_t *const pm, display_t *const display);
void pm_deinit(panel_manager_t *const pm);

#endif/*_PANEL_MANAGER_H_*/
