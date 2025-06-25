#ifndef _TEXT_INPUT_FIELD_H_
#define _TEXT_INPUT_FIELD_H_

#include "interior.h"

typedef struct text_input_field text_input_field_t;

typedef struct
{
    void (*submit) (void *const data);
    void *submit_data;
}
text_input_field_action_t;

typedef struct
{
    interior_opts_t interior;
    text_input_field_action_t action;
    size_t max_length;
    /* ... */
}
text_input_field_opts_t;


interior_interface_t text_input_field_interior_get_impl(void);


#endif/*_TEXT_INPUT_FIELD_H_*/
