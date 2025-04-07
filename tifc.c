#include "tifc.h"
#include "border.h"
#include "display.h"
#include "grid_panel.h"
#include "layout.h"
#include "panel.h"
#include "panel_factory.h"
#include "ui.h"

#include <locale.h>
#include <stdio.h>


tifc_t tifc_init(void)
{
    setlocale(LC_ALL, "");
    input_enable_mouse();
    tifc_t tifc = {
        .input = input_init(),
        .ui = ui_init(),
    };
    return tifc;
}


void tifc_deinit(tifc_t *const tifc)
{
    input_disable_mouse();
    input_deinit(&tifc->input);
    ui_deinit(&tifc->ui);
}


void tifc_render(tifc_t *const tifc)
{
    display_clear(&tifc->display);
    ui_render(&tifc->ui, &tifc->display);
    display_render(&tifc->display);
}

panel_factory_t g_factory = {0};
size_t g_array [] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

void size_t_array_render(display_t *const display, const grid_area_t *const area,
        const void *const source, const size_t limit, const size_t index)
{
    const size_t *source_ = source;
    char buf[18];
    size_t size = sprintf(buf, "%zu", source_[index]);
    border_set_t border = {._ = L"╭╮╯╰┆┄"};

    if (index < limit)
    {
        style_t style = BORDER_STYLE_1;
        if (area->is_hovered)
        {
            style = (style_t){.seq=ESC"[37;100m"};
            display_fill_area(display, style, area->area);
        }
        display_draw_border(display, style, border, area->area);
        display_draw_string_aligned(display, size, buf, area->area, style, LAYOUT_ALIGN_CENTER);
    }
    else
    {
        display_draw_string_centered(display, 13, "(unavailable)", area->area, (style_t){.seq=ESC"[31m"});
    }
}

void default_render(display_t *const display, const grid_area_t *const area,
        const void *const source, const size_t limit, const size_t index)
{
    (void) source; (void) limit; (void) index;
    border_set_t border = {._ = L"╭╮╯╰┆┄"};

    display_draw_border(display, BORDER_STYLE_1, border, area->area);
}


size_t g_array_amount(const void *const source)
{
    (void) source;
    return sizeof(g_array)/sizeof(g_array[0]);
}

void tifc_create_ui_layout(tifc_t *const tifc)
{
    grid_panel_opts_t *list_opts = &(grid_panel_opts_t){
        .panel = {
            .ifce = grid_panel_get_impl(),
            .title = "list of size_t",
            .layout = {
                .align = LAYOUT_ALIGN_LEFT,
                .size_method = LAYOUT_SIZE_RELATIVE,
                .size = {.x = 30},
        },
        .scrollable = true,
        },
        .data_source = g_array,
        .data_get_amount = g_array_amount,
        .data_render = size_t_array_render,
        .columns = 1,
        .column_layout = (grid_layout_t[]){
            {.amount = 1, .size = 100, .size_method = LAYOUT_SIZE_RELATIVE},
        },
        .rows = 11,
        .row_layout = (grid_layout_t[]){
            {.amount = 11, .size = 3},
        },
        .areas = 11,
        .areas_layout = (grid_area_opts_t[]){
            {{0, 0}, {0, 0}},
            {{0, 0}, {1, 1}},
            {{0, 0}, {2, 2}},
            {{0, 0}, {3, 3}},
            {{0, 0}, {4, 4}},
            {{0, 0}, {5, 5}},
            {{0, 0}, {6, 6}},
            {{0, 0}, {7, 7}},
            {{0, 0}, {8, 8}},
            {{0, 0}, {9, 9}},
            {{0, 0}, {10, 10}},
        }
    };
    panel_factory_init(&g_factory, list_opts, sizeof(*list_opts));
    (void) ui_add_panel(&tifc->ui, &g_factory);

    ui_recalculate(&tifc->ui, &tifc->display);
}

int tifc_event_loop(void)
{
    tifc_t tifc = tifc_init();
    resize_hook_with_data_t resize_hook = {
        .data = &tifc.ui,
        .hook = ui_resize_hook,
    };
    display_set_resize_handler(&tifc.display, resize_hook);
    tifc_create_ui_layout(&tifc);

    int exit_status = 0;

    while (1)
    {
        // input_display_overlay(&tifc.input, (disp_pos_t){.x = 0, .y = 3});
        tifc_render(&tifc);
        input_hooks_t *hooks = &tifc.ui.hooks;
        exit_status = input_handle_events(&tifc.input, hooks, &tifc.ui);
        if (0 != exit_status)
        {
            display_erase();
            break;
        }
    }

    tifc_deinit(&tifc);
    panel_factory_deinit(&g_factory);
    return exit_status;
}

int main(void)
{
    return tifc_event_loop();
}

