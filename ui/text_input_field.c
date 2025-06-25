#include "text_input_field.h"

#include "display.h"
#include "dynarr.h"
#include "input.h"
#include "interior.h"
#include "layout.h"
#include "logger.h"
#include "utils.h"

typedef enum
{
    TIF_INACTIVE = 0,
    TIF_ACTIVE,
    TIF_ERROR
}
text_input_state_t;

/* only extended part */
typedef struct
{
    dynarr_t *text;
    size_t    caret;
    size_t    offset;
    text_input_state_t state;
}
text_input_field_slice_t;


struct text_input_field
{
    interior_t interior;
    text_input_field_slice_t text_input_field;
};


static void *text_input_field_alloc(Arena *arena);
static void text_input_field_init(interior_t *const base, void *opts, Arena *const arena);
static void text_input_field_deinit(interior_t *const base);
static void text_input_field_recalculate(interior_t *const base, disp_area_t *const panel_area);
static void text_input_field_render(const interior_t *base, display_t *const display);
static void text_input_field_enter(interior_t *const base, const disp_pos_t pos);
static void text_input_field_hover(interior_t *const base, const disp_pos_t pos);
static void text_input_field_leave(interior_t *const base, const disp_pos_t pos);
static void text_input_field_recv_focus(interior_t *const base);
static void text_input_field_lost_focus(interior_t *const base);
static void text_input_field_scroll(interior_t *const base, const disp_pos_t pos, const int dir);
static void text_input_field_press(interior_t *const base, const disp_pos_t pos, const int btn);
static void text_input_field_release(interior_t *const base, const disp_pos_t pos, const int btn);
static void text_input_field_keystroke(interior_t *const interior, const keystroke_event_t *const event);


interior_interface_t text_input_field_interior_get_impl(void)
{
    return (interior_interface_t){
        .alloc       = text_input_field_alloc,
        .init        = text_input_field_init,
        .deinit      = text_input_field_deinit,
        .recalculate = text_input_field_recalculate,
        .render      = text_input_field_render,
        .enter       = text_input_field_enter,
        .hover       = text_input_field_hover,
        .leave       = text_input_field_leave,
        .scroll      = text_input_field_scroll,
        .press       = text_input_field_press,
        .release     = text_input_field_release,
        .keystroke   = text_input_field_keystroke,
        .recv_focus  = text_input_field_recv_focus,
        .lost_focus  = text_input_field_lost_focus,
    };
}

static void *text_input_field_alloc(Arena *arena)
{
    return arena_alloc(arena, sizeof(text_input_field_t));
}


static void text_input_field_init(interior_t *const base, void *opts, Arena *const arena)
{
    text_input_field_opts_t *tif_opts = opts;
    assert(tif_opts/*->option*/);
    UNUSED(arena);

    text_input_field_t *interior = (text_input_field_t*)base;

    interior->text_input_field = (text_input_field_slice_t) {
        .text = dynarr_create(.element_size = sizeof(char))
    };
}


static void text_input_field_deinit(interior_t *const base)
{
    text_input_field_t *interior = (text_input_field_t*)base;
    dynarr_destroy(interior->text_input_field.text);
}


static void text_input_field_recalculate(interior_t *const base, disp_area_t *const panel_area)
{
    UNUSED(base, panel_area);
}


static void text_input_field_render(const interior_t *base, display_t *const display)
{
    text_input_field_t *interior = (text_input_field_t*)base;
    interior_area_t *area = dynarr_first(interior->interior.layout.areas);
    const size_t width = disp_area_width(area->area);
    const size_t window_length =  width <= 2 ? 0 : width - 2;
    const size_t text_length = dynarr_size(interior->text_input_field.text);

    border_set_t border[] = {
        {._ = L"┌┐┘└│─"},
        {._ = L"╔╗╝╚║═"},
        {._ = L"▛▜▟▙▍▃"},
    };
    style_t styles[] = {
        {.seq = ESC"[37m"},    // inactive
        {.seq = ESC"[37m"},    // active
        {.seq = ESC"[30;41m"}, // error
    };

    display_fill_area(display,
            styles[interior->text_input_field.state],
            area->area);
    display_draw_border(display, styles[interior->text_input_field.state],
            border[interior->text_input_field.state],
            area->area);

    // Left text overflow indicator
    disp_pos_t pos = area->area.first;
    pos.y += disp_area_height(area->area)/2;
    if (interior->text_input_field.offset > 0)
    {
        display_set_char(display, '<', pos);
    }

    // Right text overflow indicator
    pos.x = area->area.second.x;
    if (text_length - interior->text_input_field.offset > window_length)
    {
        display_set_char(display, '>', pos);
    }

    // Draw centered text content
    disp_area_t text_area = area->area;
    text_area.first.x += 1;
    display_draw_string_aligned(display,
            (text_length - interior->text_input_field.offset) >= window_length
                ? window_length
                : (text_length - interior->text_input_field.offset),
            vector_data(interior->text_input_field.text) + interior->text_input_field.offset,
            text_area,
            styles[0],
            LAYOUT_ALIGN_LEFT_V_CENTER);

    // Do not draw caret when inactive
    if (TIF_INACTIVE == interior->text_input_field.state) return;

    // Draw caret
    disp_pos_t caret_pos = {
        .x = area->area.first.x + 1 + interior->text_input_field.caret,
        .y = area->area.first.y + (disp_area_height(area->area)) / 2
    };
    display_set_style(display, styles[2], caret_pos);
}


