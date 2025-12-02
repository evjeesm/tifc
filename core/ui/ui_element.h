#ifndef _UI_ELEMENT_H_
#define _UI_ELEMENT_H_

#include "arena.h"
#include "display.h"
#include "input.h"

#define UI_ELEMENT_FIELDS \


typedef size_t ui_id_t;
struct ui_element;

typedef struct ui_element_interface
{
    void *(*alloc) (Arena *const arena);
    void (*init) (struct ui_element *const element, const void *const opts);
    void (*deinit) (struct ui_element *const element);
    void (*recalculate) (struct ui_element *const element, disp_area_t *const bounds);
    void (*render) (const struct ui_element *element, display_t *const display);
    void (*enter) (struct ui_element *const ui_element, const disp_pos_t pos);
    void (*hover) (struct ui_element *const ui_element, const disp_pos_t pos);
    void (*leave) (struct ui_element *const ui_element, const disp_pos_t pos);
    void (*scroll) (struct ui_element *const ui_element, const disp_pos_t pos, const int direction);
    void (*press) (struct ui_element *const ui_element, const disp_pos_t pos, const int btn);
    void (*release) (struct ui_element *const ui_element, const disp_pos_t pos, const int btn);
    void (*keystroke) (struct ui_element *const ui_element, const keystroke_event_t *const event);
    void (*recv_focus) (struct ui_element *const ui_element);
    void (*lost_focus) (struct ui_element *const ui_element);
    /* ... */
}
ui_element_interface_t;


typedef struct ui_element
{
    ui_id_t                id;
    ui_id_t                parent;
    disp_area_t            area;
    ui_element_interface_t impl;
    Arena                 *arena;
}
ui_element_t;


typedef struct
{
    ui_element_interface_t impl;
    Arena                 *arena;
}
ui_element_opts_t;


void *ui_element_alloc(const ui_element_opts_t *const opts);
void ui_element_init(ui_element_t *const ui_element, const ui_element_opts_t *const opts);
void ui_element_deinit(ui_element_t *const ui_element);
void ui_element_recalculate(ui_element_t *ui_element, disp_area_t *const bounds);
void ui_element_render(const ui_element_t *ui_element, display_t *const display);
void ui_element_enter(ui_element_t *const ui_element, const disp_pos_t pos);
void ui_element_hover(ui_element_t *const ui_element, const disp_pos_t pos);
void ui_element_leave(ui_element_t *const ui_element, const disp_pos_t pos);
void ui_element_scroll(ui_element_t *const ui_element, const disp_pos_t pos, const int direction);
void ui_element_press(ui_element_t *const ui_element, const disp_pos_t pos, const int btn);
void ui_element_release(ui_element_t *const ui_element, const disp_pos_t pos, const int btn);
void ui_element_keystroke(ui_element_t *const ui_element, const keystroke_event_t *const event);
void ui_element_recv_focus(ui_element_t *const ui_element);
void ui_element_lost_focus(ui_element_t *const ui_element);

#endif // _UI_ELEMENT_H_
