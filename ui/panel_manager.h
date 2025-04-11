#ifndef _PANEL_MANAGER_H_
#define _PANEL_MANAGER_H_

#include "dynarr.h"
#include "arena.h"
#include "panel.h"

typedef struct
{
    Arena    arena;   /* arena that is used for creating panels */
    dynarr_t *panels; /* storage for panel references */
}
panel_manager_t;

void pm_init(panel_manager_t *const pm);
void pm_add_panel(panel_manager_t *const pm, const panel_opts_t *const opts);
void pm_delete_panels(panel_manager_t *const pm);
void pm_recalculate(panel_manager_t *const pm, disp_area_t *const bounds);
panel_t *pm_peek_panel(panel_manager_t *const pm, const disp_pos_t pos);
void pm_render(const panel_manager_t *const pm, display_t *const display);
void pm_deinit(panel_manager_t *const pm);

#endif/*_PANEL_MANAGER_H_*/
