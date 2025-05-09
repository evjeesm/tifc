#include "input.h"

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

static void on_hover(const mouse_event_t *const hover, void *const param)
{
    (void) param;
    printf("hover at %u, %u\n", hover->position.x, hover->position.y);
}

static void on_press(const mouse_event_t *const press, void *const param)
{
    (void) param;
    printf("press %d, at %u, %u\n",
        press->mouse_button,
        press->position.x, press->position.y);
}

static void on_release(const mouse_event_t *const press, void *const param)
{
    (void) param;
    printf("release %d, at %u, %u\n",
        press->mouse_button,
        press->position.x, press->position.y);
}

static void on_drag_begin(const mouse_event_t *const drag_begin,
        void *const param)
{
    (void) param;
    printf("drag %d begin, at %u, %u\n",
        drag_begin->mouse_button,
        drag_begin->position.x, drag_begin->position.y);
}

static void on_drag(const mouse_event_t *const begin,
        const mouse_event_t *const moved, void *const param)
{
    (void) param;
    printf("drag %d at %u, %u\n",
        begin->mouse_button,
        moved->position.x, moved->position.y);
}

static void on_drag_end(const mouse_event_t *const drag_begin,
        const mouse_event_t *const drag_end, void *const param)
{
    (void) param;
    printf("drag end %d from %u, %u to %u, %u\n",
        drag_begin->mouse_button,
        drag_begin->position.x, drag_begin->position.y,
        drag_end->position.x, drag_end->position.y);
}

static void on_scroll(const mouse_event_t *const scroll, void *const param)
{
    (void) param;
    printf("scroll %d at %u, %u\n",
        scroll->mouse_button,
        scroll->position.x, scroll->position.y);
}

static void on_keystroke(const keystroke_event_t *const keystroke, void *const param)
{
    (void) param;
    // Check for Ctrl+D
    if (keystroke->stroke == '\x04')
    {
        printf("\nEOF detected. Exiting...\n");
        exit(EXIT_SUCCESS);
    }
    // Check for Ctrl+C
    if (keystroke->stroke == '\x03')
    {
        printf("\nCTRL+C detected. Exiting...\n");
        exit(EXIT_SUCCESS);
    }
    printf("keystroke mod(%d), ch: '%c' (%x)\n", keystroke->modifier, keystroke->stroke, keystroke->stroke);
}


static void on_navigation(const keystroke_event_t *const keystroke, void *const param)
{
    (void) param;
    switch ((int)keystroke->code)
    {
        case KEY_UP:        S_LOG(LOGGER_DEBUG, "UP"); break;
        case KEY_DOWN:      S_LOG(LOGGER_DEBUG, "DOWN"); break;
        case KEY_RIGHT:     S_LOG(LOGGER_DEBUG, "RIGHT"); break;
        case KEY_LEFT:      S_LOG(LOGGER_DEBUG, "LEFT"); break;
        case KEY_HOME:      S_LOG(LOGGER_DEBUG, "HOME"); break;
        case KEY_END:       S_LOG(LOGGER_DEBUG, "END"); break;
        case KEY_PAGE_UP:   S_LOG(LOGGER_DEBUG, "PAGE_UP"); break;
        case KEY_PAGE_DOWN: S_LOG(LOGGER_DEBUG, "PAGE_DOWN"); break;
    }
    S_LOG(LOGGER_DEBUG, " mod(%d)\n", keystroke->modifier);
}


static void on_special_key(const keystroke_event_t *const keystroke, void *const param)
{
    (void) param;
    switch ((int)keystroke->code)
    {
        case KEY_ESC: S_LOG(LOGGER_DEBUG, "ESC\n"); break;
        case KEY_F1:  S_LOG(LOGGER_DEBUG, "F1\n"); break;
        case KEY_F2:  S_LOG(LOGGER_DEBUG, "F2\n"); break;
        case KEY_F3:  S_LOG(LOGGER_DEBUG, "F3\n"); break;
        case KEY_F4:  S_LOG(LOGGER_DEBUG, "F4\n"); break;
        case KEY_F5:  S_LOG(LOGGER_DEBUG, "F5\n"); break;
        case KEY_F6:  S_LOG(LOGGER_DEBUG, "F6\n"); break;
        case KEY_F7:  S_LOG(LOGGER_DEBUG, "F7\n"); break;
        case KEY_F8:  S_LOG(LOGGER_DEBUG, "F8\n"); break;
        case KEY_F9:  S_LOG(LOGGER_DEBUG, "F9\n"); break;
        case KEY_F10: S_LOG(LOGGER_DEBUG, "F10\n"); break;
        case KEY_F11: S_LOG(LOGGER_DEBUG, "F11\n"); break;
        case KEY_F12: S_LOG(LOGGER_DEBUG, "F12\n"); break;
        case KEY_INSERT: S_LOG(LOGGER_DEBUG, "INSERT\n"); break;
        case KEY_BACKSPACE: S_LOG(LOGGER_DEBUG, "BACKSPACE\n"); break;
        case KEY_DELETE: S_LOG(LOGGER_DEBUG, "DELETE mod(%d)\n", keystroke->modifier);
    }
}

int main(void)
{
    input_hooks_t hooks = {
        .on_hover = on_hover,
        .on_press = on_press,
        .on_release = on_release,
        .on_drag_begin = on_drag_begin,
        .on_drag = on_drag,
        .on_drag_end = on_drag_end,
        .on_scroll = on_scroll,
        .on_keystroke = on_keystroke,
        .on_special_key = on_special_key,
        .on_navigation = on_navigation,
    };
    input_enable_mouse();
    input_t input = input_init();
    while (1)
    {
        int status = input_handle_events(&input, &hooks, NULL);
        if (status) break;
    }

    input_deinit(&input);
    input_disable_mouse();
}
