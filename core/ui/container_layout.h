#ifndef _CONTAINER_LAYOUT_H_
#define _CONTAINER_LAYOUT_H_

#include "display_types.h"
#include "dynarr.h"

#include <stdint.h>

#define MAX_COLUMNS 256
#define MAX_ROWS 256

typedef struct container_area container_area_t;

typedef enum
{
    LAYOUT_SIZE_FIXED = 0, /* fixed amount */
    LAYOUT_SIZE_RELATIVE   /* percents from free space left */
}
layout_size_method_t;

typedef struct
{
    uint8_t left;
    uint8_t top;
    uint8_t right;
    uint8_t bot;
}
padding_t;


typedef struct container_layout
{
    /* TODO consider arena to store layouts and spans */
    dynarr_t *layout;

    /* columns & rows spans recalculated on resize */
    dynarr_t *spans;

    /* Configured content areas.
        size will extend to `rows * columns` at max. */
    dynarr_t *areas;

    uint16_t columns;
    uint16_t rows;

    padding_t padding;
}
container_layout_t;


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
container_area_def_t;


#define IS_INVALID_SPAN(span_ptr) ((span_ptr)->start == (uint16_t) -1 \
                                  && (span_ptr)->end == (uint16_t) -1)
#define INVALID_SPAN ((span_t){-1, -1})

#define NO_LAYOUT_OPT ((container_layout_opts_t){0})
#define IS_NO_LAYOUT_OPT(layout_opts_p) (0 == memcmp(&NO_LAYOUT_OPT, layout_opts_p, sizeof(NO_LAYOUT_OPT)))

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
    container_area_def_t  *areas_def;
    padding_t padding;
}
container_layout_opts_t;


struct container_area
{
    container_area_def_t def;
    disp_area_t area;
};


void container_layout_init(container_layout_t *const layout,
        const container_layout_opts_t *const opts);

void container_layout_deinit(container_layout_t *const layout);

void container_layout_add_area(container_layout_t *const layout,
        const container_area_def_t *const opts);

void container_layout_recalculate(container_layout_t *const layout,
        const disp_area_t *const ui_element_area);

size_t container_layout_count_valid_areas(const container_layout_t *const layout);

container_area_t *container_layout_peek_area(const container_layout_t *const layout, const disp_pos_t pos);

ssize_t container_layout_peek_area_index(const container_layout_t *const layout, const disp_pos_t pos);

bool container_area_is_visible(const container_area_t *const area);

#endif/*_CONTAINER_LAYOUT_H_*/
