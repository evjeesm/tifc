#ifndef _PANEL_H_
#define _PANEL_H_

#include "display.h"
#include "grid.h"
#include "layout.h"

typedef struct
{
    layout_align_t       align;
    layout_size_method_t size_method;
    disp_pos_t           size;
}
panel_layout_t;

typedef enum
{
    PANEL_CONTENT_TYPE_RAW = 0, // custom rendering
    PANEL_CONTENT_TYPE_GRID     // table like rendering
}
panel_content_type_t;

typedef size_t (*data_get_amount_t) (const void *const data_source);

typedef struct
{
    const char    * title;
    unsigned int    title_size;
    panel_layout_t  layout;
    style_t         style;
    disp_area_t     area;

    void *data_source;
    data_get_amount_t data_get_amount;
    area_render_t data_render;

    panel_content_type_t content_type;
    union content {
        grid_t      grid;
    }
    content;
}
panel_t;


typedef struct
{
    const char     * title;
    panel_layout_t   layout;

    void *data_source;
    data_get_amount_t data_get_amount;
    area_render_t data_render;
    bool scrollable;

    // Specific to grid content type:
    //  (columns == 0 && rows == 0) means raw content type
    uint8_t columns;
    uint8_t rows;
    uint16_t areas;

    grid_layout_t *column_layout;
    grid_layout_t *row_layout;
    grid_area_opts_t *areas_layout;
}
panel_opts_t;

void panel_init(panel_t *const panel, const panel_opts_t *const opts);

void panel_deinit(panel_t *const panel);

void panel_render(const panel_t *panel, display_t *const display);

void panel_recalculate_layout(panel_t *panel,
                              disp_area_t *const bounds);

void panel_hover(panel_t *const panel, const disp_pos_t pos);

void panel_scroll(panel_t *const panel, const int direction);

void panel_set_data_source(panel_t *const panel, void *data_source, area_render_t data_render);

#endif // _PANEL_H_
