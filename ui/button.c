#include "button.h"

#include "display_types.h"
#include "interior.h"
#include "interior_layout.h"

#include "logger.h"


/* only extended part */
typedef struct
{
    int sentinel; /* TODO: placeholder */
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
static void button_interior_hover(interior_t *const base, const disp_pos_t pos);
static void button_interior_scroll(interior_t *const base, const int dir);


interior_interface_t button_interior_get_impl(void)
{
    return (interior_interface_t){
        .alloc = button_interior_alloc,
        .init = button_interior_init,
        .deinit = button_interior_deinit,
        .recalculate = button_interior_recalculate,
        .render = button_interior_render,
        .hover = button_interior_hover,
        .scroll = button_interior_scroll,
    };
}


static void *button_interior_alloc(Arena *arena)
{
    return arena_alloc(arena, sizeof(button_interior_t));
}


static void button_interior_init(interior_t *const base, void *opts, Arena *const arena)
{
    (void) arena;
    button_interior_opts_t *button_opts = opts;
    (void) button_opts;
    button_interior_t *interior = (button_interior_t*)base;

    interior->button = (button_interior_slice_t) {
    };
}


static void button_interior_deinit(interior_t *const base)
{
    (void) base;
}


static void button_interior_recalculate(interior_t *const base, disp_area_t *const panel_area)
{
    (void) base; (void) panel_area;
}


static void button_interior_render(const interior_t *base, display_t *const display)
{
    button_interior_t *interior = (button_interior_t*)base;
    (void) interior; (void) display;
}


static void button_interior_hover(interior_t *const base, const disp_pos_t pos)
{
    (void) base; (void) pos;
}


static void button_interior_scroll(interior_t *const base, const int dir)
{
    (void) base; (void) dir;
}
