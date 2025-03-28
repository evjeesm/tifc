#ifndef _LAYOUT_H_
#define _LAYOUT_H_

typedef enum
{
    LAYOUT_ALIGN_TOP            = 1 << 0,
    LAYOUT_ALIGN_BOT            = 1 << 1,
    LAYOUT_ALIGN_LEFT           = 1 << 2,
    LAYOUT_ALIGN_RIGHT          = 1 << 3,

    LAYOUT_ALIGN_TOP_H_CENTER   = LAYOUT_ALIGN_TOP
                                | LAYOUT_ALIGN_LEFT
                                | LAYOUT_ALIGN_RIGHT,

    LAYOUT_ALIGN_BOT_H_CENTER   = LAYOUT_ALIGN_BOT
                                | LAYOUT_ALIGN_LEFT
                                | LAYOUT_ALIGN_RIGHT,

    LAYOUT_ALIGN_LEFT_V_CENTER  = LAYOUT_ALIGN_LEFT
                                | LAYOUT_ALIGN_TOP
                                | LAYOUT_ALIGN_BOT,
    
    LAYOUT_ALIGN_RIGHT_V_CENTER = LAYOUT_ALIGN_RIGHT
                                | LAYOUT_ALIGN_TOP
                                | LAYOUT_ALIGN_BOT,

    LAYOUT_ALIGN_CENTER         = LAYOUT_ALIGN_LEFT
                                | LAYOUT_ALIGN_RIGHT
                                | LAYOUT_ALIGN_TOP
                                | LAYOUT_ALIGN_BOT,
}
layout_align_t;

typedef enum
{
    LAYOUT_SIZE_FIXED = 0, /* fixed amount */
    LAYOUT_SIZE_RELATIVE   /* percents from free space left */
}
layout_size_method_t;


#endif /* _LAYOUT_H_ */
