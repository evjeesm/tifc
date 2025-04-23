#include "interior_layout.h"
#include "display_types.h"
#include "dynarr.h"
#include "logger.h"

#include <assert.h>
#include <string.h>

#define MIN_LAYOUT_AREA_SIZE 1

static void unwrap_opts(dynarr_t **const layout, counted_layout_def_t *defs, const size_t amount);

static void calculate_spans(size_t start_offset, size_t length,
        const size_t spans_amount, const size_t offset,
        const dynarr_t *const layout, dynarr_t *const spans);

static void calc_areas(dynarr_t *const areas, span_t columns[], span_t rows[]);
static int valid_area_count(const void *const element, void *const param);


void interior_layout_init(interior_layout_t *const layout,
        const interior_layout_opts_t *const opts)
{
    assert(layout);
    assert(opts->columns > 0);
    assert(opts->rows > 0);
    assert(opts->columns_def);
    assert(opts->rows_def);

    *layout = (interior_layout_t) {
        .columns = opts->columns,
        .rows = opts->rows,
        .layout = dynarr_create(
            .element_size = sizeof(layout_def_t),
            .initial_cap = opts->columns + opts->rows,
        ),
        .spans = dynarr_create(
            .element_size = sizeof(span_t),
            .initial_cap = opts->columns + opts->rows,
        ),
        .areas = dynarr_create(
            .element_size = sizeof(interior_area_t),
        ),
        .padding = opts->padding,
    };

    // TODO: should implement reserve range for dynarr
    dynarr_spread_insert(&layout->spans, 0,
            opts->columns + opts->rows, TMP_REF(span_t, 0));

    unwrap_opts(&layout->layout, opts->columns_def, opts->columns);
    unwrap_opts(&layout->layout, opts->rows_def, opts->rows);

    // add areas
   for (size_t ai = 0; ai < opts->areas; ++ai)
   {
        interior_layout_add_area(layout, &opts->areas_def[ai]);
   }
}


void interior_layout_deinit(interior_layout_t *const layout)
{
    assert(layout);
    dynarr_destroy(layout->layout);
    dynarr_destroy(layout->spans);
    dynarr_destroy(layout->areas);
}


void interior_layout_add_area(interior_layout_t *const layout,
        const interior_area_def_t *const opts)
{
    assert(layout);

    assert(opts->column.start <= opts->column.end);
    assert(opts->row.start <= opts->row.end);
    assert(opts->column.end < layout->columns);
    assert(opts->row.end < layout->rows);

    interior_area_t area = {
        .def = *opts,
        .area = INVALID_AREA,
    };
    // implement reserve for dynarr ?
    (void) dynarr_append(&layout->areas, &area);
}


void interior_layout_recalculate(interior_layout_t *const layout,
        const disp_area_t *const panel_area)
{
    assert(layout);
    assert(panel_area);

    S_LOG(LOGGER_DEBUG, "Panel Area: [%u, %u][%u, %u]\n",
            panel_area->first.x, panel_area->first.y,
            panel_area->second.x, panel_area->second.y);

    const disp_pos_t panel_size = {
        panel_area->second.x - panel_area->first.x + 1,
        panel_area->second.y - panel_area->first.y + 1,
    };

    /* subtracting panel's border */
    const ssize_t width = panel_size.x - layout->padding.left - layout->padding.right;
    const ssize_t height = panel_size.y - layout->padding.top - layout->padding.bot;
    assert(width >= 0);
    assert(height >= 0);

    S_LOG(LOGGER_DEBUG,
        "\nrecalculate_layout"
        "\n==================\n");

    S_LOG(LOGGER_DEBUG, "Calculate columns:\n");
    calculate_spans(panel_area->first.x + layout->padding.left, width, layout->columns,
            0 /* columns offset */,
            layout->layout, layout->spans);

    S_LOG(LOGGER_DEBUG, "Calculate rows:\n");
    calculate_spans(panel_area->first.y + layout->padding.right, height, layout->rows,
            layout->columns /* rows offset */,
            layout->layout, layout->spans);

    S_LOG(LOGGER_DEBUG,
        "\nAreas"
        "\n==================\n");

    calc_areas(layout->areas,
        dynarr_get(layout->spans, 0),
        dynarr_get(layout->spans, layout->columns));
}


size_t interior_layout_count_valid_areas(const interior_layout_t * const layout)
{
    size_t count = 0;
    dynarr_foreach(layout->areas, valid_area_count, &count);
    return count;
}


