#include "grid.h"
#include "border.h"
#include "display.h"
#include "display_types.h"
#include "dynarr.h"
#include "layout.h"
#include "input.h"

#include <assert.h>

#define MIN_GRID_AREA_SIZE 1

void grid_init(grid_t *const grid,
    uint8_t columns,
    uint8_t rows,
    grid_layout_t* column_layout,
    grid_layout_t* row_layout)
{
    assert(grid);
    assert(columns > 0);
    assert(rows > 0);
    assert(column_layout);
    assert(row_layout);

    grid->columns = columns;
    grid->rows = rows;

    grid->layout = dynarr_create
        (
            .element_size = sizeof(grid_layout_t),
            .initial_cap = columns + rows,
        );

    grid->spans = dynarr_create
        (
            .element_size = sizeof(span_t),
            .initial_cap = columns + rows,
        );

    // TODO: should implement reserve range for dynarr
    dynarr_spread_insert(&grid->spans, 0, columns + rows, TMP_REF(span_t, 0));

    grid->areas = dynarr_create (
        .element_size = sizeof(grid_area_t),
    );

    uint8_t rept = 0;
    for (uint8_t c = 0; c < columns; ++c)
    {
        assert(0 < column_layout->amount);
        if (column_layout->amount == rept)
        {
            rept = 0;
            ++column_layout;
        }
        ++rept;
        dynarr_append(&grid->layout, column_layout);
    }

    rept = 0;
    for (uint8_t r = 0; r < rows; ++r)
    {
        assert(0 < row_layout->amount);
        if (row_layout->amount == rept)
        {
            rept = 0;
            ++row_layout;
        }
        ++rept;
        dynarr_append(&grid->layout, row_layout);
    }
}


void grid_deinit(grid_t *const grid)
{
    dynarr_destroy(grid->layout);
    dynarr_destroy(grid->spans);
    dynarr_destroy(grid->areas);
}


void grid_add_area(grid_t *const grid, const grid_area_opts_t *const opts)
{
    assert(grid);

    assert(opts->column.start <= opts->column.end);
    assert(opts->row.start <= opts->row.end);
    assert(opts->column.end < grid->columns);
    assert(opts->row.end < grid->rows);

    grid_area_t area = {
        .grid_area_opts = *opts,
        .area = INVALID_AREA,
    };
    // implement reserve for dynarr ?
    (void) dynarr_append(&grid->areas, &area);
}


void grid_render(const grid_t *const grid, display_t *const display,
        const void *const source, const size_t limit, const area_render_t render)
{
    assert(render);
    const size_t areas_amount = dynarr_size(grid->areas);
    for (size_t i = 0; i < areas_amount; ++i)
    {
        grid_area_t *area = dynarr_get(grid->areas, i);
        border_set_t border = {._ = L"╭╮╯╰┆┄"};
        (void) area;
        (void) border;
        if (IS_INVALID_AREA(&area->area)) { return; }
        render(display, area, source, limit, i + grid->scroll_offset);
        // display_draw_border(display, BORDER_STYLE_1, border, area->area);
    }
}

static grid_area_t *find_hovered_area(dynarr_t *const areas, const disp_pos_t pos)
{
    const size_t areas_amount = dynarr_size(areas);
    for (size_t i = 0; i < areas_amount; ++i)
    {
        grid_area_t *area = dynarr_get(areas, i);

        if (pos.x >= area->area.first.x && pos.x <= area->area.second.x
          && pos.y >= area->area.first.y && pos.y <= area->area.second.y)
        {
            return area;
        }
    }
    return NULL;
}


void grid_hover(grid_t *const grid, const disp_pos_t pos)
{
    grid_area_t *hovered = find_hovered_area(grid->areas, pos);

    if (grid->last_hovered && grid->last_hovered != hovered)
    {
        grid->last_hovered->is_hovered = false;
    }
    grid->last_hovered = hovered;

    if (!hovered) { return; }
    hovered->is_hovered = true;
    
}

static int valid_area_count(const void *const element, void *const param)
{
    size_t *count = param;
    const grid_area_t *area = element;

    if (! IS_INVALID_AREA(&area->area)) { ++*count; }

    return 0;
}

static size_t count_valid_areas(const dynarr_t *const areas)
{
    size_t count = 0;
    dynarr_foreach(areas, valid_area_count, &count);
    return count;
}

static void adjust_scroll_position(grid_t *const grid,
        const size_t limit)
{
    // CACHE? or maybe use last_hovered 
    const size_t valid_areas = count_valid_areas(grid->areas);
    const size_t max_scroll_offset = (limit <= valid_areas) ? 0 : limit - valid_areas;

    if (grid->scroll_offset >= max_scroll_offset)
    {
        grid->scroll_offset = max_scroll_offset;
        return;
    }
}

