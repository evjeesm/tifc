#ifndef _SPIDEX_H_
#define _SPIDEX_H_

#include "dynarr.h"

#include <stdbool.h>

#ifndef SPIDEX_COORDINATE_TYPE
#    define SPIDEX_COORDINATE_TYPE ssize_t
#endif
typedef SPIDEX_COORDINATE_TYPE spidex_coord_t;

typedef struct
{
    dynarr_t *x;
    dynarr_t *y;
    compare_t value_cmp;
}
spidex_t;

typedef struct
{
    spidex_coord_t start;
    spidex_coord_t end; /* excluding */
    dynarr_t *values;
}
spidex_range_t;

typedef struct
{
    spidex_coord_t start;
    spidex_coord_t end; /* excluding */
}
spidex_interval_t;

typedef struct
{
    spidex_coord_t x;
    spidex_coord_t y;
}
spidex_pos_t;

typedef struct
{
    spidex_pos_t start;
    spidex_pos_t end; /* excluding */
}
spidex_area_t;

typedef enum
{
    SPIDEX_OK = 0,
    SPIDEX_OVERLAP = 1,
    SPIDEX_NO_VALUE = 2,
}
spidex_status_t;

typedef void* spidex_value_t;

void spidex_init(spidex_t *const spidex, const compare_t value_cmp);
void spidex_deinit(spidex_t *const spidex);
bool spidex_is_empty(const spidex_t *const spidex);
bool spidex_has_intersect(const spidex_t *const spidex, const spidex_area_t *const area);

/* O(LogN + M)
 * N - amount of registered areas
 * M - amount of areas that intersect with new area on one of the axis
 *
 * Does not protect from registering same value several times.
 * Do prevents overlaps (in that case returns SPIDEX_OVERLAP status).
 * Returns SPIDEX_OK on success
 */
spidex_status_t spidex_add(spidex_t *const spidex, const spidex_area_t *const area, spidex_value_t value);

/* O(X + Y)
 *  X - amount of registered ranges at x-axis
 *  Y - amount of registered ranges at y-axis
 */
void spidex_remove(spidex_t *const spidex, const spidex_value_t value);
/* TODO: spidex_guided_remove */

/* TODO: spidex_find */

/* O(LogN + M)
 */
spidex_status_t spidex_query(const spidex_t *const spidex, spidex_pos_t pos, spidex_value_t *const out_value);


/* O(1)
 */
void spidex_reset(spidex_t *const spidex);


#endif/* _SPIDEX_H_ */
