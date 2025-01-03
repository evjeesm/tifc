#ifndef _UI_H_
#define _UI_H_

#include "layout.h"
#include "input.h"
#include "sparse.h"
#include "panel.h"

typedef struct
{
    input_hooks_t hooks;
    sparse_t     *panels;
    size_t        last_panel_id;
    sparse_t     *items;
    size_t        last_item_id;
}
ui_t;

ui_t ui_init(void);
void ui_deinit(ui_t *const ui);

void
ui_recalculate_layout(ui_t *const ui,
        const display_t *const display);
void
ui_resize_hook(const display_t *const display,
        void *const data);
void
ui_render(const ui_t *const ui,
        display_t *const display);
void
ui_add_panel(ui_t *const ui,
        const char *title,
        layout_t layout,
        style_t style);
void
ui_add_item(ui_t *const ui, const char *title);

#endif /* _UI_H_ */
