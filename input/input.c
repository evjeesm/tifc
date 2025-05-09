#include "input.h"
#include "display.h"
#include "hash.h"
#include "circbuf.h"
#include "logger.h"

#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/epoll.h>
#include <signal.h>
#include <assert.h>
#include <fcntl.h>
#include <ctype.h>

#define MAX_EVENTS 10
#define FEED_ERROR(ch) { S_LOG(LOGGER_CRITICAL, "Error: '%c' = (%x)\n", ch, ch); return INPUT_ERROR; }

typedef struct
{
    int temp;
}
buffer_t;

// global struct monitoring signals
typedef struct
{
    bool sigint;
}
signal_monitor_t;
static volatile signal_monitor_t s_sm = {0};

static void setup_signal_handlers(void);
static void handle_sigint(int sig, siginfo_t *info, void *ctx);
static void handle_mouse(input_t *const input, const input_hooks_t *const hooks, void *const param);
static void handle_keyboard(input_t *const input, const input_hooks_t *const hooks, void *const param);
static void handle_special_keys(input_t *const input, const input_hooks_t *const hooks, void *const param);
static void handle_navigation(input_t *const input, const input_hooks_t *const hooks, void *const param);
static mouse_event_t decode_mouse_event(unsigned char buffer[static 3]);
static void print_mouse_event(const mouse_event_t *const event);
static int input_read(input_t *const input);
static void input_on_timeout(input_t *const input, const input_hooks_t *const hooks, void *const param);
static int input_process(input_t *const input, const input_hooks_t *const hooks, void *const param);
static int input_feed(input_t *const input, const input_hooks_t *const hooks, void *const param, const char ch);

static bool is_upper(int ch);
static bool is_control_char(int ch);
static keycode_t map_ascii(char ch);
static keycode_t map_nav(int ch);
static keycode_t map_fk(int ch);

input_t input_init(void)
{
    int epfd = epoll_create1(0);
    if (-1 == epfd)
    {
        perror("epoll_create1");
        exit(EXIT_FAILURE);
    }
    struct epoll_event ev;
    ev.data.fd = STDIN_FILENO; // Monitor standard input
    ev.events = EPOLLIN;

    if (-1 == epoll_ctl(epfd, EPOLL_CTL_ADD, STDIN_FILENO, &ev))
    {
        perror("epoll_ctl: stdin");
        exit(EXIT_FAILURE);
    }
    hashmap_t *descriptors = hm_create(
        .hashfunc = hash_int,
        .key_size = sizeof(int),
        .value_size = sizeof(buffer_t),
    );
    if (!descriptors)
    {
        exit(EXIT_FAILURE);
    }
    circbuf_t *queue = circbuf_create(.initial_cap=INPUT_QUEUE_SIZE);
    if (!queue)
    {
        exit(EXIT_FAILURE);
    }

    setup_signal_handlers();

    return (input_t) {
        .queue = queue,
        .epfd = epfd,
        .descriptors = descriptors,
    };
}

void input_deinit(input_t *const input)
{
    hm_destroy(input->descriptors);
    circbuf_destroy(input->queue);
    close(input->epfd);
}

void input_enable_mouse(void)
{
    // disable canon mode and echo 
    struct termios attr;
    tcgetattr(STDIN_FILENO, &attr);
    attr.c_cc[VMIN] = 1; // Minimum number of characters to read
    attr.c_cc[VTIME] = 0; // No timeout
    attr.c_lflag &= ~(ICANON | ECHO | ISIG);
    tcsetattr(STDIN_FILENO, TCSANOW, &attr);
    printf(MOUSE_EVENTS_ON PASTE_MODE_ON);
    fflush(stdout);
}


void input_disable_mouse(void)
{
    // enable canon mode and echo 
    struct termios attr;
    tcgetattr(STDIN_FILENO, &attr);
    attr.c_lflag |= (ICANON | ECHO | ISIG);
    tcsetattr(STDIN_FILENO, TCSANOW, &attr);
    printf(MOUSE_EVENTS_OFF PASTE_MODE_OFF);
    fflush(stdout);
}