void grid_scroll(grid_t *const grid, const int direction, const size_t limit)
{
    if (!(grid->is_scrollable)) { return; }
    if (SCROLL_UP == direction)
    {
        if (grid->scroll_offset == 0) { return; }
        --grid->scroll_offset;
        return;
    }

    ++grid->scroll_offset;

    adjust_scroll_position(grid, limit);
}


static void calculate_spans(size_t start_offset, size_t length,
        const size_t spans_amount, const size_t offset,
        const dynarr_t *const grid_layout, dynarr_t *const grid_spans);

static void calc_areas(dynarr_t *const areas, span_t columns[], span_t rows[]);


void grid_recalculate_layout(grid_t *const grid, const disp_area_t *const panel_area)
{
    assert(grid);
    assert(panel_area);

    S_LOG(LOGGER_DEBUG, "Panel Area: [%u, %u][%u, %u]\n",
            panel_area->first.x, panel_area->first.y,
            panel_area->second.x, panel_area->second.y);

    const disp_pos_t panel_size = {
        panel_area->second.x - panel_area->first.x + 1,
        panel_area->second.y - panel_area->first.y + 1,
    };

    /* subtracting panel's border */
    const ssize_t width = panel_size.x - 2;
    const ssize_t height = panel_size.y - 2;
    assert(width >= 0);
    assert(height >= 0);

    S_LOG(LOGGER_DEBUG,
        "\ngrid_recalculate_layout"
        "\n==================\n");

    S_LOG(LOGGER_DEBUG, "Calculate columns:\n");
    calculate_spans(panel_area->first.x + 1, width, grid->columns,
            0 /* columns offset */,
            grid->layout, grid->spans);

    S_LOG(LOGGER_DEBUG, "Calculate rows:\n");
    calculate_spans(panel_area->first.y + 1, height, grid->rows,
            grid->columns /* rows offset */,
            grid->layout, grid->spans);

    S_LOG(LOGGER_DEBUG,
        "\nAreas"
        "\n==================\n");

    calc_areas(grid->areas,
        dynarr_get(grid->spans, 0),
        dynarr_get(grid->spans, grid->columns));

    adjust_scroll_position(grid, 0);
}


static void calculate_spans(size_t start_offset, size_t length,
        const size_t spans_amount, const size_t offset,
        const dynarr_t *const grid_layout, dynarr_t *const grid_spans)
{
    // access layout
    const grid_layout_t *layout = dynarr_get(grid_layout, offset);

    // access span
    span_t *span = dynarr_get(grid_spans, offset);
    size_t size;

    for (size_t i = 0; i < spans_amount; ++i)
    {
        if (0 == length)
        {
            *span = INVALID_SPAN;
            S_LOG(LOGGER_DEBUG, "Invalid Span %d = {%u, %u}\n", i, span->start, span->end);
            ++span;
            continue;
        }

        span->start = start_offset;

        size = layout->size;
        if (LAYOUT_SIZE_RELATIVE == layout->size_method)
        {
            size = layout->size * length / 100;
            if (size < MIN_GRID_AREA_SIZE) size = MIN_GRID_AREA_SIZE;
        }

        // if no space left for other areas, then consume rest of the panel
        if (length < size || (length - size < MIN_GRID_AREA_SIZE)) size = length;

        span->end = span->start + size - 1;
        start_offset += size;
        length -= size;

        S_LOG(LOGGER_DEBUG, "Span %d = {%u, %u}\n", i, span->start, span->end);
        ++layout;
        ++span;
    }
}


static void calc_areas(dynarr_t *const areas, span_t columns[], span_t rows[])
{
    const size_t areas_amount = dynarr_size(areas);

    for (size_t i = 0; i < areas_amount; ++i)
    {
        grid_area_t *area = dynarr_get(areas, i);
        const span_t *start_column = &columns[area->grid_area_opts.column.start];
        const span_t *start_row = &rows[area->grid_area_opts.row.start];

        if (IS_INVALID_SPAN(start_column) || IS_INVALID_SPAN(start_row))
        {
            S_LOG(LOGGER_DEBUG, "Invalid Area %d\n", i);
            area->area = INVALID_AREA;
            continue;
        }

        const size_t c_range = area->grid_area_opts.column.end - area->grid_area_opts.column.start;
        const span_t *end_column = start_column;
        for (size_t i = 0; !IS_INVALID_SPAN(end_column) && i < c_range; ++end_column, ++i);

        const size_t r_range = area->grid_area_opts.row.end - area->grid_area_opts.row.start;
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
            area->area.first.x,
            area->area.first.y,
            area->area.second.x,
            area->area.second.x
        );
    }
}


