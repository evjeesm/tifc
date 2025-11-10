#ifndef _UI_H_
#define _UI_H_

#include "input.h"

typedef struct
{
    input_hooks_t hooks;
    bool exit_requested;
}
ui_t;

#endif /* _UI_H_ */