int input_handle_events(input_t *const input, const input_hooks_t *const hooks, void *const param)
{
    struct epoll_event events[MAX_EVENTS];
    int events_num = 0;
    // acquire events
    while (true)
    {
        events_num = epoll_wait(input->epfd, events, MAX_EVENTS, 10);
        if (events_num != -1)
        {
            break; // epoll succeded
        }
        else if (errno != EINTR) // something went wrong
        {
            perror("epoll_wait");
            return errno;
        }
        // Retry epoll_wait
    }

    if (!events_num)  input_on_timeout(input, hooks, param);

    for (int e = 0; e < events_num; e++)
    {
        if (events[e].events & EPOLLIN)
        {
            // process standard input
            if (events[e].data.fd == STDIN_FILENO)
            {
                // Data available from stdin
                int status = input_read(input);
                if (0 != status)
                {
                    return status;
                }

                // Try to process the data
                status = input_process(input, hooks, param);
                if (0 != status)
                {
                    return status;
                }
            }

            { // process buffers
                buffer_t *buffer = hm_get(input->descriptors, &events[e].data.fd);
                if (buffer)
                {
                    // write data to buffer
                }
            }
        }
    }
    return 0;
}


void input_display_overlay(input_t *const input, disp_pos_t pos)
{
    printf(ESC "[%d;%dH", pos.y, pos.x);
    print_mouse_event(&input->mouse_mode.last_mouse_event);
}


static int input_read(input_t *input)
{
    const size_t available_space = circbuf_avail_to_write(input->queue);
    if (available_space == 0)
    {
        return 1;  // queue is full of unprocessed data
    }

    const size_t to_read = available_space < INPUT_BUFFER_SIZE
        ? available_space
        : INPUT_BUFFER_SIZE;

    unsigned char buffer[INPUT_BUFFER_SIZE] = {0};
    int input_bytes = read(STDIN_FILENO, buffer, to_read);

    if (input_bytes == 0)
    {
        return 0; // nothing to decode
    }

    if (input_bytes == -1)
    {
        int error = errno;
        perror("read from stdin");
        return error; // just propagate error code for now
    }

    // send input into the queue
    (void) circbuf_write(input->queue, input_bytes, buffer);
    return 0;
}


static void input_on_timeout(input_t *const input, const input_hooks_t *const hooks, void *const param)
{
    input_sm_t *sm = &input->state_machine;
    keystroke_event_t *ke = &input->keystroke_mode.keystroke;
    if (sm->escape_pressed && '\x1b' == ke->stroke)
    {
        sm->state = S0;
        ke->code = KEY_ESC;
        handle_special_keys(input, hooks, param);
    }
    // S_LOG(LOGGER_DEBUG, "TIMEOUT!");
    input->state_machine.escape_pressed = false;
}


static int input_process(input_t *const input, const input_hooks_t *const hooks, void *const param)
{
    unsigned char buffer[INPUT_BUFFER_SIZE];
    const size_t available = circbuf_avail_to_read(input->queue);
    const size_t to_process = available < INPUT_BUFFER_SIZE
        ? available
        : INPUT_BUFFER_SIZE;

    const size_t bytes = circbuf_read(input->queue, to_process, &buffer);
    for (size_t i = 0; i < bytes ; ++i)
    {
        int status = input_feed(input, hooks, param, buffer[i]);
        if (status) return status;
    }
    return INPUT_SUCCESS;
}


