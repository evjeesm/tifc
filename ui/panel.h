#ifndef _PANEL_H_
#define _PANEL_H_

#include "display.h"
#include "input.h"
#include "interior.h"

typedef struct
{
    layout_align_t       align;
    layout_size_method_t size_method;
    disp_pos_t           size;
}
panel_layout_t;


typedef struct panel
{
    panel_layout_t layout;
    disp_area_t    area;
    interior_t     *interior;
}
panel_t;


typedef struct
{
    panel_layout_t layout;
    void           *interior_opts;
}
panel_opts_t;


void panel_init(panel_t *const panel, const panel_opts_t *const opts, Arena *const arena);
void panel_deinit(panel_t *const panel);
void panel_render(const panel_t *panel, display_t *const display);
void panel_recalculate(panel_t *panel, disp_area_t *const bounds);
void panel_enter(panel_t *const panel, const disp_pos_t pos);
void panel_hover(panel_t *const panel, const disp_pos_t pos);
void panel_leave(panel_t *const panel, const disp_pos_t pos);
void panel_scroll(panel_t *const panel, const disp_pos_t pos, const int direction);
void panel_press(panel_t *const panel, const disp_pos_t pos, const int btn);
void panel_release(panel_t *const panel, const disp_pos_t pos, const int btn);
void panel_keystroke(panel_t *const panel, const keystroke_event_t *const event);
void panel_special_key(panel_t *const panel, const keystroke_event_t *const event);
void panel_navigation(panel_t *const panel, const keystroke_event_t *const event);
void panel_recv_focus(panel_t *const panel);
void panel_lost_focus(panel_t *const panel);

/* TODO: add other event handlers by panel interface */

#endif // _PANEL_H_
