#ifndef _VIEW_INTERIOR_H_
#define _VIEW_INTERIOR_H_

#include "interior.h"

typedef struct view_interior view_interior_t;

typedef void (*area_render_t)(display_t *const display,
        const interior_area_t *const area, const void *const source,
        const size_t limit, const size_t index, const bool hovered);

typedef size_t (*get_amount_t)(const void *const data);

typedef struct data_source
{
    void          *data;
    get_amount_t  get_amount;
    area_render_t render;
}
data_source_t;

typedef struct
{
    interior_opts_t interior;
    data_source_t   source;
}
view_interior_opts_t;

interior_interface_t view_interior_get_impl(void);

#endif/*_VIEW_INTERIOR_H_*/
