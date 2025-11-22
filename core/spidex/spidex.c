#include "spidex.h"
#include "utils.h"

#include <assert.h>

static void axis_add(dynarr_t **axis, const spidex_coord_t start,
                     const spidex_coord_t end,
                     const spidex_value_t value,
                     const compare_t value_cmp);

static spidex_range_t new_range(const spidex_coord_t start, const spidex_coord_t end,
                                const spidex_value_t value);

static spidex_range_t dup_range(const spidex_range_t *const range);

static bool get_area_value(const spidex_range_t *const x, const spidex_range_t *const y,
                           const compare_t value_cmp, spidex_value_t *const value_out);

static void add_value(spidex_range_t *const range, const spidex_value_t value, const compare_t value_cmp);

static void spidex_range_init(spidex_range_t *const range, spidex_coord_t start,
                              spidex_coord_t end);

static ssize_t spidex_cmp(const void *const value, const void *const element,
                          void *const param);

static int delete_ranges(void *const range, void *const param);


void spidex_init(spidex_t *const spidex, const compare_t value_cmp)
{
    assert(spidex);

    spidex->x = dynarr_create(.element_size = sizeof(spidex_range_t));
    assert(spidex->x && "Unable to allocate 'x'");

    spidex->y = dynarr_create(.element_size = sizeof(spidex_range_t));
    assert(spidex->y && "Unable to allocate 'y'");

    spidex->value_cmp = value_cmp;
    assert(spidex->value_cmp && "Has to have a valid compare callback!");
}


void spidex_deinit(spidex_t *const spidex)
{
    assert(spidex);

    dynarr_transform(spidex->x, delete_ranges, NULL);
    dynarr_destroy(spidex->x);

    dynarr_transform(spidex->y, delete_ranges, NULL);
    dynarr_destroy(spidex->y);
}


bool spidex_is_empty(const spidex_t *const spidex)
{
    assert(spidex);
    return 0 == dynarr_size(spidex->x);
}


bool spidex_has_intersect(const spidex_t *const spidex, const spidex_area_t *const area)
{
    const spidex_coord_t x_end = area->end.x;
    spidex_coord_t x_start = area->start.x;
    const size_t x_amount = dynarr_size(spidex->x);

    const ssize_t found_x_index = dynarr_binary_find_index(spidex->x, &x_start, spidex_cmp, NULL);
    if (-1 == found_x_index) // no intersecting x-ranges
    {
        return false;
    }
    size_t x_index = found_x_index;

    const ssize_t found_y_index = dynarr_binary_find_index(spidex->y, &area->start.y, spidex_cmp, NULL);
    if (-1 == found_y_index) // no intersecting y-ranges
    {
        return false;
    }
    const size_t y_begin_index = found_y_index;

    while (x_start < x_end) // X
    {
        const spidex_coord_t y_end = area->end.y;

        if (x_index == x_amount) break;
        spidex_range_t *x_range = dynarr_get(spidex->x, x_index);

        spidex_coord_t y_start = area->start.y;
        const size_t y_amount = dynarr_size(spidex->y);
        size_t y_index = y_begin_index;

        while (y_start < y_end) // Y
        {
            if (y_index == y_amount) break;

            spidex_range_t *y_range = dynarr_get(spidex->y, y_index);
            spidex_value_t area_value;

            bool has_area_value = get_area_value(x_range, y_range, spidex->value_cmp, &area_value);
            if (has_area_value) return true;

            y_start = y_range->end;
            ++y_index;
        }

        x_start = x_range->end;
        ++x_index;
    }

    return false;
}


spidex_status_t spidex_add(spidex_t *const spidex,
                           const spidex_area_t *const area,
                           spidex_value_t value)
{
    assert(spidex);
    assert(spidex->value_cmp);

    if (spidex_has_intersect(spidex, area))
    {
        return SPIDEX_OVERLAP;
    }

    // x
    axis_add(&spidex->x, area->start.x, area->end.x, value, spidex->value_cmp);

    // y
    axis_add(&spidex->y, area->start.y, area->end.y, value, spidex->value_cmp);

    return SPIDEX_OK;
}


void spidex_remove(spidex_t *const spidex, spidex_value_t value)
{
    assert(spidex);
    UNUSED(value);
}


spidex_value_t spidex_query(const spidex_t *const spidex, spidex_pos_t pos)
{
    assert(spidex);
    UNUSED(pos);
    return 0;
}


void spidex_reset(spidex_t *const spidex)
{
    assert(spidex);
    assert(spidex->x);
    assert(spidex->y);

    dynarr_clear(spidex->x);
    dynarr_clear(spidex->y);
}


