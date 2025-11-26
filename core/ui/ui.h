#ifndef _UI_H_
#define _UI_H_

#include "ui_element.h"
#include "ui_container.h"

#include "input.h"
#include "hashmap.h"


typedef struct
{
    /* Element storage */
    hashmap_t *elements;
    hashmap_t *containers;
    ui_id_t new_element_id;

    input_hooks_t hooks;
    bool exit_requested;
}
ui_t;


typedef enum ui_status
{
    UI_SUCCESS = 0,
    UI_FAIL = 1,
    UI_CONTAINER_NOT_FOUND = 2,
    UI_ELEMENT_NOT_FOUND = 3,
}
ui_status_t;


void ui_init(ui_t *const ui);
void ui_deinit(ui_t *const ui);
void ui_recalculate(ui_t *const ui, const display_t *const display);

ui_id_t ui_create_element(ui_t *const ui, ui_element_opts_t *const opts);
ui_id_t ui_create_container(ui_t *const ui, ui_container_opts_t *const opts);
ui_status_t ui_assign_to_container(ui_t *const ui, ui_id_t element_id, ui_id_t container_id, ui_id_t area_id);



#endif /* _UI_H_ */
