#ifndef _BUTTON_H_
#define _BUTTON_H_

#include "interior.h"

typedef struct button_interior button_interior_t;


typedef struct
{
    interior_opts_t interior;
}
button_interior_opts_t;


interior_interface_t button_interior_get_impl(void);


#endif/*_BUTTON_H_*/