static void axis_add(dynarr_t **axis,
                     spidex_coord_t start, const spidex_coord_t end,
                     const spidex_value_t value, const compare_t value_cmp)
{
    const ssize_t found_x_index = dynarr_binary_find_index(*axis, &start, spidex_cmp, NULL);
    size_t index = (-1 == found_x_index) ? 0 : found_x_index;
    ssize_t prev_end = -1ul;

    while (start < end)
    {
        if (index == dynarr_size(*axis))
        {
            // alloc rest
            spidex_range_t rest = new_range(start, end, value);
            (void) dynarr_insert(axis, index, &rest);
            start = rest.end;
            index += 1;
            continue;
        }

        spidex_range_t *cur_range = dynarr_get(*axis, index);
        if (cur_range->start > prev_end)
        {
            // fill the gap
            spidex_coord_t gap_end = (end < cur_range->start) ? end : cur_range->start;
            spidex_range_t gap = new_range(prev_end, gap_end, value);

            (void) dynarr_insert(axis, index, &gap);
            start = gap.end;
            index += 1;
            continue;
        }

        if (cur_range->start < start)
        {
            // split by start
            spidex_range_t split = dup_range(cur_range);
            add_value(&split, value, value_cmp);
            cur_range->end = start;
            split.start = start;
            (void) dynarr_insert(axis, index + 1, &split);

            index += 1;
        }
        else // cur_range->start == start
        {
            if (cur_range->end > end)
            {
                // split by end
                spidex_range_t split = dup_range(cur_range);
                add_value(cur_range, value, value_cmp);
                cur_range->end = end;
                split.start = end;
                (void) dynarr_insert(axis, index + 1, &split);
                index += 1;
            }
            else // cur_range->end <= end
            {
                add_value(cur_range, value, value_cmp);
            }
            prev_end = cur_range->end;
            start = cur_range->end;
        }
    }
}


static spidex_range_t new_range(const spidex_coord_t start, const spidex_coord_t end,
                                const spidex_value_t value)
{
    spidex_range_t range;
    spidex_range_init(&range, start, end);

    (void) dynarr_append(&range.values, &value);
    return range;
}


static spidex_range_t dup_range(const spidex_range_t *const range)
{
    assert(range);

    spidex_range_t dup = *range;

    dup.values = dynarr_clone(range->values);
    assert(dup.values);

    return dup;
}


static bool get_area_value(const spidex_range_t *const x, const spidex_range_t *const y,
                           const compare_t value_cmp, spidex_value_t *const value_out)
{
    const size_t x_size = dynarr_size(x->values);
    const size_t y_size = dynarr_size(y->values);

    size_t x_index = 0;
    size_t y_index = 0;

    while (x_index < x_size && y_index < y_size)
    {
        spidex_value_t *x_value = dynarr_get(x->values, x_index);
        spidex_value_t *y_value = dynarr_get(y->values, y_index);
        ssize_t cmp_result = value_cmp(y_value, x_value, NULL);
        if (0 > cmp_result)
        {
            ++y_index;
        }
        else if (0 < cmp_result)
        {
            ++x_index;
        }
        else
        {
            *value_out = *x_value;
            return true;
        }
    }

    return false;
}


static void add_value(spidex_range_t *const range, const spidex_value_t value, const compare_t value_cmp)
{
    assert(range);
    assert(!dynarr_binary_find(range->values, &value, value_cmp, NULL)
        && "Should not add same value twise!");

    (void) dynarr_binary_insert(&range->values, &value, value_cmp, NULL, NULL);
}


static void spidex_range_init(spidex_range_t *const range, spidex_coord_t start,
                              spidex_coord_t end)
{
    assert(range);
    assert(start < end);

    range->values = dynarr_create(.element_size = sizeof(spidex_value_t));
    assert(range->values && "Failed to allocate 'range->values'!");

    range->start = start;
    range->end = end;
}


static ssize_t spidex_cmp(const void *const value, const void *const element,
                          void *const param)
{
    const spidex_coord_t *coord = value;
    const spidex_range_t *range = element;
    UNUSED(param);

    if ((*coord - range->start) < 0)
    {
        return -1;
    }

    if ((*coord - range->end) > 0)
    {
        return  1;
    }

    return 0;
}


static int delete_ranges(void *const range, void *const param)
{
    UNUSED(param);
    spidex_range_t *_range = range;
    dynarr_destroy(_range->values);
    return 0;
}
