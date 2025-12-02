#ifndef _UI_CONTAINER_H_
#define _UI_CONTAINER_H_

#include "ui_element.h"
#include "container_layout.h"

#include "sparse.h"

typedef struct
{
    container_layout_opts_t layout;
}
ui_container_opts_t;


typedef struct ui_container
{
    container_layout_t layout;
    sparse_t *children;
}
ui_container_t;


ui_element_interface_t ui_container_get_impl(void);

void ui_container_assign_child(ui_container_t *const container, ui_id_t element_id, ui_id_t area_id);

void remove_child(ui_container_t *const container, ui_id_t child_id);



#endif/*_UI_CONTAINER_H_ */
