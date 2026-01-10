#include "event_system.h"
#include "utils.h"
#include "testing.h"

#include <stdio.h>
#include <stdbool.h>
#include <stdio.h>

// USER DEFINED EVENTS IMPLEMENTATION
typedef enum
{
    MY_EVENT_KEYBOARD = 0,
    MY_EVENT_MOUSE
}
my_event_type;

typedef struct
{
    void *binding_stub;
}
keyboard_event_type_data_t;

typedef struct
{
    void *spidex_stub;
}
mouse_event_type_data_t;

void keyboard_subscribe(void *const subscriber, event_type_t *const event_type)
{
    printf("SUBSCRIBE for keyboard; subscriber: (%p) -> event_type: (%p)\n", subscriber, event_type);
}

void keyboard_unsubscribe(void *const subscriber, event_type_t *const event_type)
{
    printf("UNSUB from keyboard; subscriber: (%p) -> event_type_data: (%p)\n", subscriber, event_type);
}

void keyboard_dispatch(void *const event_data, event_type_t *const event_type)
{
    printf("DISPATCH keyboard; event_data: (%p) -> event_type: (%p)\n", event_data, event_type);
}

void keyboard_close(event_type_t *const event_type)
{
    printf("CLOSE keyboard event: (%p) \n", event_type);
}

static event_publisher_interface_t get_my_kb_event_impl(void)
{
    return (event_publisher_interface_t){
        .subscribe = keyboard_subscribe,
        .unsubscribe = keyboard_unsubscribe,
        .dispatch = keyboard_dispatch,
        .close = keyboard_close,
    };
}

void mouse_subscribe(void *const subscriber, event_type_t *const event_type)
{
    printf("SUBSCRIBE for mouse; subscriber: (%p) -> event_type: (%p)\n", subscriber, event_type);
}

void mouse_unsubscribe(void *const subscriber, event_type_t *const event_type)
{
    printf("UNSUB from mouse; subscriber: (%p) -> event_type_data: (%p)\n", subscriber, event_type);
}

void mouse_dispatch(void *const event_data, event_type_t *const event_type)
{
    printf("DISPATCH mouse; event_data: (%p) -> event_type: (%p)\n", event_data, event_type);
}

void mouse_close(event_type_t *const event_type)
{
    printf("CLOSE mouse event: (%p) \n", event_type);
}

static event_publisher_interface_t get_my_mouse_event_impl(void)
{
    return (event_publisher_interface_t){
        .subscribe = mouse_subscribe,
        .unsubscribe = mouse_unsubscribe,
        .dispatch = mouse_dispatch,
        .close = mouse_close,
    };
}

typedef void (*setup_func_t) (event_system_t *const es);
typedef bool (*test_func_t) (event_system_t *const es);
typedef void (*cleanup_func_t) (event_system_t *const es);

static bool run_test(const char *const test_name, setup_func_t setup_func, test_func_t test_func, cleanup_func_t cleanup_func);
static void setup(event_system_t *es);
static void cleanup(event_system_t *es);

static bool test_es(event_system_t *const es)
{
    TEST_ASSERT(es, RED("TEST"));

    /* USER allocates event type data */
    mouse_event_type_data_t mouse_event_type_data = {0};
    keyboard_event_type_data_t keyboard_event_type_data = {0};

    event_type_opts_t my_mouse_event_type_opts = {
        .type_id = MY_EVENT_MOUSE,
        .impl = get_my_mouse_event_impl(),
        .data = &mouse_event_type_data,
    };

    event_type_opts_t my_keyboard_event_type_opts = {
        .type_id = MY_EVENT_KEYBOARD,
        .impl = get_my_kb_event_impl(),
        .data = &keyboard_event_type_data,
    };

    /* Registering event types */
    es_create_publisher(es, &my_mouse_event_type_opts);
    es_create_publisher(es,  &my_keyboard_event_type_opts);


    TEST_SUCCESS();
}

int main(void)
{
    RUN_TEST(setup, test_es, cleanup);

    return 0;
}

static void setup(event_system_t *es)
{
    es_init(es);
}

static void cleanup(event_system_t *es)
{
    es_deinit(es);
}

static bool run_test(const char *const test_name, setup_func_t setup_func, test_func_t test_func, cleanup_func_t cleanup_func)
{
    event_system_t es;
    setup_func(&es);

    bool result = test_func(&es);
    if (!result) TEST_MSG("["RED("FAILED")"] Test %s\n", test_name);

    cleanup_func(&es);
    return result;
}

