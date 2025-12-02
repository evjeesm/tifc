#include "spidex.h"
#include "utils.h"

#include <assert.h>

static void axis_add(dynarr_t **const axis,
        const spidex_coord_t start,
        const spidex_coord_t end,
        const spidex_value_t value,
        const compare_t value_cmp);

static bool is_valid_area(const spidex_area_t *const area);

static ssize_t find_first_intersecting_range(dynarr_t *const axis, const spidex_interval_t interval);

static size_t find_insert_place(dynarr_t *const axis, const spidex_coord_t start);

static spidex_range_t *query_range(const dynarr_t *const axis, const spidex_coord_t coord);

static spidex_range_t new_range(const spidex_coord_t start, const spidex_coord_t end,
        const spidex_value_t value);

static spidex_range_t dup_range(const spidex_range_t *const range);

static spidex_status_t get_area_value(const spidex_range_t *const x, const spidex_range_t *const y,
        const compare_t value_cmp, spidex_value_t *const out_value);

static void add_value(spidex_range_t *const range, const spidex_value_t value, const compare_t value_cmp);

static void spidex_range_init(spidex_range_t *const range, spidex_coord_t start,
        spidex_coord_t end);

static ssize_t spidex_intersect_cmp(const void *const value,
        const void *const element,
        void *const param);

static ssize_t spidex_cmp(const void *const value,
        const void *const element,
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

    if (dynarr_size(spidex->x))
    {
        dynarr_transform(spidex->x, delete_ranges, NULL);
    }
    dynarr_destroy(spidex->x);

    if (dynarr_size(spidex->y))
    {
        dynarr_transform(spidex->y, delete_ranges, NULL);
    }
    dynarr_destroy(spidex->y);
}


bool spidex_is_empty(const spidex_t *const spidex)
{
    assert(spidex);
    return 0 == dynarr_size(spidex->x);
}


