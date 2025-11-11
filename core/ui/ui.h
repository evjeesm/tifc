#ifndef _UI_H_
#define _UI_H_

#include "input.h"

typedef struct
{
    input_hooks_t hooks;
    bool exit_requested;
}
ui_t;

void ui_init(ui_t *const ui);
void ui_deinit(ui_t *const ui);
void ui_recalculate(ui_t *const ui, const display_t *const display);

#endif /* _UI_H_ */
