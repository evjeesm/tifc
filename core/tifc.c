#include "tifc.h"
#include "border.h"
#include "display.h"
#include "logger.h"
#include "ui.h"
#include "app.h"

#include <locale.h>
#include <stddef.h>
#include <stdio.h>

static int tifc_event_loop(void);
static void tifc_init(tifc_t *const tifc);
static void tifc_render(tifc_t *const tifc);
static void tifc_deinit(tifc_t *const tifc);


int main(void)
{
    return tifc_event_loop();
}


static int tifc_event_loop(void)
{
    tifc_t tifc;
    tifc_init(&tifc);
    resize_hook_with_data_t resize_hook = {
        .data = &tifc.ui,
    };
    display_hide_cursor();
    display_set_resize_handler(&tifc.display, resize_hook);

    void *app_data = app_get_data();
    app_init(app_data, &tifc);

    int exit_status = 0;

    while (1)
    {
        tifc_render(&tifc);
        input_hooks_t *hooks = &tifc.ui.hooks;
        exit_status = input_handle_events(&tifc.input, hooks, &tifc.ui);
        if (0 != exit_status || tifc.ui.exit_requested )
        {
            display_erase();
            break;
        }
    }

    app_deinit(app_data, &tifc);
    tifc_deinit(&tifc);
    display_show_cursor();
    return exit_status;
}


static void tifc_init(tifc_t *const tifc)
{
    display_enter_alternate_screen();
    setlocale(LC_ALL, "");
    input_enable_mouse();
    *tifc = (tifc_t){ 0 };
    input_init(&tifc->input);
    ui_init(&tifc->ui);
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
    ui_deinit(&tifc->ui);
    display_leave_alternate_screen();
}

