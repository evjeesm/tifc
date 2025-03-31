#ifndef _PANEL_INTERFACE_H_
#define _PANEL_INTERFACE_H_

#include "display_types.h"
#include "arena.h"

#include "layout.h"

#include <stdbool.h>

typedef struct display display_t;
typedef struct panel panel_t;

typedef struct
{
    void* (*alloc) (Arena *arena);
    void (*init) (panel_t *const panel, void *opts);
    void (*deinit) (panel_t *const panel);
    void (*recalculate) (panel_t *const panel, disp_area_t *const bounds);
    void (*render) (const panel_t *panel, display_t *const display);
    void (*hover) (panel_t *const panel, const disp_pos_t pos);
    void (*scroll) (panel_t *const panel, const int dir);
    /* ... */
}
panel_interface_t;

typedef struct
{
    layout_align_t       align;
    layout_size_method_t size_method;
    disp_pos_t           size;
}
panel_layout_t;

/* tells amount of the data in the source to be displayed */
typedef size_t (*data_get_amount_t) (const void *const data_source);

typedef struct
{
    panel_interface_t ifce;
    const char     * title;
    panel_layout_t   layout;
    bool scrollable;
}
panel_opts_t;

#endif/*_PANEL_INTERFACE_H_*/

