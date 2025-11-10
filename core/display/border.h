#ifndef _BORDER_H_
#define _BORDER_H_

#include <wchar.h>

#define BORDER_STYLE_1 ((style_t){ .seq = ESC"[91m" })
#define BORDER_STYLE_2 ((style_t){ .seq = ESC"[5;38;5;16;48;5;73m" })
#define BORDER_STYLE_3 ((style_t){ .seq = ESC"[6;38;5;202;48;5;23m" })
#define BORDER_STYLE_4 ((style_t){ .seq = ESC"[31m" })

#define BORDER_SET_SIZE 6
typedef struct
{
    union {
        struct {
            wchar_t top_left;
            wchar_t top_right;
            wchar_t bot_right;
            wchar_t bot_left;
            wchar_t vertical;
            wchar_t horizontal;
        };
        wchar_t _[BORDER_SET_SIZE + 1];
    };
}
border_set_t;

#endif /*_BORDER_H_*/
