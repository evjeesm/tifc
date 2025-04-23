#ifndef _COMPOSITE_INTERIOR_H_
#define _COMPOSITE_INTERIOR_H_

#include "interior.h"

typedef struct composite_interior composite_interior_t;

typedef struct
{
    size_t area_idx;
    interior_opts_t *opts;
}
component_def_t;


typedef struct
{
    interior_opts_t interior;
    size_t components_amount;
    component_def_t *component_defs;
}
composite_interior_opts_t;


interior_interface_t composite_interior_get_impl(void);

#endif/*_COMPOSITE_INTERIOR_H_ */
