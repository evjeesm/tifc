#include "tifc.h"
#include "border.h"
#include "display.h"
#include "layout.h"
#include "logger.h"
#include "ui.h"

#include <locale.h>
#include <stddef.h>
#include <stdio.h>

static int tifc_event_loop(void);
static tifc_t tifc_init(void);
static void tifc_render(tifc_t *const tifc);
static void tifc_deinit(tifc_t *const tifc);


int main(void)
{
    return tifc_event_loop();
}


static int tifc_event_loop(void)
{
    tifc_t tifc = tifc_init();
    resize_hook_with_data_t resize_hook = {
        .data = &tifc.ui,
    };
    display_hide_cursor();
    display_set_resize_handler(&tifc.display, resize_hook);

    int exit_status = 0;

    while (1)
    {
        // input_display_overlay(&tifc.input, (disp_pos_t){.x = 0, .y = 3});
        tifc_render(&tifc);
        input_hooks_t *hooks = &tifc.ui.hooks;
        exit_status = input_handle_events(&tifc.input, hooks, &tifc.ui);
        if (0 != exit_status /* || tifc.ui.exit_requested */)
        {
            display_erase();
            break;
        }
    }

    tifc_deinit(&tifc);
    display_show_cursor();
    return exit_status;
}


static tifc_t tifc_init(void)
{
    display_enter_alternate_screen();
    setlocale(LC_ALL, "");
    input_enable_mouse();
    tifc_t tifc = { 0 };
    input_init(&tifc.input);
    return tifc;
}

static void tifc_render(tifc_t *const tifc)
{
    display_clear(&tifc->display);
    display_render(&tifc->display);
}

static void tifc_deinit(tifc_t *const tifc)
{
    input_disable_mouse();
    input_deinit(&tifc->input);
    display_leave_alternate_screen();
}