static int input_feed(input_t *const input, const input_hooks_t *const hooks,
        void *const param, const char ch)
{
    input_sm_t *sm = &input->state_machine;
    keystroke_event_t *ke = &input->keystroke_mode.keystroke;

    ke->stroke = ch;

    switch (sm->state)
    {
        case S0: S_LOG(LOGGER_DEBUG, "\n");
            ke->modifier = 0; /* reset modifier */
            switch (ch)
            {
                case '\x1b':S_LOG(LOGGER_DEBUG, "ESC ");
                            sm->state = S1;
                            sm->escape_pressed = true;
                            break;

                case '\x7f':S_LOG(LOGGER_DEBUG, "BACKSPACE\n");
                            sm->state = S0;
                            ke->code = KEY_BACKSPACE;
                            handle_special_keys(input, hooks, param);
                            break;

                default:    sm->state = S0;
                            ke->code = map_ascii(ch);
                            ke->modifier |= (is_upper(ch) ? MOD_SHIFT : 0);
                            ke->modifier |= (is_control_char(ch) ? MOD_CTRL : 0);
                            handle_keyboard(input, hooks, param);
            }
            break;

        case S1:
            switch (ch)
            {
                case '\x1b':S_LOG(LOGGER_DEBUG, "ESC ");
                            sm->state = S0;
                            ke->code = KEY_ESC;
                            break; /* escape pressed */

                case '[':   S_LOG(LOGGER_DEBUG, "[ ");
                            sm->state = S2;
                            break;

                /* F1 - F4, no mod */
                case 'O':   S_LOG(LOGGER_DEBUG, "O ");
                            sm->state = S24;
                            break;

                default:    sm->state = S0;
                            ke->code = map_ascii(ch);
                            ke->modifier |= MOD_ALT;
                            ke->modifier |= (is_upper(ch) ? MOD_SHIFT : 0);
                            ke->modifier |= (is_control_char(ch) ? MOD_CTRL : 0);
                            // S_LOG(LOGGER_DEBUG, "ALT char: '%c'\n", ch);
                            handle_keyboard(input, hooks, param);
            }
            break;

        case S2:
            switch (ch)
            {
                case '0':   S_LOG(LOGGER_DEBUG, "0 ");
                            sm->state = S0;
                            break;

                case '1':   S_LOG(LOGGER_DEBUG, "1 ");
                            sm->state = S15;
                            break;

                case '2':   S_LOG(LOGGER_DEBUG, "2 ");
                            sm->state = S6;
                            break;

                case '3':   S_LOG(LOGGER_DEBUG, "3 ");
                            ke->code = KEY_DELETE;
                            sm->state = S21;
                            break;

                case '5': case '6':
                            ke->code = map_nav(ch);
                            if (!ke->code) return INPUT_ERROR;
                            sm->state = S18;
                            break;

                case 'M':   sm->state = S3;
                            break;

                case 'A': case 'B': case 'C':
                case 'D': case 'F': case 'H':
                            ke->code = map_nav(ch);
                            if (!ke->code) return INPUT_ERROR;
                            handle_navigation(input, hooks, param);
                            sm->state = S0;
                            break;
                default:
                            ke->code = map_ascii(ch);
                            handle_keyboard(input, hooks, param);
                            sm->state = S0;
            }
            break;

        /* MOUSE EVENT PROCESSING */
        case S3:
                    sm->state = S4;
                    input->event_buf[0] = ch;
                    break;
        case S4:
                    sm->state = S5;
                    input->event_buf[1] = ch;
                    break;
        case S5:
                    sm->state = S0;
                    input->event_buf[2] = ch;
                    handle_mouse(input, hooks, param);
                    break;

        /* PASTE SEQUENCE */
        case S6:
            switch (ch)
            {
                case '0':   S_LOG(LOGGER_DEBUG, "0 ");
                            sm->state = S7;
                            break;
 
                case '1': case '3': case '4':
                            S_LOG(LOGGER_DEBUG, "%c ", ch);
                            ke->code = map_fk(ch);
                            sm->state = S21;
                            break;

                case '~':   ke->code = KEY_INSERT;
                            sm->state = S0;
                            handle_special_keys(input, hooks, param);
                            break;

                default:    FEED_ERROR(ch);
            }
            break;

        case S7:
            switch (ch)
            {
                case '0':   S_LOG(LOGGER_DEBUG, "0 ");
                            sm->state = S8;
                            break;

                /* Edge case for <F9> { */
                case ';':   ke->code = map_fk('0');
                            sm->state = S22;
                            break;

                case '~':   S_LOG(LOGGER_DEBUG, "~\n");
                            ke->code = map_fk('0');
                            sm->state = S0;
                            handle_special_keys(input, hooks, param);
                            break;
                /* } */
                default:    FEED_ERROR(ch);
            }
            break;

        case S8:
            switch (ch)
            {
                case '~':   S_LOG(LOGGER_DEBUG, "~\n");
                            sm->state = S9;
                            break;
                default:    FEED_ERROR(ch);
            }
            break;

        case S9:
            switch (ch)
            {
                case '\x1b':S_LOG(LOGGER_DEBUG, "\nESC ");
                            sm->state = S10; 
                            break;
                default:    S_LOG(LOGGER_DEBUG, "%x", ch); // TODO: store pasted text
            }
            break;

        case S10:
            switch (ch)
            {
                case '[':   S_LOG(LOGGER_DEBUG, "[ ");
                            sm->state = S11;
                            break;
                default:    FEED_ERROR(ch);
            }
            break;

        case S11:
            switch (ch)
            {
                case '2':   S_LOG(LOGGER_DEBUG, "2 ");
                            sm->state = S12;
                            break;
                default:    FEED_ERROR(ch);
            }
            break;

        case S12:
            switch (ch)
            {
                case '0':   S_LOG(LOGGER_DEBUG, "0 ");
                            sm->state = S13;
                            break;
                default:    FEED_ERROR(ch);
            }
            break;

        case S13:
            switch (ch)
            {
                case '1':   S_LOG(LOGGER_DEBUG, "1 ");
                            sm->state = S14;
                            break;
                default:    FEED_ERROR(ch);
            }
            break;

        case S14:
            switch (ch)
            {
                case '~':   S_LOG(LOGGER_DEBUG, "~\n");
                            sm->state = S0;
                            S_LOG(LOGGER_DEBUG, "PASTE_END\n");/* TODO confirm paste */
                            break;
                default:    FEED_ERROR(ch);
            }
            break;

        case S15:
            switch (ch)
            {
                case ';':   S_LOG(LOGGER_DEBUG, "; ");
                            sm->state = S16;
                            // handle_keyboard(input, hooks, param);
                            break;

                case '5': case '7': case '8': case '9':
                            S_LOG(LOGGER_DEBUG, "%c ", ch);
                            ke->code = map_fk(ch);
                            sm->state = S21;
                            break;

                default:    FEED_ERROR(ch);
            }
            break;

        case S16:   sm->state = S17;
                    ke->modifier = (ch - 0x31) & 0x7;  /*3 bits*/
                    S_LOG(LOGGER_DEBUG, "mod(%d) ", ke->modifier);
                    break;

        case S17:
            switch (ch)
            {
                case 'P': case 'Q': case 'R': case 'S':
                            ke->code = map_fk(ch);
                            if (!ke->code) { return INPUT_ERROR; }

                            handle_special_keys(input, hooks, param);
                            break;

                default:    ke->code = map_nav(ch);
                            if (!ke->code) { return INPUT_ERROR; }
                            handle_navigation(input, hooks, param);
            }
            sm->state = S0;
            break;

        case S18:
            switch (ch)
            {
                case ';':   S_LOG(LOGGER_DEBUG, "; ");
                            sm->state = S19;
                            break;

                case '~':   S_LOG(LOGGER_DEBUG, "~\n");
                            sm->state = S0;
                            // S_LOG(LOGGER_DEBUG, "[%c %x]\n", ch, ch);
                            handle_navigation(input, hooks, param);
                            break;

                default:    FEED_ERROR(ch);
            }
            break;

        case S19:
            sm->state = S20;
            ke->modifier = (ch - 0x31) & 0x7;  /*3 bits*/
            S_LOG(LOGGER_DEBUG, "mod(%d) ", ke->modifier);
            break;

        case S20:
            switch (ch)
            {
                case '~':   S_LOG(LOGGER_DEBUG, "~\n");
                            sm->state = S0;
                            handle_navigation(input, hooks, param);
                            break;

                default:    FEED_ERROR(ch);
            }
            break;

        case S21:
            switch (ch)
            {
                case ';':   S_LOG(LOGGER_DEBUG, "; ");
                            sm->state = S22;
                            break;

                case '~':   S_LOG(LOGGER_DEBUG, "~\n");
                            sm->state = S0;
                            handle_special_keys(input, hooks, param);
                            break;

                default:    FEED_ERROR(ch);
            }
            break;

        case S22:
            sm->state = S21;
            ke->modifier = (ch - 0x31) & 0x7;  /*3 bits*/
            S_LOG(LOGGER_DEBUG, "mod(%d) ", ke->modifier);
            break;

        case S23:
            switch (ch)
            {
                case '~':   S_LOG(LOGGER_DEBUG, "~\n");
                            sm->state = S0;
                            handle_special_keys(input, hooks, param);
                            // handle_special_keys(input, hooks, param);
                            break;

                default:    FEED_ERROR(ch);
            }
            break;

        case S24:
            ke->code = map_fk(ch);
            if (!ke->code) { return INPUT_ERROR; }
            sm->state = S0;
            handle_special_keys(input, hooks, param);
            break;

        default:    FEED_ERROR(ch);
    }

    return INPUT_SUCCESS;
}