interior_area_t *interior_layout_peek_area(const interior_layout_t *const layout, const disp_pos_t pos)
{
    if (IS_NO_LAYOUT(layout)) { return NULL; }

    const size_t areas_amount = dynarr_size(layout->areas);
    for (size_t ai = 0; ai < areas_amount; ++ai)
    {
        interior_area_t *area = dynarr_get(layout->areas, ai);

        if (pos.x >= area->area.first.x && pos.x <= area->area.second.x
          && pos.y >= area->area.first.y && pos.y <= area->area.second.y)
        {
            return area;
        }
    }
    return NULL;
}


ssize_t interior_layout_peek_area_index(const interior_layout_t *const layout, const disp_pos_t pos)
{
    const size_t areas_amount = dynarr_size(layout->areas);
    for (size_t ai = 0; ai < areas_amount; ++ai)
    {
        interior_area_t *area = dynarr_get(layout->areas, ai);

        if (pos.x >= area->area.first.x && pos.x <= area->area.second.x
          && pos.y >= area->area.first.y && pos.y <= area->area.second.y)
        {
            return ai;
        }
    }
    return -1;
}


bool interior_area_is_visible(const interior_area_t *const area)
{
    return ! IS_INVALID_AREA(&area->area);
}


static void unwrap_opts(dynarr_t **const layout, counted_layout_def_t *defs, const size_t amount)
{
    size_t rept = 0;
    for (size_t c = 0; c < amount; ++c)
    {
        assert(0 < defs->amount);
        if (defs->amount == rept)
        {
            rept = 0;
            ++defs;
        }
        ++rept;
        dynarr_append(layout, &defs->layout);
    }
}


static void calculate_spans(size_t start_offset, size_t length,
        const size_t spans_amount, const size_t offset,
        const dynarr_t *const layout_defs, dynarr_t *const spans)
{
    // access layout
    const layout_def_t *layout_def = dynarr_get(layout_defs, offset);

    // access span
    span_t *span = dynarr_get(spans, offset);
    size_t size;

    for (size_t i = 0; i < spans_amount; ++i)
    {
        if (0 == length)
        {
            *span = INVALID_SPAN;
            S_LOG(LOGGER_DEBUG, "Invalid Span %d = {%u, %u}\n",
                    i, span->start, span->end);
            ++span;
            continue;
        }

        span->start = start_offset;

        size = layout_def->size;
        if (LAYOUT_SIZE_RELATIVE == layout_def->size_method)
        {
            size = layout_def->size * length / 100;
            if (size < MIN_LAYOUT_AREA_SIZE) size = MIN_LAYOUT_AREA_SIZE;
        }

        /* If no space left for other areas, consume rest of the panel. */
        if (length < size || (length - size < MIN_LAYOUT_AREA_SIZE))
        {
            size = length;
        }

        span->end = span->start + size - 1;
        start_offset += size;
        length -= size;

        S_LOG(LOGGER_DEBUG, "Span %d = {%u, %u}\n",
                i, span->start, span->end);

        ++layout_def;
        ++span;
    }
}


static void calc_areas(dynarr_t *const areas, span_t columns[], span_t rows[])
{
    const size_t areas_amount = dynarr_size(areas);

    for (size_t i = 0; i < areas_amount; ++i)
    {
        interior_area_t *area = dynarr_get(areas, i);
        const span_t *start_column = &columns[area->def.column.start];
        const span_t *start_row = &rows[area->def.row.start];

        if (IS_INVALID_SPAN(start_column) || IS_INVALID_SPAN(start_row))
        {
            S_LOG(LOGGER_DEBUG, "Invalid Area %d\n", i);
            area->area = INVALID_AREA;
            continue;
        }

        const size_t c_range = area->def.column.end - area->def.column.start;
        const span_t *end_column = start_column;
        for (size_t i = 0; !IS_INVALID_SPAN(end_column) && i < c_range; ++end_column, ++i);

        const size_t r_range = area->def.row.end - area->def.row.start;
        const span_t *end_row = start_row;
        for (size_t i = 0; !IS_INVALID_SPAN(end_row) && i < r_range; ++end_row, ++i);

        /* otherwise */
        area->area = (disp_area_t){
            .first = {
                .x = start_column->start,
                .y = start_row->start,
            },
            .second = {
                .x = end_column->end,
                .y = end_row->end,
            },
        };
        S_LOG(LOGGER_DEBUG, "Area %d = {%u, %u, %u, %u}\n", i,
            area->area.first.x, area->area.first.y,
            area->area.second.x, area->area.second.x
        );
    }
}


/*
* 'count_valid_areas' helper
* 'dynarr_foreach' callback
*/
static int valid_area_count(const void *const element, void *const param)
{
    size_t *count = param;
    const interior_area_t *area = element;

    if (! IS_INVALID_AREA(&area->area)) { ++*count; }

    return 0;
}
