#ifndef _BUTTON_H_
#define _BUTTON_H_

#include "interior.h"

typedef struct button_interior button_interior_t;

typedef enum
{
    BUTTON_ON_PRESS = 0,
    BUTTON_ON_RELEASE,
}
button_when_t;

typedef struct
{
    button_when_t when;
    void (*action) (void *const data);
    void *action_data;
}
button_action_t;

typedef struct
{
    interior_opts_t interior;
    button_action_t action;
}
button_interior_opts_t;


interior_interface_t button_interior_get_impl(void);


#endif/*_BUTTON_H_*/