static void text_input_field_enter(interior_t *const base, const disp_pos_t pos)
{
    UNUSED(base, pos);
}


static void text_input_field_hover(interior_t *const base, const disp_pos_t pos)
{
    UNUSED(base, pos);
}


static void text_input_field_leave(interior_t *const base, const disp_pos_t pos)
{
    UNUSED(base, pos);
}


static void text_input_field_recv_focus(interior_t *const base)
{
    UNUSED(base);
    text_input_field_t *interior = (text_input_field_t*)base;
    interior->text_input_field.state = TIF_ACTIVE;
}


static void text_input_field_lost_focus(interior_t *const base)
{
    UNUSED(base);
    text_input_field_t *interior = (text_input_field_t*)base;
    interior->text_input_field.state = TIF_INACTIVE;
}


static void text_input_field_scroll(interior_t *const base, const disp_pos_t pos, const int dir)
{
    UNUSED(base, pos, dir);
    TODO("Unimplemented yet!");
}


static void text_input_field_press(interior_t *const base, const disp_pos_t pos, const int btn)
{
    UNUSED(pos);

    text_input_field_t *interior = (text_input_field_t*) base;
    UNUSED(interior);

    if (MOUSE_1 == btn)
    {
        text_input_field_recv_focus(base);
        /* TODO: change cursor position */
    }
}


static void text_input_field_release(interior_t *const base, const disp_pos_t pos, const int btn)
{
    UNUSED(base, pos, btn);
}


static void text_input_field_keystroke(interior_t *const base, const keystroke_event_t *const event)
{
    text_input_field_t *interior = (text_input_field_t*) base;
    const size_t text_length = dynarr_size(interior->text_input_field.text);

    const interior_area_t *area = dynarr_first(interior->interior.layout.areas);
    const size_t width = disp_area_width(area->area);
    const size_t window_length =  width <= 2 ? 0 : width - 2;

    const size_t pos_from_start = interior->text_input_field.offset + interior->text_input_field.caret;
    switch (event->code)
    {
        case KEY_LEFT:
            if (interior->text_input_field.caret > 0)
            {
                --interior->text_input_field.caret;
            }
            else if (interior->text_input_field.offset > 0)
            {
                --interior->text_input_field.offset;
            }
            break;
        case KEY_RIGHT:
        {
            const size_t after_offset = text_length - interior->text_input_field.offset;
            if (interior->text_input_field.caret < after_offset)
            {
                if (interior->text_input_field.caret < window_length)
                {
                    ++interior->text_input_field.caret;
                }
                else
                {
                    ++interior->text_input_field.offset;
                }
            }
            break;
        }
        case KEY_BACKSPACE:
        {
            const bool has_chars_from_left = (text_length > 0)
                && (interior->text_input_field.offset
                    || interior->text_input_field.caret);

            if (has_chars_from_left)
            {
                dynarr_remove(&interior->text_input_field.text, pos_from_start - 1);
                if (interior->text_input_field.caret > 0)
                {
                    --interior->text_input_field.caret;
                }
                else if (interior->text_input_field.offset > 0)
                {
                    --interior->text_input_field.offset;
                }
            }
            break;
        }
        case KEY_DELETE:
        {
            const bool has_chars_from_right = text_length > 0 && pos_from_start < text_length;

            if (has_chars_from_right)
            {
                dynarr_remove(&interior->text_input_field.text, pos_from_start);
            }
            break;
        }
        case KEY_RETURN:
            // TODO: something else
            S_LOG(LOGGER_DEBUG, "Entered text: \"%s\"\n", vector_data(interior->text_input_field.text));
            break;
        case KEY_ESC:
            break;
        default:
            if (event->stroke)
            {
                dynarr_insert(&interior->text_input_field.text,
                        interior->text_input_field.caret + interior->text_input_field.offset,
                        &event->stroke);

                if (interior->text_input_field.caret < window_length)
                {
                    ++interior->text_input_field.caret;
                }
                else
                {
                    ++interior->text_input_field.offset;
                }
            }
    }
}
