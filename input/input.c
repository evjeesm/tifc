#include "input.h"
#include "display.h"
#include "hash.h"
#include "circbuf.h"

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

#define INPUT_PROCESS_BUF 1024
#define MAX_EVENTS 10

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
static int handle_keyboard(input_t *const input, char ch);
static mouse_event_t decode_mouse_event(unsigned char buffer[static 3]);
static void print_mouse_event(const mouse_event_t *const event);
static int input_read(input_t *const input);
static int input_process(input_t *const input, const input_hooks_t *const hooks, void *const param);
static int input_feed(input_t *const input, const input_hooks_t *const hooks, void *const param, const char ch);

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

    for (int e = 0; e < events_num; e++)
    {
        if (events[e].events & EPOLLIN)
        {
            // process standard input
            if (events[e].data.fd == STDIN_FILENO)
            {
                // Data available from stdin
                int status = input_read(input);
    }

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


static int input_process(input_t *const input, const input_hooks_t *const hooks, void *const param)
{
    unsigned char buffer[INPUT_PROCESS_BUF];
    const size_t available = circbuf_avail_to_read(input->queue);
    const size_t to_process = available < INPUT_BUFFER_SIZE
        ? available
        : INPUT_PROCESS_BUF;

    const size_t bytes = circbuf_read(input->queue, to_process, &buffer);
    for (size_t i = 0; i < bytes ; ++i)
    {
        int status = input_feed(input, hooks, param, buffer[i]);
        if (status) return status;
    }
    return INPUT_SUCCESS;
}


static void handle_mouse(input_t *const input, const input_hooks_t *const hooks, void *const param)
{
    mouse_mode_t *mouse_mode = &input->mouse_mode;
    mouse_event_t event = decode_mouse_event(input->mouse_mode.event_buf);

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


static int handle_keyboard(input_t *const input, char ch)
{
    (void) input;
    printf(ROW(3) "input: %c %#x        \n", ch, (int)ch);
    // Check for Ctrl+D
    if (ch == '\x04')
    {
        printf("\nEOF detected. Exiting...\n");
        return 1;
    }
    return 0;
}

static mouse_event_t decode_mouse_event(unsigned char buffer[static 3])
{
    mouse_event_t event = {
        .mouse_button = buffer[0] & 0x3    /*2 bits*/,
        .modifier = (buffer[0] >> 2) & 0x7 /*3 bits*/,
        .motion = (buffer[0] >> 5) & 0x3   /*2 bits*/,
        .position = {
            (buffer[1] - MOUSE_OFFSET) - 1, /* convert to zero-based */
            (buffer[2] - MOUSE_OFFSET) - 1, /* convert to zero-based */
        },
    };

    return event;
}


static int input_feed(input_t *const input, const input_hooks_t *const hooks, void *const param, const char ch)
{
    input_sm_t *sm = &input->state_machine;
    switch (sm->state)
    {
        case S0: switch (ch)
        {
            case '\x1b':        sm->state = S1;  break;
            default:            handle_keyboard(input, ch);
        } break;
        case S1: switch (ch)
        {
            case '\x1b':        sm->state = S0;  break; /* escape pressed */
            case '[':           sm->state = S2;  break;
            default:            return INPUT_ERROR;
        } break;
        case S2: switch (ch)
        {
            case 'M':           sm->state = S3;  break;
            case '2':           sm->state = S6;  break;
            default:            return INPUT_ERROR;
        } break;

        /* MOUSE EVENT PROCESSING */
        case S3:
                                sm->state = S4;
                                input->mouse_mode.event_buf[0] = ch;
        break;
        case S4:
                                sm->state = S5;
                                input->mouse_mode.event_buf[1] = ch;
        break;
        case S5:
                                sm->state = S0;
                                input->mouse_mode.event_buf[2] = ch;
                                handle_mouse(input, hooks, param);
        break;

        /* PASTE SEQUENCE */
        case S6: switch (ch)
        {
            case '0':           sm->state = S7;
            break;
            default:            return INPUT_ERROR;
        } break;

        case S7: switch (ch)
        {
            case '0':           sm->state = S8;  break;
            default:            return INPUT_ERROR;
        } break;

        case S8: switch (ch)
        {
            case '~':           sm->state = S9;  break;
            default:            return INPUT_ERROR;
        } break;

        case S9: switch (ch)
        {
            case '\x1b':        sm->state = S10; break;
            default: ; // TODO: store pasted text
        } break;

        case S10: switch (ch)
        {
            case '[':           sm->state = S11; break;
            default:            return INPUT_ERROR;
        } break;

        case S11: switch (ch)
        {
            case '2':           sm->state = S12; break;
            default:            return INPUT_ERROR;
        } break;

        case S12: switch (ch)
        {
            case '0':           sm->state = S13; break;
            default:            return INPUT_ERROR;
        } break;

        case S13: switch (ch)
        {
            case '1':           sm->state = S14; break;
            default:            return INPUT_ERROR;
        } break;

        case S14: switch (ch)
        {
            case '~':           sm->state = S0;
                                /* TODO confirm paste */ break;
            default:            return INPUT_ERROR;
        } break;

        // case S15:

        default:                return INPUT_ERROR;
    }

    return INPUT_SUCCESS;
}


static void print_mouse_event(const mouse_event_t *const event)
{
    printf("MOUSE_EVENT:\nbutton: %u   \n"
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
    sa.sa_sigaction = handle_sigint;
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
