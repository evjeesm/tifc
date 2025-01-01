#include "display.h"

#include <string.h>
#include <stdio.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <assert.h>

static int prev_buffer(const int active);
static bool disp_diff(const disp_char_t *const a, const disp_char_t *const b);
static void display_swap_buffers(display_t *const display);
static disp_pos_t get_terminal_size(void);
static void set_border(display_t *const display, wchar_t border_char, disp_pos_t pos, style_t style);

struct resize_handler
{
    bool resize_detected;
    resize_hook_with_data_t resize_hook;
}
volatile g_resize_handler;

static void resize_handler(int signo, siginfo_t *info, void *ctx)
{
    (void) signo; (void) info; (void) ctx;
    g_resize_handler.resize_detected = true;
}

void display_set_resize_handler(display_t *const display, resize_hook_with_data_t resize_hook)
{
    g_resize_handler.resize_hook = resize_hook;
    struct sigaction action = {0};
    action.sa_sigaction = resize_handler;
    sigaction(SIGWINCH, &action, NULL);
    display->size = get_terminal_size();
    printf(CLEAR);
}

void display_render(display_t *const display)
{
    disp_pos_t screen = get_terminal_size();
    disp_area_t screen_area = {
        .second = {
            .x = screen.x - 1,
            .y = screen.y - 1
        }
    };
    printf(SHOW_CURSOR);
    display_render_area(display, screen_area);

    fflush(stdout);
}

void display_render_area(display_t *const display, disp_area_t area)
{
    int prev = prev_buffer(display->active);
    dispbuf_ptr_t active = display->buffers[display->active];
    dispbuf_ptr_t previous = display->buffers[prev];

    bool force_reprint = false;
    if (g_resize_handler.resize_detected)
    {
        display->size = get_terminal_size();

        volatile resize_hook_with_data_t *resize_hook = &g_resize_handler.resize_hook;
        resize_hook->hook(display, resize_hook->data);

        force_reprint = true;
        g_resize_handler.resize_detected = false;
    }

    for (unsigned int line = area.first.y;
            line <= area.second.y && line < display->size.y;
            ++line)
    {
        for (unsigned int col = area.first.x;
                col <= area.second.x && col < display->size.x;
                ++col)
        {
            if (force_reprint
                || disp_diff(&active[line][col], &previous[line][col]))
            {
                if (active[line][col].style.seq)
                    printf("%s", active[line][col].style.seq);
                printf(ESC"[%d;%dH%lc", line + 1, col + 1, active[line][col].ch);
                printf(ESC RESET_STYLE);
            }
        }
    }

    display_swap_buffers(display);
}


void display_set_char(display_t *const display, wint_t ch, disp_pos_t pos)
{
    display->buffers[display->active][pos.y][pos.x].ch = ch;
}

void display_set_style(display_t *const display, style_t style, disp_pos_t pos)
{
    display->buffers[display->active][pos.y][pos.x].style = style;
}

void display_draw_border(display_t *const display, style_t style, border_set_t border, disp_area_t area)
{
    for (unsigned int y = area.first.y; y <= area.second.y; ++y)
    {
        for (unsigned int x = area.first.x; x <= area.second.x; ++x)
        {
            disp_pos_t pos = {x, y};
            if (x == area.first.x && y == area.first.y)
                set_border(display, border.top_left, pos, style);
            else if (x == area.second.x && y == area.first.y)
                set_border(display, border.top_right, pos, style);
            else if (x == area.second.x && y == area.second.y)
                set_border(display, border.bot_right, pos, style);
            else if (x == area.first.x && y == area.second.y)
                set_border(display, border.bot_left, pos, style);
            else if (x == area.first.x || x == area.second.x)
                set_border(display, border.vertical, pos, style);
            else if (y == area.first.y || y == area.second.y)
                set_border(display, border.horizontal, pos, style);
        }
    }
}

void display_fill_area(display_t *const display, style_t style, disp_area_t area)
{
    for (unsigned int y = area.first.y; y <= area.second.y; ++y)
    {
        for (unsigned int x = area.first.x; x <= area.second.x; ++x)
        {
            disp_pos_t pos = {x, y};
            display_set_style(display, style, pos);
            display_set_char(display, U' ', pos);
        }
    }
}

void display_draw_string(display_t *const display, unsigned int size, const char string[size], disp_pos_t pos, style_t style)
{
    for (unsigned int i = 0; i < size; ++i, ++pos.x)
    {
        display_set_char(display, string[i], pos);
        display_set_style(display, style, pos);
    }
}

void display_draw_string_centered(display_t *const display, unsigned int size, const char string[size], disp_area_t area, style_t style)
{
    assert(area.second.x <= display->size.x);
    assert(area.second.y <= display->size.y);
    unsigned int hmax = area.second.x - area.first.x;
    assert(size <= hmax);
    disp_pos_t pos = {
        .x = area.first.x + (hmax - size) / 2,
        .y = (area.first.y + area.second.y) / 2,
    };
    display_draw_string(display, size, string, pos, style);
}

static void set_border(display_t *const display, wchar_t border_char, disp_pos_t pos, style_t style)
{
    display_set_style(display, style, pos);
    display_set_char(display, border_char, pos);
}

void display_erase(void)
{
    printf(CLEAR);
}

void display_clear(display_t *const display)
{
    display_clear_area(display, (disp_area_t) {
        .first = {0, 0},
        .second = {
            display->size.x - 1,
            display->size.y - 1
        }
    });
}

void display_clear_area(display_t *const display, disp_area_t area)
{
    dispbuf_ptr_t active = display->buffers[display->active];

    for (unsigned int line = area.first.y;
            line <= area.second.y && line < display->size.y;
            ++line)
    {
        for (unsigned int col = area.first.x;
                col <= area.second.x && col < display->size.x;
                ++col)
        {
            active[line][col].style = (style_t){ 0 };
            active[line][col].ch = U' ';
        }
    }
}


bool disp_pos_equal(disp_pos_t a, disp_pos_t b)
{
    return a.x == b.x && a.y == b.y;
}


disp_area_t normalized_area(disp_area_t area)
{
    disp_pos_t top_left = area.first;
    disp_pos_t bottom_right = area.second;
    if (top_left.x > bottom_right.x)
    {
        unsigned int tmp = top_left.x;
        top_left.x = bottom_right.x;
        bottom_right.x = tmp;
    }
    if (top_left.y > bottom_right.y)
    {
        unsigned int tmp = top_left.y;
        top_left.y = bottom_right.y;
        bottom_right.y = tmp;
    }

    return (disp_area_t) {top_left, bottom_right};
}


static int prev_buffer(const int active)
{
    return (active + DISP_BUFFERS - 1) % DISP_BUFFERS;
}

static bool disp_diff(const disp_char_t *const a, const disp_char_t *const b)
{
    return memcmp(a, b, sizeof(disp_char_t));
}

static void display_swap_buffers(display_t *const display)
{
    display->active = (display->active + 1) % DISP_BUFFERS;
}

static disp_pos_t get_terminal_size(void)
{
    struct winsize w;
    ioctl(0, TIOCGWINSZ, &w);
    return (disp_pos_t){w.ws_col, w.ws_row};
}

