#include "ui_container.h"
#include "display_types.h"

#include "logger.h"
#include "sparse.h"
#include "utils.h"

#include <assert.h>
#include <stdlib.h>

static void *ui_container_alloc(Arena *const arena);
static void ui_container_init(ui_element_t *const ui_element, const void *const opts);
static void ui_container_deinit(ui_element_t *const ui_element);
static void ui_container_recalculate(ui_element_t *ui_element, disp_area_t *const bounds);
static void ui_container_render(const ui_element_t *ui_element, display_t *const display);
static void ui_container_enter(ui_element_t *const ui_element, const disp_pos_t pos);
static void ui_container_hover(ui_element_t *const ui_element, const disp_pos_t pos);
static void ui_container_leave(ui_element_t *const ui_element, const disp_pos_t pos);
static void ui_container_scroll(ui_element_t *const ui_element, const disp_pos_t pos, const int direction);
static void ui_container_press(ui_element_t *const ui_element, const disp_pos_t pos, const int btn);
static void ui_container_release(ui_element_t *const ui_element, const disp_pos_t pos, const int btn);
static void ui_container_keystroke(ui_element_t *const ui_element, const keystroke_event_t *const event);
static void ui_container_recv_focus(ui_element_t *const ui_element);
static void ui_container_lost_focus(ui_element_t *const ui_element);


void ui_container_assign_child(ui_container_t *const container,
                               ui_id_t element_id, ui_id_t area_id)
{
    assert(container);
    assert(sparse_get(container->children, area_id) && "Element already assigned to this area");

    (void) sparse_insert(&container->children, area_id, &element_id);
}


ui_element_interface_t ui_container_get_impl(void)
{
    return (ui_element_interface_t) {
         .alloc = ui_container_alloc,
         .init = ui_container_init,
         .deinit = ui_container_deinit,
         .recalculate = ui_container_recalculate,
         .render = ui_container_render,
         .enter = ui_container_enter,
         .hover = ui_container_hover,
         .leave = ui_container_leave,
         .scroll = ui_container_scroll,
         .press = ui_container_press,
         .release = ui_container_release,
         .keystroke = ui_container_keystroke,
         .recv_focus = ui_container_recv_focus,
         .lost_focus = ui_container_lost_focus,
    };
}


static void *ui_container_alloc(Arena *const arena)
{
    return arena_alloc(arena, sizeof(ui_container_t));
}


static void ui_container_init(ui_element_t *const ui_element, const void *const opts)
{
    *(ui_container_t*)ui_element = (ui_container_t) {
        .children = sparse_create(.element_size = sizeof(ui_id_t)),
    };
    UNUSED(opts);
}


static void ui_container_deinit(ui_element_t *const ui_element)
{
    sparse_destroy(((ui_container_t*)ui_element)->children);
}


static void ui_container_recalculate(ui_element_t *ui_element, disp_area_t *const bounds)
{
    UNUSED(ui_element, bounds);
}


static void ui_container_render(const ui_element_t *ui_element, display_t *const display)
{
    UNUSED(ui_element, display);
}


static void ui_container_enter(ui_element_t *const ui_element, const disp_pos_t pos)
{
    UNUSED(ui_element, pos);
}


static void ui_container_hover(ui_element_t *const ui_element, const disp_pos_t pos)
{
    UNUSED(ui_element, pos);
}


static void ui_container_leave(ui_element_t *const ui_element, const disp_pos_t pos)
{
    UNUSED(ui_element, pos);
}


static void ui_container_scroll(ui_element_t *const ui_element, const disp_pos_t pos, const int direction)
{
    UNUSED(ui_element, pos, direction);
}


static void ui_container_press(ui_element_t *const ui_element, const disp_pos_t pos, const int btn)
{
    UNUSED(ui_element, pos, btn);
}


static void ui_container_release(ui_element_t *const ui_element, const disp_pos_t pos, const int btn)
{
    UNUSED(ui_element, pos, btn);
}


static void ui_container_keystroke(ui_element_t *const ui_element, const keystroke_event_t *const event)
{
    UNUSED(ui_element, event);
}


static void ui_container_recv_focus(ui_element_t *const ui_element)
{
    UNUSED(ui_element);
}


static void ui_container_lost_focus(ui_element_t *const ui_element)
{
    UNUSED(ui_element);
}


