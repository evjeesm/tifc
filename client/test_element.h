#ifndef _TEST_ELEMENT_H_
#define _TEST_ELEMENT_H_

#include "ui_element.h"

typedef struct test_element_opts
{
    int sent;
}
test_element_opts_t;

typedef struct test_element
{
    int sent;
}
test_element_t;

ui_element_interface_t test_element_get_impl(void);

#endif/* _TEST_ELEMENT_H_ */
