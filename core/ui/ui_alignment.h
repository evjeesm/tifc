#ifndef _UI_ALIGNMENT_H_
#define _UI_ALIGNMENT_H_

typedef enum
{
    UI_ALIGNMENT_TOP            = 1 << 0,
    UI_ALIGNMENT_BOT            = 1 << 1,
    UI_ALIGNMENT_LEFT           = 1 << 2,
    UI_ALIGNMENT_RIGHT          = 1 << 3,

    UI_ALIGNMENT_TOP_H_CENTER   = UI_ALIGNMENT_TOP
                                | UI_ALIGNMENT_LEFT
                                | UI_ALIGNMENT_RIGHT,

    UI_ALIGNMENT_BOT_H_CENTER   = UI_ALIGNMENT_BOT
                                | UI_ALIGNMENT_LEFT
                                | UI_ALIGNMENT_RIGHT,

    UI_ALIGNMENT_LEFT_V_CENTER  = UI_ALIGNMENT_LEFT
                                | UI_ALIGNMENT_TOP
                                | UI_ALIGNMENT_BOT,

    UI_ALIGNMENT_RIGHT_V_CENTER = UI_ALIGNMENT_RIGHT
                                | UI_ALIGNMENT_TOP
                                | UI_ALIGNMENT_BOT,

    UI_ALIGNMENT_CENTER         = UI_ALIGNMENT_LEFT
                                | UI_ALIGNMENT_RIGHT
                                | UI_ALIGNMENT_TOP
                                | UI_ALIGNMENT_BOT,
}
ui_alignment_t;

#endif /* _UI_ALIGNMENT_H_ */
