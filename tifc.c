#include "tifc.h"
#include "display.h"
#include "grid.h"
#include "layout.h"
#include "panel.h"
#include "ui.h"

#include <locale.h>

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

void tifc_create_ui_layout(tifc_t *const tifc)
{
    panel_opts_t *opts = &(panel_opts_t)
    {
        .title = "top",
        .layout = {
            .align = LAYOUT_ALIGN_TOP,
            .size_method = LAYOUT_SIZE_RELATIVE,
            .size = {.y = 50},
        },
        .columns = 1,
        .column_layout = (grid_layout_t[]){
            {.size = 100, .size_method = LAYOUT_SIZE_RELATIVE},
        },
        .rows = 10,
        .row_layout = (grid_layout_t[]){
            {.size = 3},
            {.size = 3},
            {.size = 3},
            {.size = 3},
            {.size = 3},
            {.size = 3},
            {.size = 3},
            {.size = 3},
            {.size = 3},
            {.size = 3},
        },
        .areas = 10,
        .areas_layout = (grid_area_opts_t[]){
            {{0, 0}, {0, 0}, TEXT_ALIGN_LEFT},
            {{0, 0}, {1, 1}, TEXT_ALIGN_LEFT},
            {{0, 0}, {2, 2}, TEXT_ALIGN_LEFT},
            {{0, 0}, {3, 3}, TEXT_ALIGN_LEFT},
            {{0, 0}, {4, 4}, TEXT_ALIGN_LEFT},
            {{0, 0}, {5, 5}, TEXT_ALIGN_LEFT},
            {{0, 0}, {6, 6}, TEXT_ALIGN_LEFT},
            {{0, 0}, {7, 7}, TEXT_ALIGN_LEFT},
            {{0, 0}, {8, 8}, TEXT_ALIGN_LEFT},
            {{0, 0}, {9, 9}, TEXT_ALIGN_LEFT},
        }
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

