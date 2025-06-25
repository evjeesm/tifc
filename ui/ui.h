#ifndef _UI_H_
#define _UI_H_

#include "input.h"
#include "sparse.h"
#include "logger.h"
#include "panel_manager.h"
#include "panel.h"

typedef struct
{
    input_hooks_t hooks;
    panel_manager_t pm;
    bool exit_requested;
}
ui_t;

ui_t ui_init(void);
void ui_deinit(ui_t *const ui);

void ui_recalculate(ui_t *const ui, const display_t *const display);

void ui_resize_hook(const display_t *const display, void *const data);

void ui_render(const ui_t *const ui, display_t *const display);

void ui_add_panel(ui_t *const ui, const panel_opts_t *const opts);


#endif /* _UI_H_ */