static void handle_mouse(input_t *const input, const input_hooks_t *const hooks, void *const param)
{
    mouse_mode_t *mouse_mode = &input->mouse_mode;
    mouse_event_t event = decode_mouse_event(input->event_buf);
    // print_mouse_event(&event);

    mouse_mode->prev_mouse_event = mouse_mode->last_mouse_event;
    mouse_mode->last_mouse_event = event;

    const mouse_event_t *const prev = &mouse_mode->prev_mouse_event;
    const mouse_event_t *const last = &mouse_mode->last_mouse_event;

    if ( (MOUSE_STATIC == prev->motion || MOUSE_MOVING == prev->motion)
        && MOUSE_NONE == prev->mouse_button
        && MOUSE_NONE != last->mouse_button)
    {
        mouse_mode->mouse_pressed = *last;
        hooks->on_press(&mouse_mode->mouse_pressed, param);
    }

    if ( MOUSE_STATIC == prev->motion
        && MOUSE_NONE != prev->mouse_button)
    {
        if ( MOUSE_MOVING == last->motion
            && MOUSE_NONE != last->mouse_button)
        {
            mouse_mode->drag = true;
            hooks->on_drag_begin(&mouse_mode->mouse_pressed, param);
        }
    }

    if (mouse_mode->drag)
    {
        hooks->on_drag(&mouse_mode->mouse_pressed, &mouse_mode->last_mouse_event, param);
    }
    else if (MOUSE_MOVING == last->motion)
    {
        hooks->on_hover(&mouse_mode->last_mouse_event, param);
    }

    if ( (MOUSE_STATIC == prev->motion || MOUSE_MOVING == prev->motion)
        && MOUSE_NONE != prev->mouse_button )
    {
        if ( MOUSE_STATIC == last->motion
            && MOUSE_NONE == last->mouse_button)
        {
            if (!mouse_mode->drag)
            {
                hooks->on_release(&mouse_mode->mouse_pressed, param);
            }
            else
            {
                mouse_mode->drag = false;
                hooks->on_drag_end(&mouse_mode->mouse_pressed, &mouse_mode->mouse_released, param);
            }
            mouse_mode->mouse_released = *last;
        }
    }

    if (MOUSE_SCROLLING == last->motion)
    {
        hooks->on_scroll(last, param);
    }
}


