#include "button.h"

#include "border.h"
#include "display.h"
#include "display_types.h"
#include "dynarr.h"
#include "interior.h"
#include "interior_layout.h"

#include "logger.h"


/* only extended part */
typedef struct
{
    bool pressed;
    button_action_t action;
}
button_interior_slice_t;


struct button_interior
{
    interior_t interior;
    button_interior_slice_t button;
};


static void *button_interior_alloc(Arena *arena);
static void button_interior_init(interior_t *const base, void *opts, Arena *const arena);
static void button_interior_deinit(interior_t *const base);
static void button_interior_recalculate(interior_t *const base, disp_area_t *const panel_area);
static void button_interior_render(const interior_t *base, display_t *const display);
static void button_interior_enter(interior_t *const base, const disp_pos_t pos);
static void button_interior_hover(interior_t *const base, const disp_pos_t pos);
static void button_interior_leave(interior_t *const base, const disp_pos_t pos);
static void button_interior_scroll(interior_t *const base, const disp_pos_t pos, const int dir);
static void button_interior_press(interior_t *const base, const disp_pos_t pos, const int btn);
static void button_interior_release(interior_t *const base, const disp_pos_t pos, const int btn);

static void action_perform(const button_action_t *const action);

interior_interface_t button_interior_get_impl(void)
{
    return (interior_interface_t){
        .alloc       = button_interior_alloc,
        .init        = button_interior_init,
        .deinit      = button_interior_deinit,
        .recalculate = button_interior_recalculate,
        .render      = button_interior_render,
        .enter       = button_interior_enter,
        .hover       = button_interior_hover,
        .leave       = button_interior_leave,
        .scroll      = button_interior_scroll,
        .press       = button_interior_press,
        .release     = button_interior_release,
    };
}


static void *button_interior_alloc(Arena *arena)
{
    return arena_alloc(arena, sizeof(button_interior_t));
}


static void button_interior_init(interior_t *const base, void *opts, Arena *const arena)
{
    button_interior_opts_t *button_opts = opts;
    assert(button_opts->action.action);
    UNUSED(arena);

    button_interior_t *interior = (button_interior_t*)base;

    interior->button = (button_interior_slice_t) {
        .action = button_opts->action,
    };
}


static void button_interior_deinit(interior_t *const base)
{
    UNUSED(base);
}


static void button_interior_recalculate(interior_t *const base, disp_area_t *const panel_area)
{
    UNUSED(base, panel_area);
}


static void button_interior_render(const interior_t *base, display_t *const display)
{
    button_interior_t *interior = (button_interior_t*)base;
    interior_area_t *area = dynarr_first(interior->interior.layout.areas);

    border_set_t border = {._ = L"╔╗╝╚║═"};
    style_t styles[] = {
        {.seq = ESC"[37m"},    // released
        {.seq = ESC"[30;47m"}, // pressed
    };

    display_fill_area(display, styles[interior->button.pressed], area->area);
    display_draw_border(display, styles[interior->button.pressed], border, area->area);
}


static void button_interior_press(interior_t *const base, const disp_pos_t pos, const int btn)
{
    UNUSED(pos);
    button_interior_t *interior = (button_interior_t*)base;
    interior->button.pressed = true;
    if (BUTTON_ON_PRESS == interior->button.action.when)
    {
        action_perform(&interior->button.action);
    }
    S_LOG(LOGGER_DEBUG, "btn pressed: %d \n", btn);
}


static void button_interior_release(interior_t *const base, const disp_pos_t pos, const int btn)
{
    UNUSED(pos);
    button_interior_t *interior = (button_interior_t*)base;
    interior->button.pressed = false;
    if (BUTTON_ON_RELEASE == interior->button.action.when)
    {
        action_perform(&interior->button.action);
    }
    S_LOG(LOGGER_DEBUG, "btn released: %d \n", btn);
}


static void button_interior_enter(interior_t *const base, const disp_pos_t pos)
{
    UNUSED(base, pos);
}


static void button_interior_hover(interior_t *const base, const disp_pos_t pos)
{
    UNUSED(base, pos);
}


static void button_interior_leave(interior_t *const base, const disp_pos_t pos)
{
    button_interior_t *interior = (button_interior_t*)base;
    UNUSED(pos);
    interior->button.pressed = false;
}


static void button_interior_scroll(interior_t *const base, const disp_pos_t pos, const int dir)
{
    UNUSED(base, pos, dir);
}


static void action_perform(const button_action_t *const action)
{
    action->action(action->action_data);
}
