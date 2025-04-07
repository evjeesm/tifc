#include "grid_panel.h"
#include "grid.h"
#include "panel.h"

struct grid_panel
{
    panel_t panel;
    grid_t grid;

    // Content view:
    void *data_source;
    data_get_amount_t data_get_amount;
    area_render_t data_render; // TODO:item_render?
};

static void* grid_panel_alloc(Arena *arena);
static void grid_panel_init(panel_t *const panel, void *opts);
static void grid_panel_deinit(panel_t *const panel);
static void grid_panel_recalculate(panel_t *const panel, disp_area_t *const bounds);
static void grid_panel_render(const panel_t *const panel, display_t *display);
static void grid_panel_hover(panel_t *const panel, const disp_pos_t pos);
static void grid_panel_scroll(panel_t *const panel, const int dir);


panel_interface_t grid_panel_get_impl(void)
{
    return (panel_interface_t) {
        .alloc = grid_panel_alloc,
        .init = grid_panel_init,
        .deinit = grid_panel_deinit,
        .recalculate = grid_panel_recalculate,
        .render = grid_panel_render,
        .hover = grid_panel_hover,
        .scroll = grid_panel_scroll,
    };
}


static void* grid_panel_alloc(Arena *arena)
{
    return arena_alloc(arena, sizeof(grid_panel_t));
}


static void grid_panel_init(panel_t *const panel, void *opts)
{
    grid_panel_t *panel_ = (void*) panel;
    grid_panel_opts_t *opts_ = opts;

    // GRID CONTENT:
    grid_init(&panel_->grid,
        opts_->columns,
        opts_->rows,
        opts_->column_layout,
        opts_->row_layout);

    // set grid to scrollable
    panel_->grid.is_scrollable = opts_->panel.scrollable;

    // setup data source
    panel_->data_source = opts_->data_source;
    panel_->data_get_amount = opts_->data_get_amount;
    panel_->data_render = opts_->data_render;

    for (size_t a = 0; a < opts_->areas; ++a)
    {
        grid_add_area(&panel_->grid, &opts_->areas_layout[a]);
    }
}


static void grid_panel_deinit(panel_t *const panel)
{
    grid_panel_t *panel_ = (void*) panel;

    // GRID CONTENT:
    grid_deinit(&panel_->grid);
}


static void grid_panel_recalculate(panel_t *const panel, disp_area_t *const bounds)
{
    (void)(bounds);
    grid_panel_t *panel_ = (void*) panel;

    // GRID CONTENT:
    grid_recalculate_layout(&panel_->grid, &panel->area);
}


static void grid_panel_render(const panel_t *const panel, display_t *display)
{
    grid_panel_t *panel_ = (void*) panel;

    // TODO: factor out ?
    size_t amount = 0;
    if (panel_->data_source)
    { 
        amount = panel_->data_get_amount(panel_->data_source);
    }

    grid_render(&panel_->grid, display, panel_->data_source, amount, panel_->data_render);
}


static void grid_panel_hover(panel_t *const panel, const disp_pos_t pos)
{
    grid_panel_t *panel_ = (void*) panel;
    grid_hover(&panel_->grid, pos);
}


static void grid_panel_scroll(panel_t *const panel, const int dir)
{
    grid_panel_t *panel_ = (void*) panel;
    size_t amount = 0;

    if (panel_->data_source)
    {
        amount = panel_->data_get_amount(panel_->data_source);
    }

    grid_scroll(&panel_->grid, dir, amount);
}


