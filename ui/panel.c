#include "panel.h"
#include "display.h"
#include "grid.h"
#include "layout.h"

#include <assert.h>
#include <string.h>

#define MIN_PANEL_SIZE 2

static void
centralize_vertical(unsigned int vertical_size,
                    unsigned int vmax,
                    disp_area_t *panel_area,
                    disp_area_t *bounds);
static void
centralize_horizontal(unsigned int horizontal_size,
                      unsigned int hmax,
                      disp_area_t *panel_area,
                      disp_area_t *bounds);
static void
dock_to_top(unsigned int vertical_size,
            unsigned int vmax,
            disp_area_t *panel_area,
            disp_area_t *bounds);
static void
dock_to_bot(unsigned int vertical_size,
            unsigned int vmax,
            disp_area_t *panel_area,
            disp_area_t *bounds);
static void
dock_to_left(unsigned int horizontal_size,
             unsigned int hmax,
             disp_area_t *panel_area,
             disp_area_t *bounds);
static void
dock_to_right(unsigned int horizontal_size,
              unsigned int hmax,
              disp_area_t *panel_area,
              disp_area_t *bounds);
static void
fill_vertical(disp_area_t *panel_area,
              disp_area_t *bounds);
static void
fill_horizontal(disp_area_t *panel_area,
                disp_area_t *bounds);

static disp_area_t
calc_panel_area(const panel_layout_t *const layout,
                disp_area_t *const bounds);
static void
panel_draw_title(const panel_t *panel,
                 display_t *const display)
{
    disp_area_t title_area = panel->area;
    title_area.second.y = title_area.first.y;
    display_draw_string_centered(display, panel->title_size, panel->title, title_area, panel->style);
}


void panel_init(panel_t *const panel,
                const panel_opts_t *const opts)
{
    assert(panel);
    assert(opts);
    assert((opts->columns == 0 && opts->rows == 0)   // both zero or nether one is
        || (opts->columns != 0 && opts->rows != 0));

    *panel = (panel_t){
        .title = opts->title,
        .title_size = strlen(opts->title),
        .layout = opts->layout,
        .area = INVALID_AREA,
        .content_type = (opts->columns == 0 && opts->rows == 0)
            ? PANEL_CONTENT_TYPE_RAW
            : PANEL_CONTENT_TYPE_GRID,
        .data_source = opts->data_source,
        .data_get_amount = opts->data_get_amount,
        .data_render = opts->data_render,
    };

    if (PANEL_CONTENT_TYPE_RAW == panel->content_type)
    {
        // TODO: allocate raw panel
        return;
    }

    // GRID CONTENT:
    grid_init(&panel->content.grid,
        opts->columns,
        opts->rows,
        opts->column_layout,
        opts->row_layout);
    
    // set grid to scollable 
    panel->content.grid.is_scrollable = opts->scrollable;

    for (size_t a = 0; a < opts->areas; ++a)
    {
        grid_add_area(&panel->content.grid, &opts->areas_layout[a]);
    }
}


void panel_deinit(panel_t *const panel)
{
    assert(panel);
    if (PANEL_CONTENT_TYPE_RAW == panel->content_type)
    {
        // TODO: deallocate raw panel
        return;
    }
    grid_deinit(&panel->content.grid);
}


void panel_recalculate_layout(panel_t *panel,
                              disp_area_t *const bounds)
{
    panel->area = calc_panel_area(&panel->layout, bounds);

    if (IS_INVALID_AREA(&panel->area)) return;

    if (PANEL_CONTENT_TYPE_RAW == panel->content_type)
    {
        // recalculate for raw
        return;
    }

    // GRID:
    grid_recalculate_layout(&panel->content.grid, &panel->area); // TODO
}


void panel_render(const panel_t *panel,
                  display_t *const display)
{
    assert(panel);

    // dont render panel if has no valid area
    if (IS_INVALID_AREA(&panel->area)) return;

    disp_area_t panel_area = panel->area;
    border_set_t border = {._ = L"╭╮╯╰│─"};
    display_draw_border(display, panel->style, border, panel_area);
    // display_fill_area(display, panel->style, panel_area);
    panel_draw_title(panel, display);

    // render temporary TODO
    if (PANEL_CONTENT_TYPE_RAW == panel->content_type)
    {
        // TODO: custom render
        return;
    }
    size_t amount = panel->data_get_amount(panel->data_source);
    grid_render(&panel->content.grid, display, panel->data_source, amount, panel->data_render);
}


void panel_hover(panel_t *const panel, const disp_pos_t pos)
{
    if (panel->content_type == PANEL_CONTENT_TYPE_RAW)
    {
        return;
    }

    grid_hover(&panel->content.grid, pos);
}


void panel_scroll(panel_t *const panel, const disp_pos_t pos, const int direction)
{
    if (panel->content_type == PANEL_CONTENT_TYPE_RAW)
    {
        return;
    }

    size_t amount = panel->data_get_amount(panel->data_source);
    grid_scroll(&panel->content.grid, pos, direction, amount);
}


void panel_set_data_source(panel_t *const panel, void *data_source, area_render_t data_render)
{
    assert(panel);
    panel->data_source = data_source;
    panel->data_render = data_render;
}