static void handle_keyboard(input_t *const input, const input_hooks_t *const hooks, void *const param)
{
    hooks->on_keystroke(&input->keystroke_mode.keystroke, param);
}


static void handle_special_keys(input_t *const input, const input_hooks_t *const hooks, void *const param)
{
    (void) input; (void) hooks; (void) param;
    switch ((int)input->keystroke_mode.keystroke.code)
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
        case KEY_DELETE: S_LOG(LOGGER_DEBUG, "DELETE mod(%d)\n",
                                 input->keystroke_mode.keystroke.modifier);
    }}


static void handle_navigation(input_t *const input, const input_hooks_t *const hooks, void *const param)
{
    (void) input; (void) hooks; (void) param;
    switch ((int)input->keystroke_mode.keystroke.code)
    {
        case KEY_UP: S_LOG(LOGGER_DEBUG, "UP mod(%d)\n",
                    input->keystroke_mode.keystroke.modifier);
            break;
        case KEY_DOWN: S_LOG(LOGGER_DEBUG, "DOWN mod(%d)\n",
                    input->keystroke_mode.keystroke.modifier);
            break;
        case KEY_RIGHT: S_LOG(LOGGER_DEBUG, "RIGHT mod(%d)\n",
                    input->keystroke_mode.keystroke.modifier);
            break;
        case KEY_LEFT: S_LOG(LOGGER_DEBUG, "LEFT mod(%d)\n",
                    input->keystroke_mode.keystroke.modifier);
            break; 
        case KEY_HOME: S_LOG(LOGGER_DEBUG, "HOME mod(%d)\n",
                    input->keystroke_mode.keystroke.modifier);
            break;
        case KEY_END: S_LOG(LOGGER_DEBUG, "END mod(%d)\n",
                    input->keystroke_mode.keystroke.modifier);
            break;
        case KEY_PAGE_UP: S_LOG(LOGGER_DEBUG, "PAGE_UP mod(%d)\n",
                    input->keystroke_mode.keystroke.modifier);
            break;
        case KEY_PAGE_DOWN: S_LOG(LOGGER_DEBUG, "PAGE_DOWN mod(%d)\n",
                    input->keystroke_mode.keystroke.modifier);
            break;
    }
}


