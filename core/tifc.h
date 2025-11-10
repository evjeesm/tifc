#ifndef _TIFC_H_
#define _TIFC_H_

#include "display.h"
#include "input.h"
#include "ui.h"

typedef struct tifc
{
    display_t    display;
    input_t      input;
    ui_t         ui;
}
tifc_t;

#endif /* _TIFC_H_ */
