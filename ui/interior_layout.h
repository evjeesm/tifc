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
interior_area_def_t;


#define IS_INVALID_SPAN(span_ptr) ((span_ptr)->start == (uint16_t) -1 \
                                  && (span_ptr)->end == (uint16_t) -1)
#define INVALID_SPAN ((span_t){-1, -1})


typedef struct
{
    layout_size_method_t size_method;
    uint16_t size;
}
layout_def_t;


typedef struct
{
    layout_def_t layout;
    uint16_t     amount; /* amount of entries affected by this definition */
}
counted_layout_def_t;


typedef struct
{
    size_t columns;
    size_t rows;
    size_t areas;
    counted_layout_def_t *columns_def;
    counted_layout_def_t *rows_def;
    interior_area_def_t  *areas_def;
}
interior_layout_opts_t;


struct interior_area
{
    interior_area_def_t def;
    disp_area_t area;
};


void interior_layout_init(interior_layout_t *const layout,
        const interior_layout_opts_t *const opts);

void interior_layout_deinit(interior_layout_t *const layout);

void interior_layout_add_area(interior_layout_t *const layout,
        const interior_area_def_t *const opts);

void interior_layout_recalculate(interior_layout_t *const layout,
        const disp_area_t *const panel_area);

size_t interior_layout_count_valid_areas(const interior_layout_t *const layout);

interior_area_t *interior_layout_peek_area(const interior_layout_t *const layout, const disp_pos_t pos);

bool interior_area_is_visible(const interior_area_t *const area);

#endif/*_INTERIOR_LAYOUT_H_*/
