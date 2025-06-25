#ifndef _INPUT_H_
#define _INPUT_H_

#include "circbuf.h"
#include "display.h"
#include "hashmap.h"
#include "logger.h"

#include <stddef.h>

#define INPUT_QUEUE_SIZE  4*1024 // 4kb
#define INPUT_BUFFER_SIZE 256

#ifndef ESC
#define ESC "\x1b"
#endif

#define EVENT_BUF_SIZE          8
#define MOUSE_EVENT_BUF_SIZE    3
#define MOUSE_EVENT_HEADER  ESC "[M"

#define MOUSE_EVENTS_ON     ESC "[?1003h"
#define MOUSE_EVENTS_OFF    ESC "[?1003l"

#define PASTE_MODE_ON       ESC "[?2004h"
#define PASTE_MODE_OFF      ESC "[?2004l"

#define MOUSE_OFFSET 0x20

typedef enum
{
    MOUSE_1, SCROLL_UP = MOUSE_1,
    MOUSE_2, SCROLL_DOWN = MOUSE_2,
    MOUSE_3,
    MOUSE_NONE
}
mouse_button_t;

typedef enum
{
    MOUSE_STATIC = 1,
    MOUSE_MOVING,
    MOUSE_SCROLLING
}
mouse_motion_t;

typedef enum
{
    MOD_NONE  = 0,
    MOD_SHIFT = 1,
    MOD_ALT   = 2,
    MOD_CTRL  = 4,
}
input_modifier_t;

typedef enum
{
    KEY_ESC = 0,
    KEY_F1,       KEY_F2,         KEY_F3,     KEY_F4,
    KEY_F5,       KEY_F6,         KEY_F7,     KEY_F8,
    KEY_F9,       KEY_F10,        KEY_F11,    KEY_F12,
    KEY_UP,       KEY_DOWN,       KEY_RIGHT,  KEY_LEFT,
    KEY_PAGE_UP,  KEY_PAGE_DOWN,  KEY_HOME,   KEY_END,
    KEY_RETURN,
    KEY_BACKSPACE,
    KEY_DELETE,
    KEY_INSERT,
    KEY_SPACE,
    KEY_TAB,
    KEY_BACKTICK,
    KEY_SEMICOLON,
    KEY_QUOTE,
    KEY_COMMA,
    KEY_PERIOD,
    KEY_SLASH,
    KEY_MINUS,
    KEY_PLUS,
    KEY_SQBR_OPEN,
    KEY_SQBR_CLOSE,
    KEY_BACK_SLASH,
    KEY_0,    KEY_1,    KEY_2,
    KEY_3,    KEY_4,    KEY_5,
    KEY_6,    KEY_7,    KEY_8,    KEY_9,
    KEY_A,    KEY_B,    KEY_C,    KEY_D,
    KEY_E,    KEY_F,    KEY_G,    KEY_H,
    KEY_I,    KEY_J,    KEY_K,    KEY_L,
    KEY_M,    KEY_N,    KEY_O,    KEY_P,
    KEY_Q,    KEY_R,    KEY_S,    KEY_T,
    KEY_U,    KEY_V,    KEY_W,    KEY_X,
    KEY_Y,    KEY_Z,
}
keycode_t;


typedef struct
{
    input_modifier_t modifier;
    keycode_t code;
    int stroke;
}
keystroke_event_t;


typedef struct
{
    input_modifier_t modifier;
    int mouse_button;
    mouse_motion_t motion;
    disp_pos_t position;
}
mouse_event_t;


typedef struct
{
    mouse_event_t prev_mouse_event;
    mouse_event_t last_mouse_event;
    mouse_event_t mouse_pressed;
    mouse_event_t mouse_released;
    bool drag;
}
mouse_mode_t;


typedef struct
{
    keystroke_event_t keystroke;
}
keystroke_mode_t;


