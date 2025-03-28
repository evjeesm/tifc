#ifndef _GRID_H_
#define _GRID_H_

#include "dynarr.h"
#include "layout.h"
#include "logger.h"
#include "display.h"

#include <stdint.h>

#define MAX_COLUMNS 256
#define MAX_ROWS 256

typedef struct
{
    /* TODO consider arena to store layouts and spans */
    dynarr_t *layout;

    /* columns & rows spans recalculated on resize */
    dynarr_t *spans;

    /* Configured content areas.
        size will extend to `rows * columns` at max. */
    dynarr_t *areas;

    uint8_t columns;
    uint8_t rows;

    struct grid_area_t *last_hovered;
}
grid_t;

typedef struct
{
    uint16_t start;
    uint16_t end;
}
span_t;

typedef struct
{
    layout_size_method_t size_method;
    uint16_t             size;
    uint16_t             amount; /* amount of entries affected by this definition */
}
grid_layout_t;


#define IS_INVALID_SPAN(span_ptr) ((span_ptr)->start == (uint16_t) -1 \
                                  && (span_ptr)->end == (uint16_t) -1)
#define INVALID_SPAN ((span_t){-1, -1})

typedef struct
{
    span_t column;
    span_t row;
}
grid_area_opts_t;

typedef struct grid_area_t
{
    grid_area_opts_t grid_area_opts;
    disp_area_t area;
    bool is_hovered;
}
grid_area_t;

void grid_init(grid_t *const grid,
    uint8_t columns,
    uint8_t rows,
    grid_layout_t* column_layout,
    grid_layout_t* row_layout);

void grid_deinit(grid_t *const grid);


void grid_add_area(grid_t *const grid,
        const grid_area_opts_t *const span);

typedef void (*area_render_t) (display_t *const display,
        const grid_area_t *const area, const void *const source, const size_t limit, const size_t index);

void grid_render(const grid_t *const grid, display_t *const display,
        const void *const source, const size_t limit, const area_render_t render);

void grid_recalculate_layout(grid_t *const grid,
        const disp_area_t *const panel_area);

void grid_hover(grid_t *const grid, const disp_pos_t pos);

#endif// _GRID_H_
