#ifndef _GRID_PANEL_H_
#define _GRID_PANEL_H_

#include "panel_interface.h"
#include "grid.h"

typedef struct grid_panel grid_panel_t;

typedef struct
{
    panel_opts_t panel;

    // Specific to grid content type:
    //  (columns == 0 && rows == 0) means raw content type
    uint8_t columns;
    uint8_t rows;
    uint16_t areas;

    grid_layout_t *column_layout;
    grid_layout_t *row_layout;
    grid_area_opts_t *areas_layout;

    // Content view:
    void *data_source;
    data_get_amount_t data_get_amount;
    area_render_t data_render; // TODO:item_render?
}
grid_panel_opts_t;

panel_interface_t grid_panel_get_impl(void);


#endif/*_GRID_PANEL_H_*/