static disp_area_t
calc_panel_area(const panel_layout_t *const layout,
                disp_area_t *const bounds)
{
    unsigned int horizontal_size = bounds->second.x - bounds->first.x + 1;
    unsigned int vertical_size = bounds->second.y - bounds->first.y + 1;
    unsigned int width = layout->size.x;
    unsigned int height = layout->size.y;

    if (layout->size_method == LAYOUT_SIZE_RELATIVE)
    {
        assert(width <= 100); // percent
        assert(height <= 100);
        width = horizontal_size * width / 100;
        height = vertical_size * height / 100;

        if (width < MIN_PANEL_SIZE) width = MIN_PANEL_SIZE;
        if (height < MIN_PANEL_SIZE) height = MIN_PANEL_SIZE;
    }

    if (vertical_size - height < MIN_PANEL_SIZE) height = vertical_size;
    if (horizontal_size - width < MIN_PANEL_SIZE) width = horizontal_size;

    // panel should not be rendered
    if (horizontal_size == 0 || vertical_size == 0)
    {
        return INVALID_AREA;
    }

    disp_area_t panel_area = {0};

    if (LAYOUT_ALIGN_CENTER == layout->align
        || 0 == layout->align)
    {
        centralize_vertical(vertical_size, height, &panel_area, bounds);
        centralize_horizontal(horizontal_size, width, &panel_area, bounds);
        *bounds = (disp_area_t){0}; /* no free space left */
    }
    else if (LAYOUT_ALIGN_TOP_H_CENTER == layout->align)
    {
        centralize_horizontal(horizontal_size, width, &panel_area, bounds);
        dock_to_top(vertical_size, height, &panel_area, bounds);
    }
    else if (LAYOUT_ALIGN_BOT_H_CENTER == layout->align)
    {
        centralize_horizontal(horizontal_size, width, &panel_area, bounds);
        dock_to_bot(vertical_size, height, &panel_area, bounds);
    }
    else if (LAYOUT_ALIGN_LEFT_V_CENTER == layout->align)
    {
        centralize_vertical(vertical_size, height, &panel_area, bounds);
        dock_to_left(horizontal_size, width, &panel_area, bounds);
    }
    else if (LAYOUT_ALIGN_RIGHT_V_CENTER == layout->align)
    {
        centralize_vertical(vertical_size, height, &panel_area, bounds);
        dock_to_right(horizontal_size, width, &panel_area, bounds);
    }
    else if ((LAYOUT_ALIGN_TOP | LAYOUT_ALIGN_BOT) & layout->align)
    {
        fill_horizontal(&panel_area, bounds);
        (LAYOUT_ALIGN_TOP & layout->align)
            ? dock_to_top(vertical_size, height, &panel_area, bounds)
            : dock_to_bot(vertical_size, height, &panel_area, bounds);
    }
    else if ((LAYOUT_ALIGN_LEFT | LAYOUT_ALIGN_RIGHT) & layout->align)
    {
        fill_vertical(&panel_area, bounds);
        (LAYOUT_ALIGN_LEFT & layout->align)
            ? dock_to_left(horizontal_size, width, &panel_area, bounds)
            : dock_to_right(horizontal_size, width, &panel_area, bounds);
    }

    return panel_area;
}

static void
centralize_vertical(unsigned int vertical_size,
                    unsigned int vmax,
                    disp_area_t *panel_area,
                    disp_area_t *bounds)
{
    panel_area->first.y = bounds->first.y;
    panel_area->second.y = bounds->second.y;
    if (vmax && vmax < vertical_size)
    {
        unsigned int padding = vertical_size - vmax;
        unsigned int top = padding / 2;
        unsigned int bot = padding - top;
        panel_area->first.y += top;
        panel_area->second.y -= bot;
    }
}

static void
centralize_horizontal(unsigned int horizontal_size,
                      unsigned int hmax,
                      disp_area_t *panel_area,
                      disp_area_t *bounds)
{
    panel_area->first.x = bounds->first.x;
    panel_area->second.x = bounds->second.x;
    if (hmax && hmax < horizontal_size)
    {
        unsigned int padding = horizontal_size - hmax;
        unsigned int left = padding / 2;
        unsigned int right = padding - left;
        panel_area->first.x += left;
        panel_area->second.x -= right;
    }
}

static void
dock_to_top(unsigned int vertical_size,
            unsigned int vmax,
            disp_area_t *panel_area,
            disp_area_t *bounds)
{
    panel_area->first.y = bounds->first.y;
    vmax = (vertical_size > vmax) ? vmax : vertical_size;
    panel_area->second.y = panel_area->first.y + vmax - 1;
    bounds->first.y = panel_area->second.y + 1;
}

static void
dock_to_bot(unsigned int vertical_size,
            unsigned int vmax,
            disp_area_t *panel_area,
            disp_area_t *bounds)
{
    panel_area->second.y = bounds->second.y;
    vmax = (vertical_size > vmax) ? vmax : vertical_size;
    panel_area->first.y = panel_area->second.y - vmax + 1;
    bounds->second.y = panel_area->first.y - 1;
}

static void
dock_to_left(unsigned int horizontal_size,
             unsigned int hmax,
             disp_area_t *panel_area,
             disp_area_t *bounds)
{
    panel_area->first.x = bounds->first.x;
    hmax = (horizontal_size > hmax) ? hmax : horizontal_size;
    panel_area->second.x = panel_area->first.x + hmax - 1;
    bounds->first.x = panel_area->second.x + 1;
}

static void
dock_to_right(unsigned int horizontal_size,
              unsigned int hmax,
              disp_area_t *panel_area,
              disp_area_t *bounds)
{
    panel_area->second.x = bounds->second.x;
    hmax = (horizontal_size > hmax) ? hmax : horizontal_size;
    panel_area->first.x = panel_area->second.x - hmax + 1;
    bounds->second.x = panel_area->first.x - 1;
}

static void
fill_vertical(disp_area_t *panel_area,
              disp_area_t *bounds)
{
    panel_area->first.y = bounds->first.y;
    panel_area->second.y = bounds->second.y;
}

static void
fill_horizontal(disp_area_t *panel_area,
                disp_area_t *bounds)
{
    panel_area->first.x = bounds->first.x;
    panel_area->second.x = bounds->second.x;
}
