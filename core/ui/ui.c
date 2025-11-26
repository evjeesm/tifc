#include "ui.h"
#include "display_types.h"
#include "input.h"
#include "logger.h"
#include "display.h"
#include "utils.h"

//
// Mouse events
//
static void ui_hover(const mouse_event_t *const, void *const);
static void ui_press(const mouse_event_t *const, void *const);
static void ui_release(const mouse_event_t *const, void *const);
static void ui_drag_begin(const mouse_event_t *const, void *const);
static void ui_drag(const mouse_event_t *const, const mouse_event_t *const, void *const);
static void ui_drag_end(const mouse_event_t *const, const mouse_event_t *const, void *const);
static void ui_scroll(const mouse_event_t *const, void *const);

//
// Keyboard
//
static void ui_keystroke(const keystroke_event_t *const, void *const);

static input_hooks_t hooks_init(void)
{
    return (input_hooks_t)
    {
        .on_hover = ui_hover,
        .on_press = ui_press,
        .on_release = ui_release,
        .on_drag_begin = ui_drag_begin,
        .on_drag = ui_drag,
        .on_drag_end = ui_drag_end,
        .on_scroll = ui_scroll,
        .on_keystroke = ui_keystroke,
    };
}


void ui_init(ui_t *const ui)
{
    *ui = (ui_t){
        .hooks = hooks_init(),
    };

    hashmap_t *elements = hm_create(
        .hashfunc = hash_long,
        .key_size = sizeof(ui_id_t),
        .value_size = sizeof(ui_element_t*),
    );

    if (!elements)
    {
        exit(EXIT_FAILURE);
    }

    hashmap_t *containers = hm_create(
        .hashfunc = hash_long,
        .key_size = sizeof(ui_id_t),
        .value_size = sizeof(ui_container_t*),
    );

    if (!containers)
    {
        exit(EXIT_FAILURE);
    }

    ui->elements = elements;
    ui->containers = containers;
}


void ui_deinit(ui_t *const ui)
{
    assert(ui);
}


void ui_recalculate(ui_t *const ui, const display_t *const display)
{
    assert(ui);
    assert(display);

    // disp_area_t bounds = {
    //     .first = {0, 0},
    //     .second = {display->size.x - 1, display->size.y - 1}
    // };
}


ui_id_t ui_create_element(ui_t *const ui, ui_element_opts_t *const opts)
{
    ui_id_t id = ui->new_element_id++; /* reserve id */

    ui_element_t *element = ui_element_alloc(opts);
    ui_element_init(element, opts);

    (void) hm_insert(&ui->elements, &id, &element);

    return id;
}


ui_id_t ui_create_container(ui_t *const ui, ui_container_opts_t *const opts)
{
    ui_id_t id = ui_create_element(ui, (ui_element_opts_t *const) opts);
    ui_element_t *element = *(ui_element_t**) hm_get(ui->elements, &id);
    (void) hm_insert(&ui->containers, &id, (ui_container_t*) element);

    return id;
}


ui_status_t ui_assign_to_container(ui_t *const ui, ui_id_t element_id,
                                   ui_id_t container_id, ui_id_t area_id)
{
    ui_element_t *element =  *(ui_element_t**) hm_get(ui->elements, &element_id);
    if (!element)
    {
        return UI_ELEMENT_NOT_FOUND;
    }

    ui_container_t *container = *(ui_container_t**) hm_get(ui->containers, &container_id);
    if (!container)
    {
        return UI_CONTAINER_NOT_FOUND;
    }

    /* TODO assign element to container */
    UNUSED(area_id);

    return UI_SUCCESS;
}


static void ui_hover(const mouse_event_t *const hover, void *const param)
{
    // ui_t *ui = param;
    UNUSED(param);
    S_LOG(LOGGER_DEBUG, "UI::hover, at %u, %u\n",
        hover->position.x, hover->position.y);
}


static void ui_press(const mouse_event_t *const press, void *const param)
{
    // ui_t *ui = param;
    UNUSED(param);
    S_LOG(LOGGER_DEBUG, "UI::press %d, at %u, %u\n",
        press->mouse_button, press->position.x, press->position.y);
}


static void ui_release(const mouse_event_t *const press, void *const param)
{
    // ui_t *ui = param;
    UNUSED(param);
    S_LOG(LOGGER_DEBUG, "UI::release %d, at %u, %u\n",
        press->mouse_button, press->position.x, press->position.y);
}


static void ui_drag_begin(const mouse_event_t *const begin,
        void *const param)
{
    UNUSED(param);
    S_LOG(LOGGER_DEBUG, "UI::drag %d begin, at %u, %u\n",
        begin->mouse_button, begin->position.x, begin->position.y);
}


static void ui_drag(const mouse_event_t *const begin, const mouse_event_t *const moved, void *const param)
{
    UNUSED(param);
    S_LOG(LOGGER_DEBUG, "UI::drag %d drag moving to %u, %u\n",
        begin->mouse_button, moved->position.x, moved->position.y);
}


static void ui_drag_end(const mouse_event_t *const begin,
        const mouse_event_t *const end, void *const param)
{
    UNUSED(param);
    S_LOG(LOGGER_DEBUG, "UI::drag %d from %u, %u to %u, %u\n",
        begin->mouse_button,
        begin->position.x, begin->position.y,
        end->position.x, end->position.y);
}


static void ui_scroll(const mouse_event_t *const scroll, void *const param)
{
    // ui_t *ui = param;

    UNUSED(param);
    S_LOG(LOGGER_DEBUG, "UI::scroll %d at %u, %u\n",
        scroll->mouse_button, scroll->position.x, scroll->position.y);
}


static void ui_keystroke(const keystroke_event_t *const event, void *const param)
{
    ui_t *ui = param;

    S_LOG(LOGGER_DEBUG, "UI::keystroke %x(%d)\n mod:%x\n", event->stroke, event->code, event->modifier);

    if (KEY_D == event->code && MOD_CTRL == event->modifier)
    {
        ui->exit_requested = true;
        return;
    }
}