static mouse_event_t decode_mouse_event(unsigned char buffer[static 3])
{
    mouse_event_t event = {
        .mouse_button = buffer[0] & 0x3,    /*2 bits*/
        .modifier = (buffer[0] >> 2) & 0x7, /*3 bits*/
        .motion = (buffer[0] >> 5) & 0x3,   /*2 bits*/
        .position = {
            (buffer[1] - MOUSE_OFFSET) - 1, /* convert to zero-based */
            (buffer[2] - MOUSE_OFFSET) - 1, /* convert to zero-based */
        },
    };

    return event;
}


static void print_mouse_event(const mouse_event_t *const event)
{
    S_LOG(LOGGER_DEBUG, "MOUSE_EVENT:\nbutton: %u   \n"
        "mod:[shift: %u, alt: %u, ctrl: %u]   \n"
        "motion: %s, x: %d, y: %d)   \n",
        event->mouse_button,
        (event->modifier)      & 0x1,
        (event->modifier >> 1) & 0x1,
        (event->modifier >> 2) & 0x1,
        event->motion == MOUSE_STATIC ? "static" :
        event->motion == MOUSE_MOVING ? "moving" : "scroll",
        event->position.x, event->position.y);
}


static void setup_signal_handlers(void)
{
    // Set sigint handler
    struct sigaction sa;
    sa.sa_sigaction = (void*) SIG_IGN; (void) handle_sigint;
    sa.sa_flags = 0; // No special flags
    sigemptyset(&sa.sa_mask);
    sigaction(SIGINT, &sa, NULL);
}

// Signal handler for SIGINT (Ctrl+C)
static void handle_sigint(int sig, siginfo_t *info, void *ctx)
{
    (void) sig; (void) info; (void) ctx;
    s_sm.sigint = true;
}


