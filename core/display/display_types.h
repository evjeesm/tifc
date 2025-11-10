#ifndef _DISPLAY_TYPES_
#define _DISPLAY_TYPES_

#include <stdint.h>

/* Describes position of the character on the screen */
typedef struct
{
    uint16_t x;
    uint16_t y;
}
disp_pos_t;

/* Describes area of the screen */
typedef struct
{
    disp_pos_t first;  /* left-top corner     */
    disp_pos_t second; /* right-bottom corner */
}
disp_area_t;

#define IS_INVALID_AREA(area_ptr) \
   ((area_ptr)->first.x  == (uint16_t) -1 &&\
    (area_ptr)->first.y  == (uint16_t) -1 &&\
    (area_ptr)->second.x == (uint16_t) -1 &&\
    (area_ptr)->second.y == (uint16_t) -1)

#define INVALID_AREA ((disp_area_t) {{-1, -1}, {-1, -1}})

#endif//_DISPLAY_TYPES_
