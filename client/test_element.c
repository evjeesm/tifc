#include "test_element.h"


static void *test_element_alloc(Arena *const arena);
static void test_element_init(ui_element_t *const element, void *opts);
static void test_element_deinit(ui_element_t *const element);
static void test_element_recalculate(ui_element_t *const element);
static void test_element_render(const ui_element_t *element, display_t *const display);


ui_element_interface_t test_element_get_impl(void)
{
    return (ui_element_interface_t){
        .alloc = test_element_alloc,
        .init = test_element_init,
        .deinit = test_element_deinit,
        .recalculate = test_element_recalculate,
        .render = test_element_render,
    };
}


static void *test_element_alloc(Arena *const arena)
{
}


static void test_element_init(ui_element_t *const element, void *opts)
{
}


static void test_element_deinit(ui_element_t *const element)
{
}


static void test_element_recalculate(ui_element_t *const element)
{
}


static void test_element_render(const ui_element_t *element, display_t *const display)
{
}