bool spidex_has_intersect(const spidex_t *const spidex, const spidex_area_t *const area)
{
    assert(spidex);
    assert(is_valid_area(area));

    const spidex_coord_t x_end = area->end.x;
    spidex_coord_t x_start = area->start.x;
    const size_t x_amount = dynarr_size(spidex->x);

    const ssize_t found_x_index = find_first_intersecting_range(spidex->x, (spidex_interval_t){ area->start.x, area->end.x });
    if (-1 == found_x_index) // no intersecting x-ranges
    {
        return false;
    }
    size_t x_index = found_x_index;

    const ssize_t found_y_index = find_first_intersecting_range(spidex->y, (spidex_interval_t){ area->start.y, area->end.y });
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

            if (SPIDEX_OK == get_area_value(x_range, y_range, spidex->value_cmp, &area_value))
            {
                return true;
            }

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
    assert(is_valid_area(area));

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


void spidex_remove(spidex_t *const spidex, const spidex_value_t value)
{
    assert(spidex);
    UNUSED(value);
}


spidex_status_t spidex_query(const spidex_t *const spidex, spidex_pos_t pos, spidex_value_t *const out_value)
{
    assert(spidex);
    assert(out_value);

    spidex_range_t *x_range = query_range(spidex->x, pos.x);
    if (!x_range)
    {
        return SPIDEX_NO_VALUE;
    }

    spidex_range_t *y_range = query_range(spidex->y, pos.y);
    if (!y_range)
    {
        return SPIDEX_NO_VALUE;
    }

    return get_area_value(x_range, y_range, spidex->value_cmp, out_value);
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
    const ssize_t found_index = find_first_intersecting_range(*axis, (spidex_interval_t){start, end});
    size_t index;
    ssize_t prev_end; // needed for detecting gaps

    if (-1 == found_index) // no intersection detected
    {
        index = find_insert_place(*axis, start);
    }
    else
    {
        index = found_index;
        spidex_range_t *found_range = dynarr_get(*axis, index);
        if (start < found_range->start && end > found_range->start) // right intersection
        {
            prev_end = start; // there is a gap to fill at the begining
        }
        else // left intersection or aligned
        {
            prev_end = found_range->start; // no gap to fill
        }
    }

    while (start < end)
    {
        if (index == dynarr_size(*axis))
        {
            // alloc rest
            spidex_range_t rest = new_range(start, end, value);
            (DISCARD) dynarr_insert(axis, index, &rest);
            start = rest.end;
            ++index;
            continue; // should exit a loop
        }

        spidex_range_t *cur_range = dynarr_get(*axis, index);
        if (prev_end < cur_range->start)
        {
            // fill the gap
            spidex_coord_t gap_end = (end < cur_range->start) ? end : cur_range->start;
            spidex_range_t gap = new_range(prev_end, gap_end, value);

            (DISCARD) dynarr_insert(axis, index, &gap);
            start = gap.end;
            prev_end = gap.end;
            ++index;
            continue;
        }

        if (start > cur_range->start)
        {
            // split by start
            spidex_range_t left_part = dup_range(cur_range); // left unmodified
            left_part.end = start;
            cur_range->start = start; // cur_range is right_part
            (DISCARD) dynarr_insert(axis, index, &left_part);
            ++index; // skip left_part
            prev_end = left_part.end;
        }
        else // start == cur_range->start
        {
            if (end < cur_range->end)
            {
                // split by end
                spidex_range_t right_part = dup_range(cur_range); // left unmodified
                add_value(cur_range, value, value_cmp); // modify cur_range
                cur_range->end = end; // trim
                right_part.start = end;
                (DISCARD) dynarr_insert(axis, index + 1, &right_part);
            }
            else // end >= cur_range->end
            {
                add_value(cur_range, value, value_cmp);
                ++index;
            }
            prev_end = cur_range->end;
            start = cur_range->end;
        }
    }
}


static bool is_valid_area(const spidex_area_t *const area)
{
    return area->start.x < area->end.x && area->start.y < area->end.y;
}


static ssize_t find_first_intersecting_range(dynarr_t *const axis, const spidex_interval_t interval)
{
    return dynarr_binary_find_index(axis, &interval, spidex_intersect_cmp, NULL);
}


// use it when you definitely know that there is no intersection
static size_t find_insert_place(dynarr_t *const axis, const spidex_coord_t start)
{
    return dynarr_binary_find_insert_place(axis, &start, spidex_cmp, NULL);
}


static spidex_range_t *query_range(const dynarr_t *const axis, const spidex_coord_t coord)
{
    return dynarr_binary_find(axis, &coord, spidex_cmp, NULL);
}


static spidex_range_t new_range(const spidex_coord_t start, const spidex_coord_t end,
                                const spidex_value_t value)
{
    assert(start < end);

    spidex_range_t range;
    spidex_range_init(&range, start, end);

    (DISCARD) dynarr_append(&range.values, &value);
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


static spidex_status_t get_area_value(const spidex_range_t *const x, const spidex_range_t *const y,
                           const compare_t value_cmp, spidex_value_t *const out_value)
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
            *out_value = *x_value;
            return SPIDEX_OK;
        }
    }

    return SPIDEX_NO_VALUE;
}


static void add_value(spidex_range_t *const range, const spidex_value_t value, const compare_t value_cmp)
{
    assert(range);
    assert(!dynarr_binary_find(range->values, &value, value_cmp, NULL)
        && "Should not add same value twice!");

    (DISCARD) dynarr_binary_insert(&range->values, &value, value_cmp, NULL, NULL);
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

    if (*coord < range->start)
    {
        return -1;
    }

    if (*coord >= range->end)
    {
        return  1;
    }

    return 0;
}

static ssize_t spidex_intersect_cmp(const void *const value,
        const void *const element,
        void *const param)
{
    const spidex_interval_t *interval = value;
    const spidex_range_t *range = element;
    UNUSED(param);

    if (interval->start < range->start) // interval shifted left from range start
    {
        if (interval->end > range->start) // interval intersects range from the left
        {
            return 0;
        }

        return -1;  // no intersection
    }
    else if (interval->start > range->start)   // interval shifted right from range start
    {
        if (interval->start < range->end) // interval intersects range from the right
        {
            return 0;
        }

        return 1; // no intersection
    }

    return 0; // interval aligned with range start (intersect)
}

static int delete_ranges(void *const range, void *const param)
{
    UNUSED(param);
    spidex_range_t *_range = range;
    dynarr_destroy(_range->values);
    return 0;
}
