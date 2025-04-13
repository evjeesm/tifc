#ifndef _DISPLAY_H_
#define _DISPLAY_H_

#include "display_types.h"
#include "border.h"
#include "layout.h"
#include <wchar.h>

#include <stdbool.h>

#define DISP_BUFFERS 2
#define DISP_MAX_WIDTH 256
#define DISP_MAX_HEIGHT 256

#define ESC         "\x1b"
#define HOME        ESC "[H"
#define ROW(n) HOME ESC "["#n"B"
#define CLEAR       ESC "[2J"
#define RESET_STYLE ESC "[0m"
#define HIDE_CURSOR ESC "[?25l"
#define SHOW_CURSOR ESC "[?25h"
#define ERASE_LINE  ESC "[K"

#define ALTER_SCREEN  ESC "[?1049h"
#define NORMAL_SCREEN ESC "[?1049l"

typedef struct
{
    const char *seq;
}
style_t;

typedef struct
{
    style_t style;
    wchar_t ch;
}
disp_char_t;
typedef disp_char_t (*dispbuf_ptr_t)[DISP_MAX_WIDTH];

typedef struct display
{
    disp_char_t buffers[DISP_BUFFERS][DISP_MAX_HEIGHT][DISP_MAX_WIDTH];
    int active; /* index of the active buffer */
    disp_pos_t size;
}
display_t;

typedef struct
{
    void *data;
    void (*hook) (const display_t *const display, void *data);
}
resize_hook_with_data_t;

void display_enter_alternate_screen(void);
void display_leave_alternate_screen(void);

void
display_set_char(display_t *const display,
        wint_t ch,
        disp_pos_t pos);
void 
display_set_style(display_t *const display,
        style_t style,
        disp_pos_t pos);
void
display_draw_border(display_t *const display,
        style_t style,
        border_set_t border,
        disp_area_t area);
void
display_fill_area(display_t *const display,
        style_t style,
        disp_area_t area);
void
display_draw_string(display_t *const display,
        unsigned int size,
        const char string[size],
        disp_pos_t pos,
        style_t style);
void
display_draw_string_centered(display_t *const display,
        unsigned int size,
        const char string[size],
        disp_area_t area,
        style_t style);

void
display_draw_string_aligned(display_t *const display,
        unsigned int size,
        const char string[size],
        disp_area_t area,
        style_t style,
        layout_align_t text_align);

void
display_render(display_t *const display);

void
display_render_area(display_t *const display,
        disp_area_t area);
void
display_clear_area(display_t *const display,
        disp_area_t area);

void 
display_set_resize_handler(display_t *const display,
                           resize_hook_with_data_t resize_hook);

void display_clear(display_t *const display);
bool disp_pos_equal(disp_pos_t a, disp_pos_t b);
disp_area_t normalized_area(disp_area_t area);
void display_erase(void);

#endif//_DISPLAY_H_
