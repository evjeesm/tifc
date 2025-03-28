#include "tifc.h"
#include "border.h"
#include "display.h"
#include "grid.h"
#include "layout.h"
#include "panel.h"
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
}


void tifc_render(tifc_t *const tifc)
{
    display_clear(&tifc->display);
    ui_render(&tifc->ui, &tifc->display);
    display_render(&tifc->display);
}


size_t g_array [] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};

void size_t_array_render(display_t *const display, const disp_area_t area, const void *const source, const size_t limit, const size_t index)
{
    const size_t *source_ = source;
    char buf[18];
    size_t size = sprintf(buf, "%zu", source_[index]);
    border_set_t border = {._ = L"╭╮╯╰┆┄"};
    display_draw_border(display, BORDER_STYLE_1, border, area);
    if (index < limit)
    {
        display_draw_string_aligned(display, size, buf, area, (style_t){0}, LAYOUT_ALIGN_CENTER);
    }
    else
    {
        display_draw_string_centered(display, 13, "(unavailable)", area, (style_t){.seq=ESC"[31m"});
    }
}


size_t g_array_amount(const void *const source)
{
    (void) source;
    return sizeof(g_array)/sizeof(g_array[0]);
}

void tifc_create_ui_layout(tifc_t *const tifc)
{
    panel_opts_t *opts = &(panel_opts_t)
    {
        .title = "top",
        .layout = {
            .align = LAYOUT_ALIGN_TOP,
            .size_method = LAYOUT_SIZE_RELATIVE,
            .size = {.y = 100},
        },
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
        },
        .data_source = g_array,
        .data_get_amount = g_array_amount,
        .data_render = size_t_array_render,
    };
    (void) ui_add_panel(&tifc->ui, opts);


    // opts->title = "left";
    // opts->layout.align = LAYOUT_ALIGN_LEFT;
    // opts->layout.size.x = 50;
    // (void) ui_add_panel(&tifc->ui, opts);
    //
    // opts->title = "right-top";
    // opts->layout.align = LAYOUT_ALIGN_TOP;
    // opts->layout.size.y = 50;
    // (void) ui_add_panel(&tifc->ui, opts);
    //
    // opts->title = "right-bot";
    // opts->layout.align = LAYOUT_ALIGN_BOT;
    // opts->layout.size.y = 100;
    // (void) ui_add_panel(&tifc->ui, opts);

    ui_recalculate_layout(&tifc->ui, &tifc->display);
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
        exit_status = input_handle_events(&tifc.input, hooks, &tifc);
        if (0 != exit_status)
        {
            display_erase();
            break;
        }
    }

    tifc_deinit(&tifc);
    return exit_status;
}

int main(void)
{
    return tifc_event_loop();
}