static bool is_upper(int ch)
{
    return ('A' <= ch && ch <= 'Z');
}


static bool is_control_char(int ch)
{
    return (ch < '0' || ch >= 0x7f/*BACKSPACE*/);
}


static keycode_t map_ascii(char ch)
{
    ch = tolower(ch);
    if ('0' <= ch && ch <= '9')   return KEY_O + (ch - '0');
    if ('a' <= ch && ch <= 'z')   return KEY_A + (ch - 'a');
    if ('\x1b' == ch)             return KEY_ESC;
    if ('\n' == ch)               return KEY_RETURN;
    if ('\x7f' == ch)             return KEY_BACKSPACE;
    if (' ' == ch)                return KEY_SPACE;
    if ('\t' == ch)               return KEY_TAB;
    if ('~' == ch || '`' == ch)   return KEY_BACKTICK;
    if (':' == ch || ';' == ch)   return KEY_SEMICOLON;
    if ('"' == ch || '\''== ch)   return KEY_QUOTE;
    if ('<' == ch || ',' == ch)   return KEY_COMMA;
    if ('>' == ch || '.' == ch)   return KEY_PERIOD;
    if ('/' == ch || '?' == ch)   return KEY_SLASH;
    if ('-' == ch || '_' == ch)   return KEY_MINUS;
    if ('=' == ch || '+' == ch)   return KEY_PLUS;
    if ('[' == ch || '{' == ch)   return KEY_SQBR_OPEN;
    if (']' == ch || '}' == ch)   return KEY_SQBR_CLOSE;
    if ('\\' == ch || '|' == ch)  return KEY_BACK_SLASH;
    if ('!' == ch)                return KEY_1;
    if ('@' == ch)                return KEY_2;
    if ('#' == ch)                return KEY_3;
    if ('$' == ch)                return KEY_4;
    if ('%' == ch)                return KEY_5;
    if ('^' == ch)                return KEY_6;
    if ('&' == ch)                return KEY_7;
    if ('*' == ch)                return KEY_8;
    if ('(' == ch)                return KEY_9;
    if (')' == ch)                return KEY_0;
    return 0;
}


static keycode_t map_nav(int ch)
{
    switch (ch)
    {
        case '5': S_LOG(LOGGER_DEBUG, "PAGE UP ");  return KEY_PAGE_UP;
        case '6': S_LOG(LOGGER_DEBUG, "PAGE DOWN ");  return KEY_PAGE_DOWN;
        case 'A': S_LOG(LOGGER_DEBUG, "UP ");  return KEY_UP;
        case 'B': S_LOG(LOGGER_DEBUG, "DOWN ");  return KEY_DOWN;
        case 'C': S_LOG(LOGGER_DEBUG, "RIGHT ");  return KEY_RIGHT;
        case 'D': S_LOG(LOGGER_DEBUG, "LEFT ");  return KEY_LEFT;
        case 'H': S_LOG(LOGGER_DEBUG, "HOME");  return KEY_HOME;
        case 'F': S_LOG(LOGGER_DEBUG, "END");  return KEY_END;
    }
    S_LOG(LOGGER_CRITICAL, "PARSE ERROR: wrong nav key!\n");
    return 0;
}


static keycode_t map_fk(int ch)
{
    switch (ch)
    {
        case '\x1b':return KEY_ESC;
        case 'P':   return KEY_F1;
        case 'Q':   return KEY_F2;
        case 'R':   return KEY_F3;
        case 'S':   return KEY_F4;
        case '5':   return KEY_F5;
        case '7':   return KEY_F6;
        case '8':   return KEY_F7;
        case '9':   return KEY_F8;
        case '0':   return KEY_F9;
        case '1':   return KEY_F10;
        case '3':   return KEY_F11;
        case '4':   return KEY_F12;
    }
    S_LOG(LOGGER_CRITICAL, "PARSE ERROR: wrong function key!\n");
    return 0;
}
