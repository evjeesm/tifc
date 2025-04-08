#ifndef _INTERIOR_LAYOUT_H_
#define _INTERIOR_LAYOUT_H_

#include "display_types.h"
#include "dynarr.h"
#include "layout.h"

#include <stdint.h>

#define MAX_COLUMNS 256
#define MAX_ROWS 256

typedef struct interior_area interior_area_t;

typedef struct interior_layout
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
}
interior_layout_t;


typedef struct
{
    uint16_t start;
    uint16_t end;
}
span_t;


typedef struct
{
    span_t column;
    span_t row;
}
interior_area_opts_t;


#define IS_INVALID_SPAN(span_ptr) ((span_ptr)->start == (uint16_t) -1 \
                                  && (span_ptr)->end == (uint16_t) -1)
#define INVALID_SPAN ((span_t){-1, -1})

typedef struct
{
    layout_size_method_t size_method;
    uint16_t size;
    uint16_t amount; /* amount of entries affected by this definition */
}
layout_def_t;


struct interior_area
{
    interior_area_opts_t opts;
    disp_area_t area;

    bool is_hovered;
};

void interior_layout_init(interior_layout_t *const layout,
        uint8_t columns, layout_def_t* column_layout,
        uint8_t rows, layout_def_t* row_layout);

void interior_layout_deinit(interior_layout_t *const layout);

void interior_layout_add_area(interior_layout_t *const layout,
        const interior_area_opts_t *const opts);

void interior_layout_recalculate(interior_layout_t *const layout,
        const disp_area_t *const panel_area);

#endif/*_INTERIOR_LAYOUT_H_*/