typedef enum
{
/* MOUSE SEQUENCE                 */
    S0 = 0x0, /* 00000000         */
    S1,       /* 00000001 ESC     */
    S2,       /* 00000010 [       */
    S3,       /* 00000011 M       */
    S4,       /* 00000100 event   */
    S5,       /* 00000101 line    */
/*  S0           00000000 col     */

/* SPECIAL KEYS:                  */
/* PASTE SEQUENCE                 */
/*  S0           00000000         */
/*  S1           00000001 ESC     */
/*  S2           00000010 [       */
    S6 = 0x10,/* 00010000 2       */
    S7,       /* 00010001 0       */
    S8,       /* 00010010 0       */
    S9,       /* 00010011 ~       */
/*  S9           00010100 *       */  // * - means any character
    S10,      /* 00010101 ESC     */
    S11,      /* 00010110 [       */
    S12,      /* 00010111 2       */
    S13,      /* 00011000 0       */
    S14,      /* 00011001 1       */
/*  S0           00000000 ~       */

/*  S0           00000000         */
/*  S1           00000001 ESC     */
/* ARROW KEYS                     */
/*  S2           00000010 [       */
    S15 = 0x20,/*00100000 1       */
    S16,       /*00100001 ;       */
    S17,       /*00100010 mod     */
/*  S0           00000000 ~       */

/* OTHER NAV KEYS                 */
/*  S2           00000010 [       */
    S18,       /*00100011 5/6/H/F */
    S19,       /*00100100 ;       !optional */
    S20,       /*00100101 mod     !-------- */
/*  S0           00000000 ~       */
/* DELETE */
/*  S2           00000010 [       */
    S21,       /*00100110 3       */
    S22,       /*00100111 ;       !optional */
    S23,       /*00101000 mod     !-------- */
/*  S0           00000000 ~       */
/*  F-KEYS (F1-F4)                */
    S24,       /*00101001 O (4f)  */
    S25,       /*00101010 P/Q/R/S */
/*  S0           00000000 ~       */
/*  F-KEYS (F5-F8)                */
/*  S2           00000010 [       */
/*  S15 = 0x20,  00100000 1       */
/*  S21,         00100110 3       */
/*  S0           00000000 ~       */
}
istate_t;


typedef enum
{
    INPUT_SUCCESS = 0,
    INPUT_ERROR,
    INPUT_QUEUE_IS_FULL,
    INPUT_EXIT,
}
input_status_t;


typedef struct
{
    unsigned char state; // stores a value of istate_t 1 byte long
    bool escape_pressed;
}
input_sm_t;


typedef struct input
{
    unsigned char    event_buf[EVENT_BUF_SIZE];
    input_sm_t       state_machine;
    circbuf_t       *queue;
    mouse_mode_t     mouse_mode;
    keystroke_mode_t keystroke_mode;
    int              epfd; /* epoll file descriptor */
    hashmap_t       *descriptors; /* maps fd to a buffer that receives and outputs */
}
input_t;


typedef struct input_hooks
{
    void (*on_hover)(const mouse_event_t *const hover, void *const param);
    void (*on_press)(const mouse_event_t *const press, void *const param);
    void (*on_release)(const mouse_event_t *const press, void *const param);
    void (*on_drag_begin)(const mouse_event_t *const begin, void *const param);
    void (*on_drag)(const mouse_event_t *const begin, const mouse_event_t *const moved, void *const param);
    void (*on_drag_end)(const mouse_event_t *const begin,
        const mouse_event_t *const end, void *const param);
    void (*on_scroll)(const mouse_event_t *const scroll, void *const param);
    void (*on_keystroke)(const keystroke_event_t *const keystroke, void *const param);
}
input_hooks_t;


input_t input_init(void);
void input_deinit(input_t *const input);
void input_enable_mouse(void);
void input_disable_mouse(void);
int input_handle_events(input_t *const input, const input_hooks_t *const hooks, void *const param);
void input_display_overlay(input_t *const input, disp_pos_t pos);


#endif//_INPUT_H_
