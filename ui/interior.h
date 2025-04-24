#ifndef _INTERIOR_H_
#define _INTERIOR_H_

#include "arena.h"
#include "display.h"
#include "interior_layout.h"


#define _UNUSED_1(arg) (void) arg
#define _UNUSED_2(arg1, arg2) _UNUSED_1(arg1); _UNUSED_1(arg2)
#define _UNUSED_3(arg1, arg2, arg3) _UNUSED_2(arg1, arg2); _UNUSED_1(arg3)
#define _CONCAT(a, b) a ## b
#define _UNUSED_COUNT(PREFIX, _1, _2, _3, NUM, ...) _CONCAT(PREFIX, NUM)
#define UNUSED(...) _UNUSED_COUNT(_UNUSED_,__VA_ARGS__, 3, 2, 1)(__VA_ARGS__)

/*
* This is a component of a panel that defines the inner content.
*/

typedef struct interior interior_t;

typedef struct
{
    void *(*alloc) (Arena *arena);
    void (*init) (interior_t *const interior, void *opts, Arena *const arena);
    void (*deinit) (interior_t *const interior);
    void (*recalculate) (interior_t *const interior, disp_area_t *const panel_area);
    void (*render) (const interior_t *interior, display_t *const display);
    void (*enter) (interior_t *const interior, const disp_pos_t pos);
    void (*hover) (interior_t *const interior, const disp_pos_t pos);
    void (*leave) (interior_t *const interior, const disp_pos_t pos);
    void (*scroll) (interior_t *const interior, const disp_pos_t pos, const int dir);
    void (*press) (interior_t *const interior, const disp_pos_t pos, const int btn);
    void (*release) (interior_t *const interior, const disp_pos_t pos, const int btn);
    /* ... */
}
interior_interface_t;


typedef struct
{
    interior_interface_t   impl;
    interior_layout_opts_t layout;
}
interior_opts_t;


struct interior
{
    interior_interface_t impl;
    interior_layout_t    layout;
};


interior_t *interior_alloc(const interior_opts_t *const opts, Arena *const arena);
void interior_init(interior_t *const interior, const interior_opts_t *const opts, Arena *const arena);
void interior_deinit(interior_t *const interior);
void interior_render(const interior_t *interior, display_t *const display);
void interior_recalculate(interior_t *interior, disp_area_t *const panel_area);
void interior_enter(interior_t *const interior, const disp_pos_t pos);
void interior_hover(interior_t *const interior, const disp_pos_t pos);
void interior_leave(interior_t *const interior, const disp_pos_t pos);
void interior_scroll(interior_t *const interior, const disp_pos_t pos, const int direction);
void interior_press(interior_t *const interior, const disp_pos_t pos, const int btn);
void interior_release(interior_t *const interior, const disp_pos_t pos, const int btn);
#endif/*_INTERIOR_H_*/
