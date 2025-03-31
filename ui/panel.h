#ifndef _PANEL_H_
#define _PANEL_H_

#include "panel_interface.h"
#include "display.h"

typedef struct panel
{
    panel_interface_t ifce;
    const char *title;
    unsigned int title_size;
    panel_layout_t layout;
    style_t style;
    disp_area_t area;
}
panel_t;

void panel_init(panel_t *const panel, const panel_opts_t *const opts);

void panel_deinit(panel_t *const panel);

void panel_render(const panel_t *panel, display_t *const display);

void panel_recalculate(panel_t *panel, disp_area_t *const bounds);

void panel_hover(panel_t *const panel, const disp_pos_t pos);

void panel_scroll(panel_t *const panel, const int direction);


/* TODO: add other event handlers by panel interface */

#endif // _PANEL_H_
